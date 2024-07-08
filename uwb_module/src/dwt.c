#include "dwt.h"

#include "string.h"

#include "systick.h"
#include "gpio.h"

#include "tiny_io.h"
#include "msgs.h"
#include "tool.h"


uwb_mode_t JudgeModeFromID(uint8_t module_id) {
    if (module_id >= 0x00 && module_id <= 0x7F) {
        return ANCHOR;
    } else if (module_id >= 0x80 && module_id <= 0xFF) {
        return TAG;
    } else {
        return UNDEFINED;
    }
}


#define MSG_MAX_FRAME_LENGTH 64
#define MSG_PAN_ID 0x2333
#define MSG_BROADCAST_ID 0xFF

#define STAGE_IDLE 0x00
#define STAGE_POLL 0x01
#define STAGE_RESPONSE 0x02
#define STAGE_FINAL 0x03

#define MAX_ANCHOR_NUMBER 4
#define MAX_TASK_NUMBER 128

#define DISTANCE_FILTER_LENGTH 8

typedef union {
    struct { /* For Anchor */
        uint32_t poll_tx;
        uint32_t no_use;
    };
    struct { /* For Tag */
        uint32_t poll_rx;
        uint32_t resp_tx;
    };
    struct {
        uint32_t reg_1;
        uint32_t reg_2;
    };
} ranging_task_t;

typedef struct _anchor_info_t {
    uint8_t id;
    float x, y;
    float dists[DISTANCE_FILTER_LENGTH];
    float dist;
    uint8_t dist_head;
    uint64_t timestamp_ms; /* Systick timestamp, not DWM. 0 means not received. */
} anchor_info_t;

/* For Anchors */
static ranging_task_t anchor_task_list[MAX_TASK_NUMBER] = {0};
static uint8_t anchor_task_list_head = MAX_TASK_NUMBER - 1;

void initialize_anchor_task_list() {
    memset(anchor_task_list, 0, sizeof(anchor_task_list));
    anchor_task_list_head = MAX_TASK_NUMBER - 1;
}

uint8_t add_anchor_task() {
    if (++ anchor_task_list_head >= MAX_TASK_NUMBER) {
        anchor_task_list_head = 0;
    }
    return anchor_task_list_head;
    /* regs need to be filled outside this */
}

uint8_t is_anchor_task_idle(uint8_t task_id) {
    return anchor_task_list[task_id].poll_tx == 0 && anchor_task_list[task_id].no_use == 0;
}

void terminate_anchor_task(uint8_t task_id) {
    anchor_task_list[task_id].reg_1 = 0;
    anchor_task_list[task_id].reg_2 = 0;
}

/* For Tags */
static ranging_task_t tag_task_list[MAX_ANCHOR_NUMBER][MAX_TASK_NUMBER] = {0};
static anchor_info_t anchor_info_list[MAX_ANCHOR_NUMBER] = {0};
static uint8_t anchor_info_list_head = MAX_ANCHOR_NUMBER - 1;

void initialize_tag_task_list_and_anchor_info() {
    memset(tag_task_list, 0, sizeof(tag_task_list));
    anchor_info_list_head = MAX_ANCHOR_NUMBER - 1;
    for (uint8_t i = 0; i < MAX_ANCHOR_NUMBER; i ++) {
        anchor_info_list[i].id = 0xFF;
        anchor_info_list[i].dist_head = DISTANCE_FILTER_LENGTH - 1;
    }
}

uint8_t is_tag_task_idle(uint8_t anchor_idx, uint8_t task_id) {
    return tag_task_list[anchor_idx][task_id].poll_rx == 0 && tag_task_list[anchor_idx][task_id].resp_tx == 0;
}

void terminate_tag_task(uint8_t anchor_idx, uint8_t task_id) {
    tag_task_list[anchor_idx][task_id].reg_1 = 0;
    tag_task_list[anchor_idx][task_id].reg_2 = 0;
}

uint8_t add_anchor_info(uint8_t anchor_id, float anchor_x, float anchor_y) {
    if (++ anchor_info_list_head >= MAX_ANCHOR_NUMBER) {
        anchor_info_list_head = 0;
    }
    anchor_info_list[anchor_info_list_head].id = anchor_id;
    anchor_info_list[anchor_info_list_head].x = anchor_x;
    anchor_info_list[anchor_info_list_head].y = anchor_y;
    anchor_info_list[anchor_info_list_head].dist_head = DISTANCE_FILTER_LENGTH - 1;
    anchor_info_list[anchor_info_list_head].timestamp_ms = 0;
    return anchor_info_list_head;
}

float distance_filter(float *dists, uint8_t length) {
    float sum = 0.0f;
    for (uint8_t i = 0; i < length; i ++) {
        sum += dists[i];
    }
    return sum / length;
}

uint8_t add_distance_to_anchor(uint8_t anchor_idx, float distance) {
    if (++ anchor_info_list[anchor_idx].dist_head >= DISTANCE_FILTER_LENGTH) {
        anchor_info_list[anchor_idx].dist_head = 0;
    }
    anchor_info_list[anchor_idx].dists[anchor_info_list[anchor_idx].dist_head] = distance;
    anchor_info_list[anchor_idx].dist = distance_filter(anchor_info_list[anchor_idx].dists, DISTANCE_FILTER_LENGTH);
    return anchor_info_list[anchor_idx].dist_head;
}

uint8_t get_anchor_index_by_id(uint8_t anchor_id) {
    for (uint8_t i = 0; i < MAX_ANCHOR_NUMBER; i ++) {
        if (anchor_info_list[i].id == anchor_id) {
            return i;
        }
    }
    return 0xFF;
}


static void AnchorTXConfirmationCallback(const dwt_cb_data_t *data);
static void AnchorRXOkCallback(const dwt_cb_data_t *data);
static void AnchorRXTimeoutCallback(const dwt_cb_data_t *data);
static void AnchorRXErrorCallback(const dwt_cb_data_t *data);
static void TagTXConfirmationCallback(const dwt_cb_data_t *data);
static void TagRXOkCallback(const dwt_cb_data_t *data);
static void TagRXTimeoutCallback(const dwt_cb_data_t *data);
static void TagRXErrorCallback(const dwt_cb_data_t *data);

void EXTI0_IRQHandler(void) {
    EXTI_ClearITPendingBit(DECAIRQ_EXTI);
    dwt_isr();
}

uint8_t Initialize() {
    uwb_mode_t mode = JudgeModeFromID(module_config.module_id);

    if (mode == UNDEFINED) {
        debug_printf("[Error] Invalid module ID.\n");
        return 1;
    }

    reset_DW1000();
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        debug_printf("[Error] DW1000 initialization failed.\n");
        return 1;
    }
    spi_set_rate_high();

    dwt_configure(&dwt_config);

    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    if (mode == ANCHOR) {
        dwt_setdblrxbuffmode(1);

        dwt_setcallbacks(&AnchorTXConfirmationCallback, &AnchorRXOkCallback, &AnchorRXTimeoutCallback, &AnchorRXErrorCallback);
        dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);

        dwt_setrxtimeout(RX_TIMEOUT_UUS);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // Initialize the task list
        initialize_anchor_task_list();
    } else if (mode == TAG) {
        dwt_setdblrxbuffmode(1);

        dwt_setcallbacks(&TagTXConfirmationCallback, &TagRXOkCallback, &TagRXTimeoutCallback, &TagRXErrorCallback);
        dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);

        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // Initialize the task list
        initialize_tag_task_list_and_anchor_info();
    }

    debug_printf("[Info] Module initialized. (ID: %d)\n", module_config.module_id);

    return 0;
}


/* Temporary variables & Interruption */
volatile uint8_t rx_buffer[MSG_MAX_FRAME_LENGTH] = {0x00};

volatile uint8_t tx_flag = 0;
volatile uint16_t tx_len = 0;
volatile uint8_t tx_buffer[MSG_MAX_FRAME_LENGTH] = {0x00};

/* State variables */
static uint32_t last_tick_ms = 0;
static uint8_t toggle_flag = 0;

/* Anchor Related */
static void AnchorTXConfirmationCallback(const dwt_cb_data_t *data) {
    uint8_t dest_id = re_get_dest_id(tx_buffer);
    uint8_t task_id = re_get_task_id(tx_buffer);
    uint8_t msg_type = re_get_msg_type(tx_buffer);

    if (msg_type == STAGE_POLL) {
//        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Broadcast poll for task %d.\n", task_id);
        // TODO: 见后 RXOK 中断, 暂时写死，轮流发给两个 tag.
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent poll to %d for task %d.\n", dest_id, task_id);

        // Record the poll transmission timestamp
        anchor_task_list[task_id].poll_tx = dwt_readtxtimestamplo32();
    } else if (msg_type == STAGE_FINAL) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent final to %d for task %d.\n", dest_id, task_id);

        // TODO: Debug, 结论: 算出的 final_tx 和发出去的实际时间确实一样. final_tx 是需要手动加上 TX_ANT_DLY 的, 因为 delayed TX 的时间会自动考虑 TX_ANT_DLY.
//        uint32_t final_tx;
//        uint16_t payload_head_index = re_get_payload_head_index();
//        memcpy(&final_tx, tx_buffer + payload_head_index + 8, 4);
//        debug_printf("%lu, %lu.\n", final_tx, dwt_readtxtimestamplo32());
    }
}

static void AnchorRXOkCallback(const dwt_cb_data_t *data) {
    if (data->datalength <= MSG_MAX_FRAME_LENGTH) {
        dwt_readrxdata(rx_buffer, data->datalength, 0);
    }

    uint32_t rx_ts_hi = dwt_readrxtimestamphi32();
    uint32_t rx_ts_lo = dwt_readrxtimestamplo32();

    uint16_t pan_id = re_get_pan_id(rx_buffer);
    uint16_t dest_id = re_get_dest_id(rx_buffer);
    uint16_t src_id = re_get_src_id(rx_buffer);
    uint8_t task_id = re_get_task_id(rx_buffer);
    uint8_t msg_type = re_get_msg_type(rx_buffer);
    if (
        rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
        pan_id == MSG_PAN_ID &&
        dest_id == module_config.module_id &&
        !is_anchor_task_idle(task_id) &&
        msg_type == STAGE_RESPONSE
        ) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received response from %d for task %d.\n", src_id, task_id);

        // Set expected time of transmission
        uint32_t expected_final_tx_time = rx_ts_hi + ((RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8);
        dwt_setdelayedtrxtime(expected_final_tx_time); // 接收的是 dwt time 高 32 位

        // Calculate and record final_tx (add antenna delay)
        uint32_t final_tx = (uint32_t) ((((uint64_t) (expected_final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY);

        // Prepare the final message
        uint8_t payload[12] = {0x00};
        memcpy(payload, &anchor_task_list[task_id].poll_tx, 4);
        memcpy(payload + 4, &rx_ts_lo, 4);
        memcpy(payload + 8, &final_tx, 4);
        tx_len = gen_ranging_exchange_msg(tx_buffer, MSG_PAN_ID, src_id, module_config.module_id, task_id, STAGE_FINAL, payload, sizeof(payload));

        // Send the final message
        dwt_writetxdata(tx_len, tx_buffer, 0);
        dwt_writetxfctrl(tx_len, 0, 1);
        dwt_starttx(DWT_START_TX_DELAYED);

        // Destroy the task, TODO: 不应该销毁! 可能会有多个 Tag 都在等待这个 task_id 的回复.
//        terminate_anchor_task(task_id);
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void AnchorRXTimeoutCallback(const dwt_cb_data_t *data) {
    if (tx_flag) {
        // Add a new ranging task
        uint8_t new_task_id = add_anchor_task();

        // Prepare a poll message
        uint8_t payload[8] = {0x00};
        memcpy(payload, &module_config.anchor_x, 4);
        memcpy(payload + 4, &module_config.anchor_y, 4);
//        tx_len = gen_ranging_exchange_msg(tx_buffer, MSG_PAN_ID, MSG_BROADCAST_ID, module_config.module_id, new_task_id, STAGE_POLL, payload, sizeof(payload));
        // TODO: 暂时写死，轮流发给两个 tag.
        uint8_t dest_id = toggle_flag ? 0x80 : 0x81;
        tx_len = gen_ranging_exchange_msg(tx_buffer, MSG_PAN_ID, dest_id, module_config.module_id, new_task_id, STAGE_POLL, payload, sizeof(payload));

        // Send the poll message
        dwt_writetxdata(tx_len, tx_buffer, 0);
        dwt_writetxfctrl(tx_len, 0, 1);
        dwt_starttx(DWT_START_TX_IMMEDIATE);

        tx_flag = 0;
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void AnchorRXErrorCallback(const dwt_cb_data_t *data) {
    dwt_rxreset();
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

/* Tag Related */
static void TagTXConfirmationCallback(const dwt_cb_data_t *data) {
    uint8_t task_id = re_get_task_id(tx_buffer);
    uint8_t dest_id = re_get_dest_id(tx_buffer);

    uwb_mode_t dest_mode = JudgeModeFromID(dest_id);
    if (dest_mode == ANCHOR) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent resp to %d for task %d.\n", dest_id, task_id);

        // Record the response transmission timestamp
        tag_task_list[dest_id][task_id].resp_tx = dwt_readtxtimestamplo32();
    } else if (dest_mode == TAG) {
        // TODO: Tag 间.
    }
}

static void TagRXOkCallback(const dwt_cb_data_t *data) {
    if (data->datalength <= MSG_MAX_FRAME_LENGTH) {
        dwt_readrxdata(rx_buffer, data->datalength, 0);
    }

    uint32_t rx_ts = dwt_readrxtimestamplo32();

    uint16_t pan_id = re_get_pan_id(rx_buffer);
    uint16_t dest_id = re_get_dest_id(rx_buffer);
    uint16_t src_id = re_get_src_id(rx_buffer);
    uint8_t task_id = re_get_task_id(rx_buffer);
    uint8_t msg_type = re_get_msg_type(rx_buffer);
    if (rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE && pan_id == MSG_PAN_ID) {
        for (int i = 0; i <= 10000; i ++); // TODO: 不加有时候收不到 final.
        uwb_mode_t src_mode = JudgeModeFromID(src_id);
        uint8_t anchor_index = get_anchor_index_by_id(src_id); // 0xFF means not found
        if (src_mode == ANCHOR) {
            if (
                (dest_id == MSG_BROADCAST_ID || dest_id == module_config.module_id) &&
                msg_type == STAGE_POLL
            ) { /* poll */
                if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received poll from %d for task %d.\n", src_id, task_id);

                // Save the anchor information
                float anchor_x, anchor_y;
                uint16_t payload_head_index = re_get_payload_head_index();
                memcpy(&anchor_x, rx_buffer + payload_head_index, 4);
                memcpy(&anchor_y, rx_buffer + payload_head_index + 4, 4);
                if (anchor_index == 0xFF) { /* Not found -> Create one */
                    anchor_index = add_anchor_info(src_id, anchor_x, anchor_y);
                }

                // Record the poll reception timestamp
                tag_task_list[anchor_index][task_id].poll_rx = rx_ts;

                // Prepare the response message
                uint8_t payload[] = {};
                tx_len = gen_ranging_exchange_msg(tx_buffer, MSG_PAN_ID, src_id, module_config.module_id, task_id, STAGE_RESPONSE, payload, sizeof(payload));

                // Send the response message
                dwt_writetxdata(tx_len, tx_buffer, 0);
                dwt_writetxfctrl(tx_len, 0, 1);
                dwt_starttx(DWT_START_TX_IMMEDIATE);
            } else if (
                dest_id == module_config.module_id &&
                !is_tag_task_idle(anchor_index, task_id) &&
                msg_type == STAGE_FINAL
            ) { /* final */
                if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received final from %d for task %d.\n", src_id, task_id);

                // Record the final reception timestamp
                uint32_t final_rx = rx_ts;

                // Extract the recorded timestamps
                uint32_t poll_rx = tag_task_list[anchor_index][task_id].poll_rx;
                uint32_t resp_tx = tag_task_list[anchor_index][task_id].resp_tx;

                // Unpack the timestamps
                uint32_t poll_tx, resp_rx, final_tx;
                uint16_t payload_head_index = re_get_payload_head_index();
                memcpy(&poll_tx, rx_buffer + payload_head_index, 4);
                memcpy(&resp_rx, rx_buffer + payload_head_index + 4, 4);
                memcpy(&final_tx, rx_buffer + payload_head_index + 8, 4);

                // Calculate time of flight and distance
                double Ra = (double) (resp_rx - poll_tx);
                double Rb = (double) (final_rx - resp_tx);
                double Da = (double) (final_tx - resp_rx);
                double Db = (double) (resp_tx - poll_rx);
                double tof = ((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db)) * DWT_TIME_UNITS;
                float distance = (float) (tof * SPEED_OF_LIGHT);

                // Print the result
//                debug_printf("   Tag: %lu, \t%lu, \t%lu\n", poll_rx, resp_tx, final_rx);
//                debug_printf("Anchor: %lu, \t%lu, \t%lu\n", poll_tx, resp_rx, final_tx);
//                debug_printf("    Ra: %lu\n", resp_rx - poll_tx);
//                debug_printf("    Rb: %lu\n", final_rx - resp_tx);
//                debug_printf("    Da: %lu\n", final_tx - resp_rx);
//                debug_printf("    Db: %lu\n", resp_tx - poll_rx);
//                debug_printf("%.4f\n", (float) distance);
//                debug_printf("%d\n", (int) (distance * 10000));

                // Save the distance
                add_distance_to_anchor(anchor_index, distance);
                anchor_info_list[anchor_index].timestamp_ms = GetSystickMs();

                // Print all the distances
//                uint8_t anchor0_index = get_anchor_index_by_id(0x00);
//                uint8_t anchor1_index = get_anchor_index_by_id(0x01);
//                if (anchor0_index != 0xFF && anchor1_index != 0xFF) {
//                    debug_printf("%d, %d\n", (int) (anchor_info_list[anchor0_index].dist * 10000), (int) (anchor_info_list[anchor1_index].dist * 10000));
//                }

                // Localization
                uint8_t valid_anchor_indices[MAX_ANCHOR_NUMBER] = {0xFF};
                uint8_t valid_anchor_number = 0;
                for (uint8_t i = 0; i < MAX_ANCHOR_NUMBER; i ++) {
                    // TODO: timeout
                    if (anchor_info_list[i].id != 0xFF) { // && (GetSystickMs() - anchor_info_list[i].timestamp_ms < module_config.distance_expired_time)) {
                        valid_anchor_indices[valid_anchor_number ++] = i;
                    }
                }
                // TODO: 暂时两点定位, 后续补充多点定位
                if (valid_anchor_number >= 2) {
//                    Point2d p1 = {anchor_info_list[valid_anchor_indices[0]].x, anchor_info_list[valid_anchor_indices[0]].y};
//                    Point2d p2 = {anchor_info_list[valid_anchor_indices[1]].x, anchor_info_list[valid_anchor_indices[1]].y};
                    float d1 = anchor_info_list[valid_anchor_indices[0]].dist;
                    float d2 = anchor_info_list[valid_anchor_indices[1]].dist;
//                    Point2d p;
//                    int result = two_point_localization(p1, d1, p2, d2, &p);
//                    if (result) {
////                        debug_printf("%.4f, %.4f\n", p.x, p.y);
////                        debug_printf("%d, %d\n", (int) (p.x * 10000), (int) (p.y * 10000));
//                        debug_printf("%d, %d, %d, %d\n", (int) (p.x * 10000), (int) (p.y * 10000), (int) (d1 * 10000), (int) (d2 * 10000));
//                    }
                    // TODO: 暂时写死 0x00 是 (0, 0), 0x01 是 (d, 0)
                    float d = anchor_info_list[valid_anchor_indices[0]].id == 0x00 ? anchor_info_list[valid_anchor_indices[1]].x : anchor_info_list[valid_anchor_indices[0]].x;
                    Point2d p = dis2cart(d1, d2, d);
//                    debug_printf("%.4f, %.4f\n", p.x, p.y);
//                    debug_printf("%d, %d\n", (int) (p.x * 10000), (int) (p.y * 10000));
                    debug_printf("%d, %d, %d, %d\n", (int) (p.x * 10000), (int) (p.y * 10000), (int) (d1 * 10000), (int) (d2 * 10000));
                }

                // Remove the task
                terminate_tag_task(anchor_index, task_id);
            }
        } else {
            // TODO: Tag 间.
        }
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void TagRXTimeoutCallback(const dwt_cb_data_t *data) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void TagRXErrorCallback(const dwt_cb_data_t *data) {
    dwt_rxreset();
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

void EventHandler() {
    uwb_mode_t mode = JudgeModeFromID(module_config.module_id);

    if (mode == ANCHOR || mode == TAG) {
        // Regularly toggle the LED
        uint32_t current_tick_ms = GetSystickMs();
        if (current_tick_ms - last_tick_ms > module_config.ranging_exchange_poll_interval) {
            last_tick_ms = current_tick_ms;
            tx_flag = 1;

            if (toggle_flag) {
                TurnOffLED((led_t) JudgeModeFromID(module_config.module_id));
                toggle_flag = 0;
            } else {
                TurnOnLED((led_t) JudgeModeFromID(module_config.module_id));
                toggle_flag = 1;
            }
        }
    } else {
        // Do nothing but wait for the module ID to be modified
    }
}
