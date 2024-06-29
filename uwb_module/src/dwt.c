#include "dwt.h"

#include "systick.h"
#include "gpio.h"

#include "tiny_io.h"


uwb_mode_t JudgeModeFromID(uint16_t module_id) {
    if (module_id >= 0x0000 && module_id <= 0x0FFF) {
        return ANCHOR;
    } else if (module_id >= 0x1000 && module_id <= 0x1FFF) {
        return TAG;
    } else {
        return UNDEFINED;
    }
}

void InitDW1000(uwb_mode_t mode) {
    if (mode == UNDEFINED) {
        debug_printf("Invalid module ID.\n");
        return;
    }

    reset_DW1000();
    spi_set_rate_low();
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        debug_printf("DW1000 initialization failed.\n");
        while (1) {
            TurnOnLED(LED_3);
            SleepMs(500);
            TurnOffLED(LED_3);
            SleepMs(500);
        }
    }
    spi_set_rate_high();

    dwt_configure(&dwt_config);

    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    if (mode == TAG) {
        dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
        dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);
    }
//    dwt_setpreambledetecttimeout(PRE_TIMEOUT);

    debug_printf("DW1000 initialized.\n");
}

#define FRAME_LEN_MAX 127
static uint8 rx_buffer[FRAME_LEN_MAX];
static uint32 status_reg = 0;
static uint16 frame_len = 0;

#define DWT_START_RX_IMMEDIATE  0

void AnchorEventHandler(uint16_t module_id) {
    // TODO

    // Test Simple RX
    /* Clear local RX buffer to avoid having leftovers from previous receptions  This is not necessary but is included here to aid reading
     * the RX buffer.
     * This is a good place to put a breakpoint. Here (after first time through the loop) the local status register will be set for last event
     * and if a good receive has happened the data buffer will have the data in it, and frame_len will be set to the length of the RX frame. */
    for (int i = 0; i < FRAME_LEN_MAX; i ++) {
        rx_buffer[i] = 0;
    }

    /* Activate reception immediately. See NOTE 3 below. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    /* Poll until a frame is properly received or an error/timeout occurs. See NOTE 4 below.
     * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
     * function to access it. */
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

    if (status_reg & SYS_STATUS_RXFCG) {
        /* A frame has been received, copy it to our local buffer. */
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
        if (frame_len <= FRAME_LEN_MAX) {
            dwt_readrxdata(rx_buffer, frame_len, 0);
        }

        debug_printf("%s\n", rx_buffer);

        /* Clear good RX frame event in the DW1000 status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
    } else {
        /* Clear RX error events in the DW1000 status register. */
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    }
}

/* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0xC5 for a blink).
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 1 below.
 *     - byte 10/11: frame check-sum, automatically set by DW1000.  */
static uint8 tx_msg[] = {0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0};

#define BLINK_FRAME_SN_IDX 1
#define TX_DELAY_MS 1000

void TagEventHandler(uint16_t module_id) {
    // TODO

    // Test Simple TX
    /* Write frame data to DW1000 and prepare transmission. See NOTE 4 below.*/
    dwt_writetxdata(sizeof(tx_msg), tx_msg, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(tx_msg), 0); /* Zero offset in TX buffer, no ranging. */

    /* Start transmission. */
    dwt_starttx(DWT_START_TX_IMMEDIATE);

    /* Poll DW1000 until TX frame sent event set. See NOTE 5 below.
     * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
     * function to access it.*/
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));

    /* Clear TX frame sent event. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    /* Execute a delay between transmissions. */
    sleep_ms(TX_DELAY_MS);

    /* Increment the blink frame sequence number (modulo 256). */
    tx_msg[BLINK_FRAME_SN_IDX] ++;
}
