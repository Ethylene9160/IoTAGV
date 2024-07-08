#ifndef UWB_MODULE_DWT_H
#define UWB_MODULE_DWT_H

#include "stdint.h"

#include "deca_port.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"

#include "uwb_module_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ANCHOR,
    TAG,
    UNDEFINED
} uwb_mode_t;

uwb_mode_t JudgeModeFromID(uint8_t module_id);


static dwt_config_t dwt_config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_1024,   /* Preamble length. */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    1,               /* Use non-standard SFD (Boolean) */
    DWT_BR_110K,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

#define DWT_START_RX_IMMEDIATE 0

#define UUS_TO_DWT_TIME 65536

//#define TX_ANT_DLY 0
//#define RX_ANT_DLY 32950
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

//#define POLL_RX_TO_RESP_TX_DLY_UUS 8000
#define RESP_RX_TO_FINAL_TX_DLY_UUS 8000
#define RX_TIMEOUT_UUS 5000

#define SPEED_OF_LIGHT 299702547


uint8_t Initialize();

void EventHandler();

#ifdef __cplusplus
}
#endif

#endif