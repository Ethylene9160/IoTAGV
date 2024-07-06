#ifndef UWB_TRANSCEIVER_H
#define UWB_TRANSCEIVER_H

#ifdef c_plus_plus
extern "C" {
#endif

#include "stm32f10x.h"
#include <stdio.h>
#include <cstdint>
#include <string.h>
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"
#include "port.h"
#include "lcd_oled.h"
#include "trilateration.h"
#include <math.h>

#include "deca_port.h"
#include "stm32_eval.h"

#define RNG_DELAY_MS 5
#define RX_BUF_LEN 24
#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define ALL_MSG_TAG_IDX 3
#define FINAL_MSG_POLL_TX_TS_IDX 10
#define FINAL_MSG_RESP_RX_TS_IDX 14
#define FINAL_MSG_FINAL_TX_TS_IDX 18
#define FINAL_MSG_TS_LEN 4
#define SPEED_OF_LIGHT 299702547
#define UUS_TO_DWT_TIME 65536

static dwt_config_t config = {
    2, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 9, 9, 1, DWT_BR_110K, DWT_PHRMODE_STD, (1025 + 64 - 32)
};

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0x0, 0xDE, 'W', 'A', 'V', 'E', 0x21, 0, 0};
static uint8 rx_resp_msg[] = {0x41, 0x88, 0, 0x0, 0xDE, 'V', 'E', 'W', 'A', 0x10, 0x02, 0, 0, 0, 0};
static uint8 tx_final_msg[] = {0x41, 0x88, 0, 0x0, 0xDE, 'W', 'A', 'V', 'E', 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8 distance_msg[] = {0x41, 0x88, 0, 0x0, 0xDE, 'W', 'A', 'V', 'E', 0xAA, 0, 0, 0, 0, 0};

static uint8 rx_buffer[RX_BUF_LEN];
static uint32_t status_reg = 0;
static uint64_t poll_tx_ts, resp_rx_ts, final_tx_ts;

static uint64_t get_tx_timestamp_u64(void);
static uint64_t get_rx_timestamp_u64(void);
static void final_msg_get_ts(const uint8 *ts_field, uint32 *ts);
static void final_msg_set_ts(uint8 *ts_field, uint64_t ts);

void init(void) {
    peripherals_init();
    printf("Initializing DW1000...\r\n");
    reset_DW1000();
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADUCODE) == -1) {
        printf("DW1000 initialization failed!\r\n");
        while (1) {
            STM_EVAL_LEDOn(LED1);
            deca_sleep(100);
            STM_EVAL_LEDOff(LED1);
            deca_sleep(100);
        }
    }
    spi_set_rate_high();
    dwt_configure(&config);
    dwt_setleds(1);
    dwt_setrxantennadelay(32950);
    dwt_settxantennadelay(0);
    printf("Initialization complete.\r\n");
}

void send_distance_info(uint8 tag_id) {
    tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    tx_poll_msg[ALL_MSG_TAG_IDX] = tag_id;
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
    dwt_writetxfctrl(sizeof(tx_poll_msg), 0);
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    dwt_rxenable(0);
}

void receive_distance_info(uint8 tag_id) {
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))) {}
    if (status_reg & SYS_STATUS_RXFCG) {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
        uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (frame_len <= RX_BUF_LEN) {
            dwt_readrxdata(rx_buffer, frame_len, 0);
        }
        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0) {
            poll_tx_ts = get_tx_timestamp_u64();
            resp_rx_ts = get_rx_timestamp_u64();
            uint32 final_tx_time = (resp_rx_ts + (2800 * UUS_TO_DWT_TIME)) >> 8;
            dwt_setdelayedtrxtime(final_tx_time);
            final_tx_ts = (((uint64_t)(final_tx_time & 0xFFFFFFFE)) << 8) + 0;
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
            final_msg_set_ts(&tx_final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);
            tx_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
            tx_final_msg[ALL_MSG_TAG_IDX] = tag_id;
            dwt_writetxdata(sizeof(tx_final_msg), tx_final_msg, 0);
            dwt_writetxfctrl(sizeof(tx_final_msg), 0);
            dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
            while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR))) {}
            if (status_reg & SYS_STATUS_RXFCG) {
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
                frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
                if (frame_len <= RX_BUF_LEN) {
                    dwt_readrxdata(rx_buffer, frame_len, 0);
                }
                if (memcmp(rx_buffer, distance_msg, ALL_MSG_COMMON_LEN) == 0) {
                    double tof = ((resp_rx_ts - poll_tx_ts) * (double)(final_rx_ts - resp_tx_ts) - (final_tx_ts - resp_rx_ts) * (double)(resp_tx_ts - poll_rx_ts)) / ((double)(resp_rx_ts - poll_tx_ts) + (double)(final_rx_ts - resp_tx_ts) + (double)(final_tx_ts - resp_rx_ts) + (double)(resp_tx_ts - poll_rx_ts));
                    double distance = tof * SPEED_OF_LIGHT;
                    printf("Distance: %f meters\r\n", distance);
                }
            } else {
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
            }
        }
    } else {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    }
}

static uint64 get_tx_timestamp_u64(void) {
    uint8 ts_tab[5];
    uint64 ts = 0;
    dwt_readtxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

static uint64 get_rx_timestamp_u64(void) {
    uint8 ts_tab[5];
    uint64 ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

static void final_msg_get_ts(const uint8 *ts_field, uint32 *ts) {
    *ts = 0;
    for (int i = 0; i < FINAL_MSG_TS_LEN; i++) {
        *ts += ts_field[i] << (i * 8);
    }
}

static void final_msg_set_ts(uint8 *ts_field, uint64 ts) {
    for (int i = 0; i < FINAL_MSG_TS_LEN; i++) {
        ts_field[i] = (uint8) ts;
        ts >>= 8;
    }
}



#ifdef c_plus_plus
}
#endif

#endif //UWB_TRANSCEIVER_H
