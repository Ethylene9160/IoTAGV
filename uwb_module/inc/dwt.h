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

#define DWT_START_RX_IMMEDIATE 0

#define UUS_TO_DWT_TIME 65536

//#define TX_ANT_DLY 0
//#define RX_ANT_DLY 32950
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

#define TX_TO_RX_DLY_UUS 60
//#define POLL_TX_TO_RESP_RX_DLY_UUS 150 // 150 // TODO 设定预期发完 poll 后多久开 rx 等 resp
//#define RESP_TX_TO_FINAL_RX_DLY_UUS 500 // 500 // TODO 设定预期发完 resp 后多久开 rx 等 final

//#define POLL_RX_TO_RESP_TX_DLY_UUS 10000 // 2800 // TODO 设定预期从接收到 poll 消息到发送 resp 消息的时间, 例程 2600
#define RESP_RX_TO_FINAL_TX_DLY_UUS 10000 // 3100 // TODO 设定预期从接收到响应消息到发送最终消息的时间, 不能太小，可能要调整
#define RX_TIMEOUT_UUS 5000 // 2700

#define PRE_TIMEOUT 8

#define SPEED_OF_LIGHT 299702547


uint8_t InitDW1000();

void EventHandler();

#ifdef __cplusplus
}
#endif

#endif