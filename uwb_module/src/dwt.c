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

uint64_t GetTXTimeStamp(void) {
    uint8_t data[5];
    uint64_t timestamp = 0;
    dwt_readtxtimestamp(data);
    for (int i = 4; i >= 0; i --) {
        timestamp <<= 8;
        timestamp |= data[i];
    }
    return timestamp;
}

uint64_t GetRXTimeStamp(void) {
    uint8_t data[5];
    uint64_t timestamp = 0;
    dwt_readrxtimestamp(data);
    for (int i = 4; i >= 0; i --) {
        timestamp <<= 8;
        timestamp |= data[i];
    }
    return timestamp;
}

uint8_t InitDW1000(uwb_mode_t mode) {
    if (mode == UNDEFINED) {
        debug_printf("Invalid module ID.\n");
        return 1;
    }

    reset_DW1000();
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        debug_printf("DW1000 initialization failed.\n");
        return 1;
    }
    spi_set_rate_high();

    dwt_configure(&dwt_config);

    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    if (mode == ANCHOR) {
        dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
        dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);
//        dwt_setrxtimeout(0);
    }
//    dwt_setpreambledetecttimeout(PRE_TIMEOUT);

    debug_printf("DW1000 initialized.\n");

    return 0;
}

#define MAX_FRAME_LEN 64
#define PAN_ID 0x2333
#define BROADCAST_ID 0xFFFF
#define RANGING_TASK_INTERVAL 1000

static uint8_t task_id_seq = 0; // TODO, queuing tasks
static uint32_t status_reg = 0;

static uint8 rx_buffer[MAX_FRAME_LEN] = {0x00};

void AnchorEventHandler(uint16_t module_id) {
    // Send poll message
    uint8_t poll_frame[MAX_FRAME_LEN];
    uint8_t poll_payload[] = {};
    uint16_t poll_frame_len = gen_ranging_exchange_msg(poll_frame, PAN_ID, BROADCAST_ID, module_id, task_id_seq, RANGING_EXCHANGE_MSG_TYPE_POLL, poll_payload, sizeof(poll_payload));

    dwt_writetxdata(poll_frame_len, poll_frame, 0);
    dwt_writetxfctrl(poll_frame_len, 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    dwt_rxenable(DWT_START_RX_IMMEDIATE); // TODO: 理论应该自动开
    debug_printf("Sent poll.\n");

    // Wait for response
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

    // Read response, calculate and send final message
    if (status_reg & SYS_STATUS_RXFCG) {
        // Clear flags to prepare for next reception or transmission
        dwt_write32bitreg(RX_FINFO_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

        // Read response frame
        uint16_t resp_frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (resp_frame_len <= MAX_FRAME_LEN) {
            dwt_readrxdata(rx_buffer, resp_frame_len, 0);
        }

        // Validate whether the response is an expected resp_frame
        uint16_t resp_pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
        uint16_t resp_dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
        uint16_t resp_src_id = rx_buffer[5] | (rx_buffer[6] << 8);
        uint8_t resp_task_id = rx_buffer[7];
        uint8_t resp_msg_type = rx_buffer[8];
        if ( // TODO
            rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
            resp_pan_id == PAN_ID &&
            resp_dest_id == module_id &&
            resp_task_id == task_id_seq && // TODO, queuing tasks, 查找 task 表
            resp_msg_type == RANGING_EXCHANGE_MSG_TYPE_RESPONSE
        ) {
            debug_printf("Received response from %d.\n", resp_src_id);

            uint64_t poll_tx_ts = GetTXTimeStamp(); // 上次发 poll 的时间, TODO, queuing tasks 时需要提前记录下来
            uint64_t resp_rx_ts = GetRXTimeStamp(); // 收到 resp 的时间, TODO, same as above

            // Set expected time of transmission
            uint32_t expected_final_tx_time = (resp_rx_ts + RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8;
            dwt_setdelayedtrxtime(expected_final_tx_time);

            // Calculate final_tx_ts (add antenna delay)
            uint64_t final_tx_ts = (((uint64_t) (expected_final_tx_time & 0xFFFFFFFE)) << 8) + TX_ANT_DLY;

            // Prepare and send final message
            uint8_t final_frame[MAX_FRAME_LEN];
            uint8_t final_payload[12] = {0x00};
            memcpy(final_payload, &poll_tx_ts, 4); // only lower 4 bytes
            memcpy(final_payload + 4, &resp_rx_ts, 4);
            memcpy(final_payload + 8, &final_tx_ts, 4);
            uint16_t final_frame_len = gen_ranging_exchange_msg(final_frame, PAN_ID, resp_src_id, module_id, resp_task_id, RANGING_EXCHANGE_MSG_TYPE_FINAL, final_payload, sizeof(final_payload));

            dwt_writetxdata(final_frame_len, final_frame, 0);
            dwt_writetxfctrl(final_frame_len, 0);
            int res = dwt_starttx(DWT_START_TX_DELAYED);

            if (res == DWT_SUCCESS) {
                // Wait for TX completion
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

                // Prepare for next task
                task_id_seq ++; // TODO, queuing tasks
            }
        }
    } else {
        // Clear RX error/timeout flags
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

        // Reset RX
        dwt_rxreset();
    }

    sleep_ms(RANGING_TASK_INTERVAL); // slow down for debugging
}

static uint64_t poll_rx_ts, resp_tx_ts, final_rx_ts;

void TagEventHandler(uint16_t module_id) {
    // Disable RX timeout
    dwt_setrxtimeout(0);

    // Activate reception immediately
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    // Wait for poll message
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

    if (status_reg & SYS_STATUS_RXFCG) {
        // Clear flags to prepare for next reception or transmission
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS); // TODO, HERE! SYS_STATUS_TXFRS!

        // Read poll frame
        uint16_t poll_frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
        if (poll_frame_len <= MAX_FRAME_LEN) {
            dwt_readrxdata(rx_buffer, poll_frame_len, 0);
        }

        // Validate whether the poll is an expected poll_frame
        uint16_t poll_pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
        uint16_t poll_dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
        uint16_t poll_src_id = rx_buffer[5] | (rx_buffer[6] << 8);
        uint8_t poll_task_id = rx_buffer[7];
        uint8_t poll_msg_type = rx_buffer[8];
        if (
            rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
            poll_pan_id == PAN_ID &&
            (poll_dest_id == BROADCAST_ID || poll_dest_id == module_id) &&
            poll_msg_type == RANGING_EXCHANGE_MSG_TYPE_POLL
        ) {
            debug_printf("Received poll from %d.\n", poll_src_id);

            task_id_seq = poll_task_id; // TODO, queuing tasks, 此处是收到 poll 后才确认 id, 以此 id 等待 final

            poll_rx_ts = GetRXTimeStamp(); // 收到 poll 的时间, TODO: queuing tasks 时需分 task_id 记录下来

//            debug_printf("Poll RX timestamp: %Lu.\n", poll_rx_ts);
//            debug_printf("Expected TX timestamp: %Lu.\n", poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME));

            // Set expected time of transmission
//            uint32_t expected_resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
//            dwt_setdelayedtrxtime(expected_resp_tx_time);

            // Set expected delay and timeout for final message reception
            dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
            dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

            // Prepare and send response message
            uint8_t resp_frame[MAX_FRAME_LEN];
            uint8_t resp_payload[] = {0x02, 0x00, 0x00};
            uint16_t resp_frame_len = gen_ranging_exchange_msg(resp_frame, PAN_ID, poll_src_id, module_id, poll_task_id, RANGING_EXCHANGE_MSG_TYPE_RESPONSE, resp_payload, sizeof(resp_payload));

            dwt_writetxdata(resp_frame_len, resp_frame, 0);
            dwt_writetxfctrl(resp_frame_len, 0);
//            int res = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
            int res = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

            if (res == DWT_ERROR) {
                // Failed to start transmission, TODO 有没有什么要做的
                debug_printf("Failed to transmit response.\n");
                return;
            }

            // Wait for final message
            while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

            if (status_reg & SYS_STATUS_RXFCG) {
                // Clear flags to prepare for next reception or transmission
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);

                // Read final frame
                uint16_t final_frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
                if (final_frame_len <= MAX_FRAME_LEN) {
                    dwt_readrxdata(rx_buffer, final_frame_len, 0);
                }

                // Validate whether the final is an expected final_frame
                uint16_t final_pan_id = rx_buffer[1] | (rx_buffer[2] << 8);
                uint16_t final_dest_id = rx_buffer[3] | (rx_buffer[4] << 8);
                uint16_t final_src_id = rx_buffer[5] | (rx_buffer[6] << 8);
                uint8_t final_task_id = rx_buffer[7];
                uint8_t final_msg_type = rx_buffer[8];
                if (
                    rx_buffer[0] == RANGING_EXCHANGE_MSG_SYNC_BYTE &&
                    final_pan_id == PAN_ID &&
                    final_dest_id == module_id &&
                    final_task_id == task_id_seq && // TODO, queuing tasks, 查找 task 表
                    final_msg_type == RANGING_EXCHANGE_MSG_TYPE_FINAL
                ) {
                    debug_printf("Received final from %d.\n", final_src_id);

                    resp_tx_ts = GetTXTimeStamp(); // 发送 resp 的时间, TODO, queuing tasks 时需要提前记录下来
                    final_rx_ts = GetRXTimeStamp(); // 收到 final 的时间

                    // Unpack timestamps recorded by the anchor
                    uint32_t poll_tx_ts, resp_rx_ts, final_tx_ts;
                    memcpy(&poll_tx_ts, rx_buffer + 9, 4);
                    memcpy(&resp_rx_ts, rx_buffer + 13, 4);
                    memcpy(&final_tx_ts, rx_buffer + 17, 4);

                    // Calculate time of flight and distance
                    double Ra = (double) (resp_rx_ts - poll_tx_ts);
                    double Rb = (double) (final_rx_ts - resp_tx_ts);
                    double Da = (double) (final_tx_ts - resp_rx_ts);
                    double Db = (double) (resp_tx_ts - poll_rx_ts);
                    double tof = ((int64_t) ((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db))) * DWT_TIME_UNITS;
                    double distance = tof * SPEED_OF_LIGHT;

                    // Output distance
                    debug_printf("Distance: %.2f m.\n", (float) distance);
                }
            } else {
                // Clear RX error/timeout flags
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

                // Reset RX
                dwt_rxreset();
            }
        }
    } else {
        // Clear RX error/timeout flags
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);

        // Reset RX
        dwt_rxreset();
    }
}

/*
 *
 */