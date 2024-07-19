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
#define STAGE_SHARE_POSITION 0x04
// #define STAGE_COMFIRM_FINAL 0x05 //通过share_position判断

#define MAX_ANCHOR_NUMBER 4
#define MAX_TASK_NUMBER 128

inline uint32_t random_generator(uint32_t zhongzi, uint32_t low, uint32_t high) {
    srand(zhongzi);//随机数种子设置
    uint32_t random_value = rand() % (high + 1 - low) + low;//随机数生成
    // printf("current random number is: %d\r\n", random_value);
    return random_value;
}

inline void put_distance(tag_info_t* self, uint8_t anchor_id, float d) {
    //均值滤波
    *((float*)self + anchor_id) = (*((float*)self + anchor_id)+d)/2;
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
        if (module_config.ranging_exchange_debug_output) debug_printf("[Error] Invalid module ID.\n");
        return 1;
    } else if (mode == ANCHOR) {
        current_state = ANCHOR_FREE;
        // 添加三个tag进去。写死。
        n1.id = 0x81;
        n1.next = &n2;
        n2.id = 0x82;
        n2.next = &n3;
        n3.id = 0x83;
        n3.next = &n1;
        current_node = &n1;
    } else if (mode == TAG) {
        current_state = TAG_FREE;
        tag_storage.d1 = 0.0f;
        tag_storage.d2 = 0.0f;
    }

    reset_DW1000();
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Error] DW1000 initialization failed.\n");
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

        // dwt_setrxtimeout(RX_TIMEOUT_UUS);
        dwt_setrxtimeout(RANDOM_TIMEOUT_DELAY);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

    } else if (mode == TAG) {
        dwt_setdblrxbuffmode(1);

        dwt_setcallbacks(&TagTXConfirmationCallback, &TagRXOkCallback, &TagRXTimeoutCallback, &TagRXErrorCallback);
        dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);

        dwt_rxenable(DWT_START_RX_IMMEDIATE);
    }

    if (module_config.ranging_exchange_debug_output) debug_printf("[Info] Module initialized. (ID: %d)\n", module_config.module_id);

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

    if (msg_type == STAGE_POLL && current_state == ANCHOR_POLL) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent poll to %d for task %d.\n", dest_id, task_id);
        anchor_poll_tx = dwt_readtxtimestamplo32();
    } else if (msg_type == STAGE_FINAL && current_state == ANCHOR_FINAL) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent final to %d for task %d.\n", dest_id, task_id);
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
    const int state = current_state;
    switch(state) {
        case ANCHOR_POLL:
            if (
                rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE && //帧头
                src_id == current_node->id && //来源id为测距id
                dest_id == module_config.module_id && //目标id为本机id
                msg_type == STAGE_RESPONSE && //消息类型为response
                pan_id == MSG_PAN_ID //网络号一致
            ) {
                if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received response from %d for task %d.\n", src_id, task_id);

                // Set expected time of transmission
                // uint32_t expected_final_tx_time = rx_ts_hi + ((RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8);
                uint32_t expected_final_tx_time = rx_ts_hi + ((RESP_RX_TO_FINAL_TX_DLY_UUS << 16) >> 8);
                dwt_setdelayedtrxtime(expected_final_tx_time); // 接收的是 dwt time 高 32 位

                // Calculate and record final_tx (add antenna delay)
                uint32_t final_tx = (uint32_t) ((((uint64_t) (expected_final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY);

                // Prepare the final message
                uint8_t payload[12] = {0x00};
                // uint8_t tx = 0;
                // done: deal with anchor_task_list[task_id].poll_tx
                memcpy(payload, &anchor_poll_tx, 4);
                memcpy(payload + 4, &rx_ts_lo, 4);
                memcpy(payload + 8, &final_tx, 4);
                tx_len = gen_ranging_exchange_msg(tx_buffer, MSG_PAN_ID, src_id, module_config.module_id, task_id, STAGE_FINAL, payload, sizeof(payload));

                // Send the final message
                dwt_writetxdata(tx_len, tx_buffer, 0);
                dwt_writetxfctrl(tx_len, 0, 1);
                dwt_starttx(DWT_START_TX_DELAYED);
                // done: 设置timeout。
                dwt_setrxtimeout(RANDOM_TIMEOUT_DELAY);
                current_state = ANCHOR_FINAL;
                /* Should not destroy the task, there can be multiple tags waiting for the same poll_tx */
            }
            break;
        case ANCHOR_FINAL: // anchor收到来自对应的tag的确认消息
            if (
                rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE && //帧头
                src_id == current_node->id && //来源id为测距id
                dest_id == module_config.module_id && //目标id为本机id
                msg_type == STAGE_SHARE_POSITION &&
                pan_id == MSG_PAN_ID //网络号一致
            ) {
                // 切换到free模式。
                tle_times = 0;
                current_state = ANCHOR_FREE;
                current_node = current_node->next;
                dwt_setrxtimeout(RANDOM_TIMEOUT_DELAY/2); // 设置一个小的延迟
            }
        default: break;
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void AnchorRXTimeoutCallback(const dwt_cb_data_t *data) {
    // task-id变为current-node的id，没啥用，占位。
    if (tx_flag) {
        // 根据现在的anchor的state决定anchor对信息的处理
        switch (current_state) {
            case ANCHOR_POLL: // 这两个阶段timeout，则超时计数器自增。
            case ANCHOR_FINAL:
                tle_times += 1;
                if(tle_times == MAX_TLE_TIMES) {
                    // 放弃任务，直接进入下一轮。
                    current_node = current_node->next; // 更新node
                    uint8_t payload[8] = {0x00};
                    memcpy(payload, &module_config.anchor_x, 4);
                    memcpy(payload + 4, &module_config.anchor_y, 4);
                    tx_len = gen_ranging_exchange_msg(
                        tx_buffer,
                        MSG_PAN_ID,
                        current_node->id,
                        module_config.module_id,
                        current_node->id,
                        STAGE_POLL,
                        payload,
                        sizeof(payload));
                    current_state = ANCHOR_POLL;
                    tle_times = 0;
                    dwt_setrxtimeout(RANDOM_TIMEOUT_DELAY);
                }
            // 否则，重新发送上一轮的消息。由于消息已经被存储好了，直接发送即可。
            // 跳出这个switch即可发送上一次的消息。
            break;

            // 发送轮询请求。
            case ANCHOR_FREE:
                uint8_t payload[8] = {0x00};
                memcpy(payload, &module_config.anchor_x, 4);
                memcpy(payload + 4, &module_config.anchor_y, 4);
                tx_len = gen_ranging_exchange_msg(
                    tx_buffer,
                    MSG_PAN_ID,
                    current_node->id,
                    module_config.module_id,
                    current_node->id,
                    STAGE_POLL,
                    payload,
                    sizeof(payload));
                tle_times = 0;
                current_state = ANCHOR_POLL;
            dwt_setrxtimeout(RANDOM_TIMEOUT_DELAY);
            break;
        }
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
    if (dest_mode == ANCHOR && current_state == TAG_RESPONSE) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Sent resp to %d for task %d.\n", dest_id, task_id);

        // Record the response transmission timestamp
        // tag_task_list[dest_id][task_id].resp_tx = tx_ts;
        // resp_tx = tag_resp_tx
        // 原来的tag_task_list[dest_id][task_id].resp_tx被保存在了全局变量tag_resp_tx。
        // 因为没有task了，每次只最多保证一次测距完成，所以这样写。
        tag_resp_tx = dwt_readtxtimestamplo32();
    } else if (dest_mode == TAG) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Shared position with other tags.\n");
    }
}

static void send_response_from_tag(uint8_t dest_id) {
    float anchor_x, anchor_y;
    uint16_t payload_head_index = re_get_payload_head_index();
    memcpy(&anchor_x, rx_buffer + payload_head_index, 4);
    memcpy(&anchor_y, rx_buffer + payload_head_index + 4, 4);

    // Prepare the response message
    uint8_t payload[] = {};
    tx_len = gen_ranging_exchange_msg(
        tx_buffer,
        MSG_PAN_ID,
        dest_id,
        module_config.module_id,
        0,
        STAGE_RESPONSE,
        payload,
        sizeof(payload));

    // Send the response message
    dwt_writetxdata(tx_len, tx_buffer, 0);
    dwt_writetxfctrl(tx_len, 0, 1);
    dwt_starttx(DWT_START_TX_IMMEDIATE);
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
        if (src_mode == ANCHOR) { // 来自基站的消息
            if (dest_id == module_config.module_id) {// TAG 在free状态，接收到anchor发来的poll
                if (current_state == TAG_FREE && msg_type == STAGE_POLL) {
                    tag_poll_rx = rx_ts;// 记录poll_rx
                    // todo: 有没有开tx
                    send_response_from_tag(src_id);
                    // 状态设置：进入抱死状态
                    current_connect_id = src_id;
                    current_state = TAG_RESPONSE;
                } else if (// 当前在tag response阶段，收到了来自anchor的stage_fianl消息
                    current_state == TAG_RESPONSE &&
                    msg_type == STAGE_FINAL &&
                    src_id == current_connect_id
                    ) {
                    if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received final from %d for task %d.\n", src_id, task_id);

                    // Record the final reception timestamp
                    uint32_t final_rx = rx_ts;

                    // Extract the recorded timestamps
                    // uint32_t poll_rx = tag_task_list[anchor_index][task_id].poll_rx;
                    // uint32_t resp_tx = tag_task_list[anchor_index][task_id].resp_tx;
                    // done: 取出poll rx和resp tx。
                    // reso_tx = tag_resp_tx, OK.
                    // poll_rx = tag_poll_rx, OK.
                    // uint32_t poll_rx, resp_tx;
                    // Unpack the timestamps
                    uint32_t poll_tx, resp_rx, final_tx;
                    uint16_t payload_head_index = re_get_payload_head_index();
                    memcpy(&poll_tx, rx_buffer + payload_head_index, 4);
                    memcpy(&resp_rx, rx_buffer + payload_head_index + 4, 4);
                    memcpy(&final_tx, rx_buffer + payload_head_index + 8, 4);

                    // Calculate time of flight and distance
                    double Ra = (double) (resp_rx - poll_tx);
                    double Rb = (double) (final_rx - tag_resp_tx);
                    double Da = (double) (final_tx - resp_rx);
                    double Db = (double) (tag_resp_tx - tag_poll_rx);
                    double tof = ((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db)) * DWT_TIME_UNITS;
                    float distance = (float) (tof * SPEED_OF_LIGHT);
                    put_distance(&tag_storage, src_id, distance);
                // Print the result
//                debug_printf("   Tag: %lu, \t%lu, \t%lu\n", poll_rx, resp_tx, final_rx);
//                debug_printf("Anchor: %lu, \t%lu, \t%lu\n", poll_tx, resp_rx, final_tx);
//                debug_printf("    Ra: %lu\n", resp_rx - poll_tx);
//                debug_printf("    Rb: %lu\n", final_rx - resp_tx);
//                debug_printf("    Da: %lu\n", final_tx - resp_rx);
//                debug_printf("    Db: %lu\n", resp_tx - poll_rx);
//                debug_printf("%.4f\n", (float) distance);
//                debug_printf("%d\n", (int) (distance * 10000));

                    float d = 2.0;
                    Point2d p = dis2cart(tag_storage.d1, tag_storage.d2, d);
                    tag_storage.x = p.x;
                    tag_storage.y = p.y;
                    send_upload_position_msg(
                        module_config.module_id,
                        p.x,
                        p.y,
                        tag_storage.d1,
                        tag_storage.d2);

                    // Broadcast the position
                    uint8_t payload[16] = {0x00};
                    memcpy(payload, &p.x, 4);
                    memcpy(payload + 4, &p.y, 4);
                    memcpy(payload + 8, &(tag_storage.d1), 4);
                    memcpy(payload + 12, &(tag_storage.d2), 4);
                    tx_len = gen_ranging_exchange_msg(
                        tx_buffer,
                        MSG_PAN_ID,
                        MSG_BROADCAST_ID,
                        module_config.module_id,
                        0xFF,
                        STAGE_SHARE_POSITION,
                        payload,
                        sizeof(payload));
                    dwt_writetxdata(tx_len, tx_buffer, 0);
                    dwt_writetxfctrl(tx_len, 0, 1);
                    dwt_starttx(DWT_START_TX_IMMEDIATE);

                    current_state = TAG_FREE;
                }else if (
                    current_state == TAG_FREE &&
                    msg_type == STAGE_FINAL &&
                    src_id == current_connect_id
                ) {
                    // 收到来自anchor的重复确认的final消息： 直接广播。
                    // 算了，不管。
                    // dwt_writetxdata(tx_len, tx_buffer, 0);
                    // dwt_writetxfctrl(tx_len, 0, 1);
                    // dwt_starttx(DWT_START_TX_IMMEDIATE);
                }

                // }
            }
        }else { //来自tag的消息
            if (dest_id == MSG_BROADCAST_ID
                && current_state == TAG_FREE // todo: 能不能在测试的间隙，接收别的tag的位置。
                ) {
                if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Received position from %d.\n", src_id);

                // Unpack the position
                float x, y, d1, d2;
                uint16_t payload_head_index = re_get_payload_head_index();
                memcpy(&x, rx_buffer + payload_head_index, 4);
                memcpy(&y, rx_buffer + payload_head_index + 4, 4);
                memcpy(&d1, rx_buffer + payload_head_index + 8, 4);
                memcpy(&d2, rx_buffer + payload_head_index + 12, 4);

                // Upload the position
                //                debug_printf("Position of %d: (%d, %d).\n", src_id, (int) (x * 10000), (int) (y * 10000));
                send_upload_position_msg(src_id, x, y, d1, d2);
            }
        }
    }
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void TagRXTimeoutCallback(const dwt_cb_data_t *data) {

    // dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
    const int state = current_state;
    switch (state) {
        case TAG_RESPONSE: // response期间超时，则再次发送response
            // todo: 开tx，关rx
            send_response_from_tag(current_connect_id);
            tle_times += 1;
            if (tle_times == MAX_TLE_TIMES) {
                tle_times = 0;
                current_state = TAG_FREE;
            }
            break;
        case TAG_FREE: //广播坐标。
            // Broadcast the position
            uint8_t payload[16] = {0x00};
            memcpy(payload, &tag_storage.x, 4);
            memcpy(payload + 4, &tag_storage.y, 4);
            memcpy(payload + 8, &(tag_storage.d1), 4);
            memcpy(payload + 12, &(tag_storage.d2), 4);
            tx_len = gen_ranging_exchange_msg(
                tx_buffer,
                MSG_PAN_ID,
                MSG_BROADCAST_ID,
                module_config.module_id,
                0xFF,
                STAGE_SHARE_POSITION,
                payload,
                sizeof(payload));
            dwt_writetxdata(tx_len, tx_buffer, 0);
            dwt_writetxfctrl(tx_len, 0, 1);
            dwt_starttx(DWT_START_TX_IMMEDIATE);
            break;
    }
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
