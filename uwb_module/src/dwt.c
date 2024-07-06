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
#define RANGING_EXCHANGE_INTERVAL 100

#define RANGING_EXCHANGE_MAX_TASK_NUMBER 64 // < 256

#define RANGING_EXCHANGE_STAGE_IDLE 0x00
#define RANGING_EXCHANGE_STAGE_POLL 0x01
#define RANGING_EXCHANGE_STAGE_RESPONSE 0x02
#define RANGING_EXCHANGE_STAGE_FINAL 0x03

typedef struct _ranging_task_t {
    uint64_t poll_rx_ts, resp_tx_ts, final_rx_ts;
    uint32_t poll_tx_ts_32, resp_rx_ts_32, final_tx_ts_32;
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
    dwt_isr();
//    do {
//        dwt_isr();
//    } while (port_CheckEXT_IRQ() == 1);
    EXTI_ClearITPendingBit(DECAIRQ_EXTI);
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
//        dwt_setcallbacks(&AnchorTXCallback, &AnchorRXCallback, NULL, NULL);
        // TODO
    } else if (mode == TAG) {
        dwt_setdblrxbuffmode(1); // TODO: 双缓冲, 开关似乎暂时不影响其他功能

        dwt_setcallbacks(&TagTXConfirmationCallback, &TagRXOkCallback, &TagRXTimeoutCallback, &TagRXErrorCallback);
        dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | DWT_INT_RFTO | DWT_INT_RXPTO | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFSL | DWT_INT_SFDT, 1);

//        dwt_setrxtimeout(RX_TIMEOUT_UUS); // TODO: 开了则一堆 timeout 夹杂几个成功, 不开则默认等待时间较长, 暂时都能用; Update: 开了似乎会把 TFRS 中断冲掉, 用中断了似乎也确实没必要开 timeout.
        dwt_rxenable(DWT_START_RX_IMMEDIATE); // TODO: 开始时启动一下, 后面全部由在中断处理结束后重启
    }

//    dwt_setpreambledetecttimeout(PRE_TIMEOUT);

    debug_printf("[Info] DW1000 initialized.\n");

    // Clear the ranging task list
    init_ranging_task_list();

    return 0;
}


uint64_t get_tx_timestamp(void) {
    uint8_t data[5];
    uint64_t timestamp = 0;
    dwt_readtxtimestamp(data);
    for (int i = 4; i >= 0; i --) {
        timestamp <<= 8;
        timestamp |= data[i];
    }
    return timestamp;
}

uint64_t get_rx_timestamp(void) {
    uint8_t data[5];
    uint64_t timestamp = 0;
    dwt_readrxtimestamp(data);
    for (int i = 4; i >= 0; i --) {
        timestamp <<= 8;
        timestamp |= data[i];
    }
    return timestamp;
}

/* Temporary variables & Interruption */
static uint32_t status_reg = 0;
volatile uint8_t rx_buffer[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH] = {0x00};
volatile uint8_t rx_flag = 0;
volatile uint8_t tx_flag = 0;

/* State variables */
static uint32_t last_tick_ms = 0;
static uint8_t toggle_flag = 0;

/* Anchor Related */
static void AnchorTXConfirmationCallback(const dwt_cb_data_t *data) {

}

static void AnchorRXOkCallback(const dwt_cb_data_t *data) {

}

static void AnchorRXTimeoutCallback(const dwt_cb_data_t *data) {

}

static void AnchorRXErrorCallback(const dwt_cb_data_t *data) {

}

void AnchorEventHandler() {
    uint16_t module_id = module_config.module_id;
    uint8_t ranging_exchange_debug_output = module_config.ranging_exchange_debug_output;

    // Regularly send poll messages
    uint32_t current_tick_ms = GetSystickMs();
    if (current_tick_ms - last_tick_ms > RANGING_EXCHANGE_INTERVAL) {
        last_tick_ms = current_tick_ms;

        if (toggle_flag) {
            TurnOffLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 0;
        } else {
            TurnOnLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 1;
        }

        // Prepare a poll message
        uint8_t new_task_id = add_ranging_task(1);
        uint8_t poll_frame[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH];
        uint8_t poll_payload[] = {};
        uint16_t poll_frame_len = gen_ranging_exchange_msg(poll_frame, RANGING_EXCHANGE_MSG_PAN_ID, RANGING_EXCHANGE_MSG_BROADCAST_ID, module_id, new_task_id, RANGING_EXCHANGE_STAGE_POLL, poll_payload, sizeof(poll_payload));

        // Send the poll message
        dwt_writetxdata(poll_frame_len, poll_frame, 0);
        dwt_writetxfctrl(poll_frame_len, 0, 1);
        int res = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
        if (res == DWT_SUCCESS) {
            // Wait for the poll message to be sent
            while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_TXFRS)));

            sleep_ms(5); // 3
            /*
             * 见后 TODO [A]
             * 这里 1 ms 还不太够.
             */

            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

            if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Poll message sent for task %d.\n", new_task_id);

            // Add the task to the task list
            uint64_t poll_tx_ts = get_tx_timestamp();
            memcpy(&ranging_task_list[ranging_task_list_head].poll_tx_ts_32, &poll_tx_ts, 4);
        } else {
            if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Failed to send poll for task %d.\n", new_task_id);
            destroy_ranging_task(new_task_id);
        }
    }

    // Listening
    dwt_setrxtimeout(RX_TIMEOUT_UUS);
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));
    if (status_reg & SYS_STATUS_RXFCG) {
        // Clear flags
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

        // Read response frame
        uint16_t resp_frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (resp_frame_len <= RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH) {
            dwt_readrxdata((uint8_t *) rx_buffer, resp_frame_len, 0);
        }

        // Validate whether the response is an expected resp_frame
        uint16_t resp_pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
        uint16_t resp_dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
        uint16_t resp_src_id = rx_buffer[5] | (rx_buffer[6] << 8);
        uint8_t resp_task_id = rx_buffer[7];
        uint8_t resp_msg_type = rx_buffer[8];
        if (
            rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
            resp_pan_id == RANGING_EXCHANGE_MSG_PAN_ID &&
            resp_dest_id == module_id &&
            ranging_task_list[resp_task_id].after_stage == RANGING_EXCHANGE_STAGE_POLL &&
            resp_msg_type == RANGING_EXCHANGE_STAGE_RESPONSE
        ) {
            if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received response from %d for task %d.\n", resp_src_id, resp_task_id);

            // Record the response timestamp
            uint64_t resp_rx_ts = get_rx_timestamp();
            memcpy(&ranging_task_list[resp_task_id].resp_rx_ts_32, &resp_rx_ts, 4);

            // Set expected time of transmission
            uint32_t expected_final_tx_time = (resp_rx_ts + RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8;
            dwt_setdelayedtrxtime(expected_final_tx_time);

            // Calculate and record final_tx_ts (add antenna delay)
            uint64_t final_tx_ts = (((uint64_t) (expected_final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;
            memcpy(&ranging_task_list[resp_task_id].final_tx_ts_32, &final_tx_ts, 4);

            // Prepare the final message
            uint8_t final_frame[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH];
            uint8_t final_payload[12] = {0x00};
            memcpy(final_payload, &ranging_task_list[resp_task_id].poll_tx_ts_32, 4);
            memcpy(final_payload + 4, &ranging_task_list[resp_task_id].resp_rx_ts_32, 4);
            memcpy(final_payload + 8, &ranging_task_list[resp_task_id].final_tx_ts_32, 4);
            uint16_t final_frame_len = gen_ranging_exchange_msg(final_frame, RANGING_EXCHANGE_MSG_PAN_ID, resp_src_id, module_id, resp_task_id, RANGING_EXCHANGE_STAGE_FINAL, final_payload, sizeof(final_payload));

            // Send the final message
            dwt_writetxdata(final_frame_len, final_frame, 0);
            dwt_writetxfctrl(final_frame_len, 0, 1);
            int res = dwt_starttx(DWT_START_TX_DELAYED);
            if (res == DWT_SUCCESS) {
                // Wait for the poll message to be sent
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                /*
                 * 见后 TODO [A]
                 * 不知道为什么这里就不需要.
                 */
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Final message sent for task %d.\n", resp_task_id);
            } else {
                if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Failed to send final for task %d.\n", resp_task_id);
            }

            // Destroy the task
            destroy_ranging_task(resp_task_id);
        }
    } else {
        // Clear RX error/timeout flags
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

        // Reset RX
        dwt_rxreset();
    }
}

/* Tag Related */
static void TagTXConfirmationCallback(const dwt_cb_data_t *data) { // TODO: 暂时看不用
    debug_printf("Tx Confirmation. len = %d.\n", data->datalength);
}

static void TagRXOkCallback(const dwt_cb_data_t *data) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);

    if (data->datalength <= RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH) {
        dwt_readrxdata(rx_buffer, data->datalength, 0);
    }

    rx_flag = 1; // TODO: if already 1

    debug_printf("Rx Ok.\n");
}

static void TagRXTimeoutCallback(const dwt_cb_data_t *data) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);

    debug_printf("Rx Timeout.\n");
}

static void TagRXErrorCallback(const dwt_cb_data_t *data) {
    dwt_rxenable(DWT_START_RX_IMMEDIATE | DWT_NO_SYNC_PTRS);

    debug_printf("Rx Error.\n");
}

void TagEventHandler() {
    uint16_t module_id = module_config.module_id;
    uint8_t ranging_exchange_debug_output = module_config.ranging_exchange_debug_output;

    // Regularly toggle the LED
    uint32_t current_tick_ms = GetSystickMs();
    if (current_tick_ms - last_tick_ms > RANGING_EXCHANGE_INTERVAL) {
        last_tick_ms = current_tick_ms;

        if (toggle_flag) {
            TurnOffLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 0;
        } else {
            TurnOnLED((led_t) JudgeModeFromID(module_id));
            toggle_flag = 1;
        }
    }

    // Listening
    if (rx_flag) {
        debug_printf("Rx Flag: %d %d %d %d %d\n", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4], rx_buffer[5]);
        rx_flag = 0;

        uint8_t frame[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

        // Send the response message
        dwt_writetxdata(6, frame, 0);
        dwt_writetxfctrl(6, 0, 1);
        dwt_starttx(DWT_START_TX_IMMEDIATE);
    }

//    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));
//    if (status_reg & SYS_STATUS_RXFCG) {
//        // Clear flags
//        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS); // TODO: 这里不清 SYS_STATUS_TXFRS 的话所的距离都是几十万, 可能就是几次发送间影响了
//
//        // Read response frame
//        uint16_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK; // RX_FINFO_RXFL_MASK_1023;
//        if (frame_len <= RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH) {
//            dwt_readrxdata(rx_buffer, frame_len, 0);
//        }
//
//        // Validate the message
//        uint16_t pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
//        uint16_t dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
//        uint16_t src_id = rx_buffer[5] | (rx_buffer[6] << 8);
//        uint8_t task_id = rx_buffer[7];
//        uint8_t msg_type = rx_buffer[8];
//        if (rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE && pan_id == RANGING_EXCHANGE_MSG_PAN_ID) {
//            if (
//                (dest_id == RANGING_EXCHANGE_MSG_BROADCAST_ID || dest_id == module_id) &&
////                !is_task_id_occupied(task_id) && // 不判断, Anchor 重复发送该 poll 即代表已覆盖任务
//                msg_type == RANGING_EXCHANGE_STAGE_POLL
//            ) {
//                if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received poll from %d for task %d.\n", src_id, task_id);
//
//                // Prepare the response message
//                uint8_t resp_frame[RANGING_EXCHANGE_MSG_MAX_FRAME_LENGTH];
//                uint8_t resp_payload[] = {};
//                uint16_t resp_frame_len = gen_ranging_exchange_msg(resp_frame, RANGING_EXCHANGE_MSG_PAN_ID, src_id, module_id, task_id, RANGING_EXCHANGE_STAGE_RESPONSE, resp_payload, sizeof(resp_payload));
//
//                // Send the response message
//                dwt_writetxdata(resp_frame_len, resp_frame, 0);
//                dwt_writetxfctrl(resp_frame_len, 0, 1);
//                int res = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
//                if (res == DWT_SUCCESS) {
//                    // Wait for the poll message to be sent
//                    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
//
//                    sleep_ms(2); // 1
//                    /*
//                     * TODO [A]
//                     * Tag 中在 while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_TXFRS))); 后不能直接跟 dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
//                     * (包括回到前面的 dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);)
//                     * 会运行一小段时间后卡死, 实验发现必须在二者之间进行一些耗时操作, 例如 sleep_ms(1) 或者调试输出. 原因未知.
//                     */
//
//                    if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Response message sent for task %d.\n", task_id);
//
//                    // Record the poll reception and response transmission timestamps
//                    uint64_t poll_rx_ts = get_rx_timestamp();
//                    ranging_task_list[task_id].poll_rx_ts = poll_rx_ts;
//                    uint64_t resp_tx_ts = get_tx_timestamp();
//                    ranging_task_list[task_id].resp_tx_ts = resp_tx_ts;
//
//                    // Update the task stage
//                    ranging_task_list[task_id].after_stage = RANGING_EXCHANGE_STAGE_RESPONSE;
//                } else {
//                    if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Failed to send response.\n");
//                    destroy_ranging_task(task_id);
//                }
//            } else if (
//                dest_id == module_id &&
//                ranging_task_list[task_id].after_stage == RANGING_EXCHANGE_STAGE_RESPONSE &&
//                msg_type == RANGING_EXCHANGE_STAGE_FINAL
//            ) {
//                if (ranging_exchange_debug_output) debug_printf("[Debug] Ranging exchanging: Received final from %d for task %d.\n", src_id, task_id);
//
//                // Record the final reception timestamp and extract the recorded timestamps
//                uint64_t final_rx_ts = get_rx_timestamp();
//                ranging_task_list[task_id].final_rx_ts = final_rx_ts;
//                uint64_t poll_rx_ts = ranging_task_list[task_id].poll_rx_ts;
//                uint64_t resp_tx_ts = ranging_task_list[task_id].resp_tx_ts;
//
//                // Unpack the timestamps
//                memcpy(&ranging_task_list[task_id].poll_tx_ts_32, rx_buffer + 9, 4);
//                memcpy(&ranging_task_list[task_id].resp_rx_ts_32, rx_buffer + 13, 4);
//                memcpy(&ranging_task_list[task_id].final_tx_ts_32, rx_buffer + 17, 4);
//                uint32_t poll_tx_ts = ranging_task_list[task_id].poll_tx_ts_32;
//                uint32_t resp_rx_ts = ranging_task_list[task_id].resp_rx_ts_32;
//                uint32_t final_tx_ts = ranging_task_list[task_id].final_tx_ts_32;
//
//                // Calculate time of flight and distance
//                double Ra = (double) (resp_rx_ts - poll_tx_ts);
//                double Rb = (double) (final_rx_ts - resp_tx_ts);
//                double Da = (double) (final_tx_ts - resp_rx_ts);
//                double Db = (double) (resp_tx_ts - poll_rx_ts);
//                double tof = ((int64_t) ((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db))) * DWT_TIME_UNITS;
//                double distance = tof * SPEED_OF_LIGHT;
//
//                // Print the result
////                debug_printf("[Info] Ranging exchanging: Distance = %.4f m.\n", (float) distance);
//                debug_printf("%.4f\n", (float) distance);
//
//                // Destroy the task
//                destroy_ranging_task(task_id);
//            }
//        }
//    } else {
//        // Clear RX error/timeout flags
//        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
//
//        // Reset RX
//        dwt_rxreset();
//    }
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
