#include "dwt.h"

#include "string.h"

#include "systick.h"
#include "gpio.h"

#include "tiny_io.h"
#include "msgs.h"


uwb_mode_t JudgeModeFromID(uint16_t module_id) {
    if (module_id >= 0x0000 && module_id <= 0x0FFF) {
        return ANCHOR;
    } else if (module_id >= 0x1000 && module_id <= 0x1FFF) {
        return TAG;
    } else {
        return UNDEFINED;
    }
}


#define RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH 128
#define RANGING_EXCHANGE_MSG_PAN_ID 0x2333
#define RANGING_EXCHANGE_MSG_BROADCAST_ID 0xFFFF

#define RANGING_EXCHANGE_MAX_TASK_NUMBER 64 // < 256

#define RANGING_EXCHANGE_STAGE_IDLE 0x00
#define RANGING_EXCHANGE_STAGE_POLL 0x01
#define RANGING_EXCHANGE_STAGE_RESPONSE 0x02
#define RANGING_EXCHANGE_STAGE_FINAL 0x03

typedef struct _ranging_task_t {
    uint32_t poll_rx, resp_tx;
    uint32_t poll_tx, resp_rx, final_tx;
    uint8_t after_stage; // 0x00, i.e. After IDLE means the place is empty.
} ranging_task_t;

static ranging_task_t ranging_task_list[RANGING_EXCHANGE_MAX_TASK_NUMBER] = {0};
static uint8_t ranging_task_list_head = 0; // Only for the anchor

void init_ranging_task_list(void) {
    memset(ranging_task_list, 0, sizeof(ranging_task_list));
    ranging_task_list_head = 0;
}

uint8_t is_task_id_occupied(uint8_t task_id) {
    return ranging_task_list[task_id].after_stage != RANGING_EXCHANGE_STAGE_IDLE;
}

uint8_t add_ranging_task(uint8_t overwriting) {
    if (++ ranging_task_list_head >= RANGING_EXCHANGE_MAX_TASK_NUMBER) {
        ranging_task_list_head = 0;
    }
    uint8_t occupied_cnt = 0;
    while (!overwriting && is_task_id_occupied(ranging_task_list_head)) {
        ranging_task_list_head ++;
        occupied_cnt ++;
        if (occupied_cnt >= RANGING_EXCHANGE_MAX_TASK_NUMBER) {
            return 0; // TODO: dead loop if overwriting not allowed, what to return
        }
        if (ranging_task_list_head >= RANGING_EXCHANGE_MAX_TASK_NUMBER) {
            ranging_task_list_head = 0;
        }
    }
    ranging_task_list[ranging_task_list_head].after_stage = RANGING_EXCHANGE_STAGE_POLL;
    return ranging_task_list_head;
}

void destroy_ranging_task(uint8_t task_id) {
    ranging_task_list[task_id].after_stage = RANGING_EXCHANGE_STAGE_IDLE;
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

uint8_t InitDW1000() {
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
    } else if (mode == TAG) {
        dwt_setdblrxbuffmode(1);

        dwt_setcallbacks(&TagTXConfirmationCallback, &TagRXOkCallback, &TagRXTimeoutCallback, &TagRXErrorCallback);
        dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);

        dwt_rxenable(DWT_START_RX_IMMEDIATE);
    }

    debug_printf("[Info] DW1000 initialized.\n");

    // Clear the ranging task list
    init_ranging_task_list();

    return 0;
}


/* Temporary variables & Interruption */
volatile uint8_t rx_buffer[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH] = {0x00};

volatile uint8_t tx_flag = 0;
volatile uint16_t tx_len = 0;
volatile uint8_t tx_buffer[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH] = {0x00};

/* State variables */
static uint32_t last_tick_ms = 0;
static uint8_t toggle_flag = 0;

/* Anchor Related */
static void AnchorTXConfirmationCallback(const dwt_cb_data_t *data) {
    uint8_t src_id = tx_buffer[5] | (tx_buffer[6] << 8);
    uint8_t task_id = tx_buffer[7];
    uint8_t msg_type = tx_buffer[8];

    if (msg_type == RANGING_EXCHANGE_STAGE_POLL) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Send poll to %d for task %d.\n", src_id, task_id);

        // Record the poll transmission timestamp
        ranging_task_list[task_id].poll_tx = dwt_readtxtimestamplo32();
    } else if (msg_type == RANGING_EXCHANGE_STAGE_FINAL) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Send final to %d for task %d.\n", src_id, task_id);
    }
}

static void AnchorRXOkCallback(const dwt_cb_data_t *data) {
    if (data->datalength <= RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH) {
        dwt_readrxdata(rx_buffer, data->datalength, 0);
    }

    uint32_t rx_ts_hi = dwt_readrxtimestamphi32();
    uint32_t rx_ts_lo = dwt_readrxtimestamplo32();

    uint16_t pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
    uint16_t dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
    uint16_t src_id = rx_buffer[5] | (rx_buffer[6] << 8);
    uint8_t task_id = rx_buffer[7];
    uint8_t msg_type = rx_buffer[8];
    if (
        rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
        pan_id == RANGING_EXCHANGE_MSG_PAN_ID &&
        dest_id == module_config.module_id &&
        ranging_task_list[task_id].after_stage == RANGING_EXCHANGE_STAGE_POLL &&
        msg_type == RANGING_EXCHANGE_STAGE_RESPONSE
    ) {
        if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received response from %d for task %d.\n", src_id, task_id);

        // Set expected time of transmission
        uint32_t expected_final_tx_time = rx_ts_hi + ((RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8); // (rx_ts + RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8;
        dwt_setdelayedtrxtime(expected_final_tx_time); // 接收的是 dwt time 高 32 位

        // Calculate and record final_tx (add antenna delay)
        uint32_t final_tx = (uint32_t) (((((uint64_t) (expected_final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY) & 0xFFFFFFFF);

        // Prepare the final message
        uint8_t payload[12] = {0x00};
        memcpy(payload, &ranging_task_list[task_id].poll_tx, 4);
        memcpy(payload + 4, &rx_ts_lo, 4);
        memcpy(payload + 8, &final_tx, 4);
        tx_len = gen_ranging_exchange_msg(tx_buffer, RANGING_EXCHANGE_MSG_PAN_ID, src_id, module_config.module_id, task_id, RANGING_EXCHANGE_STAGE_FINAL, payload, sizeof(payload));

        // Send the final message
        dwt_writetxdata(tx_len, tx_buffer, 0);
        dwt_writetxfctrl(tx_len, 0, 1);
        dwt_starttx(DWT_START_TX_DELAYED);

        // Destroy the task
        destroy_ranging_task(task_id);
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void AnchorRXTimeoutCallback(const dwt_cb_data_t *data) {
    if (tx_flag) {
        // Add a new ranging task
        uint8_t new_task_id = add_ranging_task(1); // 已设置 stage

        // Prepare a poll message
        uint8_t poll_payload[] = {};
        tx_len = gen_ranging_exchange_msg(tx_buffer, RANGING_EXCHANGE_MSG_PAN_ID, RANGING_EXCHANGE_MSG_BROADCAST_ID, module_config.module_id, new_task_id, RANGING_EXCHANGE_STAGE_POLL, poll_payload, sizeof(poll_payload));

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

void AnchorEventHandler() {
    uint16_t module_id = module_config.module_id;

    // Regularly toggle the LED
    uint32_t current_tick_ms = GetSystickMs();
    if (current_tick_ms - last_tick_ms > module_config.ranging_exchange_poll_interval) {
        last_tick_ms = current_tick_ms;
        tx_flag = 1;

        if (toggle_flag) {
            TurnOffLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 0;
        } else {
            TurnOnLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 1;
        }
    }
}

/* Tag Related */
static void TagTXConfirmationCallback(const dwt_cb_data_t *data) {
    uint8_t task_id = tx_buffer[7];

    // Record the response transmission timestamp
    ranging_task_list[task_id].resp_tx = dwt_readtxtimestamplo32();
}

static void TagRXOkCallback(const dwt_cb_data_t *data) {
    if (data->datalength <= RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH) {
        dwt_readrxdata(rx_buffer, data->datalength, 0);
    }

    uint32_t rx_ts = dwt_readrxtimestamplo32();

    uint16_t pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
    uint16_t dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
    uint16_t src_id = rx_buffer[5] | (rx_buffer[6] << 8);
    uint8_t task_id = rx_buffer[7];
    uint8_t msg_type = rx_buffer[8];
    if (rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE && pan_id == RANGING_EXCHANGE_MSG_PAN_ID) {
        if (
            (dest_id == RANGING_EXCHANGE_MSG_BROADCAST_ID || dest_id == module_config.module_id) &&
            msg_type == RANGING_EXCHANGE_STAGE_POLL
        ) {
            if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received poll from %d for task %d.\n", src_id, task_id);

            // Record the poll reception timestamp
            ranging_task_list[task_id].poll_rx = rx_ts;

            // Prepare the response message
            uint8_t payload[] = {};
            tx_len = gen_ranging_exchange_msg(tx_buffer, RANGING_EXCHANGE_MSG_PAN_ID, src_id, module_config.module_id, task_id, RANGING_EXCHANGE_STAGE_RESPONSE, payload, sizeof(payload));

            // Send the response message
            dwt_writetxdata(tx_len, tx_buffer, 0);
            dwt_writetxfctrl(tx_len, 0, 1);
            dwt_starttx(DWT_START_TX_IMMEDIATE);

            // Update the task stage
            ranging_task_list[task_id].after_stage = RANGING_EXCHANGE_STAGE_RESPONSE;
        } else if (
            dest_id == module_config.module_id &&
            ranging_task_list[task_id].after_stage == RANGING_EXCHANGE_STAGE_RESPONSE &&
            msg_type == RANGING_EXCHANGE_STAGE_FINAL
        ) {
            if (module_config.ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received final from %d for task %d.\n", src_id, task_id);

            // Record the final reception timestamp
            uint32_t final_rx = rx_ts;

            // Extract the recorded timestamps
            uint32_t poll_rx = ranging_task_list[task_id].poll_rx;
            uint32_t resp_tx = ranging_task_list[task_id].resp_tx;

            // Unpack the timestamps
            memcpy(&ranging_task_list[task_id].poll_tx, rx_buffer + 9, 4);
            memcpy(&ranging_task_list[task_id].resp_rx, rx_buffer + 13, 4);
            memcpy(&ranging_task_list[task_id].final_tx, rx_buffer + 17, 4);
            uint32_t poll_tx = ranging_task_list[task_id].poll_tx;
            uint32_t resp_rx = ranging_task_list[task_id].resp_rx;
            uint32_t final_tx = ranging_task_list[task_id].final_tx;

            // Calculate time of flight and distance
            double Ra = (double) (resp_rx - poll_tx);
            double Rb = (double) (final_rx - resp_tx);
            double Da = (double) (final_tx - resp_rx);
            double Db = (double) (resp_tx - poll_rx);
            double tof = ((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db)) * DWT_TIME_UNITS;
            double distance = tof * SPEED_OF_LIGHT;

            // Print the result
//            debug_printf("   Tag: %lu, \t%lu, \t%lu\n", poll_rx, resp_tx, final_rx);
//            debug_printf("Anchor: %lu, \t%lu, \t%lu\n", poll_tx, resp_rx, final_tx);
//            debug_printf("    Ra: %lu\n", resp_rx - poll_tx);
//            debug_printf("    Rb: %lu\n", final_rx - resp_tx);
//            debug_printf("    Da: %lu\n", final_tx - resp_rx);
//            debug_printf("    Db: %lu\n", resp_tx - poll_rx);
//            debug_printf("%.4f\n", (float) distance);
            debug_printf("%d\n", (int) (distance * 10000));

            // Destroy the task
            destroy_ranging_task(task_id);
        }
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS); // TODO: 即使有 DWT_NO_SYNC_PTRS 也不能放前面, 放前面就卡死
}

static void TagRXTimeoutCallback(const dwt_cb_data_t *data) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

static void TagRXErrorCallback(const dwt_cb_data_t *data) {
    dwt_rxreset();
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);
}

void TagEventHandler() {
    uint16_t module_id = module_config.module_id;

    // Regularly toggle the LED
    uint32_t current_tick_ms = GetSystickMs();
    if (current_tick_ms - last_tick_ms > module_config.ranging_exchange_poll_interval) {
        last_tick_ms = current_tick_ms;

        if (toggle_flag) {
            TurnOffLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 0;
        } else {
            TurnOnLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 1;
        }
    }
}

void EventHandler() {
    uwb_mode_t mode = JudgeModeFromID(module_config.module_id);

    if (mode == ANCHOR) {
        AnchorEventHandler();
    } else if (mode == TAG) {
        TagEventHandler();
    } else {
        // Do nothing but wait for the module ID to be modified
    }
}
