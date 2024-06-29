#ifndef UWB_MODULE_DWT_H
#define UWB_MODULE_DWT_H

#include "stdint.h"

#include "deca_port.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_sleep.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ANCHOR,
    TAG,
    UNDEFINED
} uwb_mode_t;

uwb_mode_t JudgeModeFromID(uint16_t module_id);

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

#define TX_ANT_DLY 0
#define RX_ANT_DLY 32950

#define POLL_TX_TO_RESP_RX_DLY_UUS 150
#define RESP_RX_TIMEOUT_UUS 2700

#define PRE_TIMEOUT 8


void InitDW1000(uwb_mode_t mode);

void AnchorEventHandler(uint16_t module_id);
void TagEventHandler(uint16_t module_id);

#ifdef __cplusplus
}
#endif

#endif