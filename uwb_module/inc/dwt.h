#ifndef UWB_MODULE_DWT_H
#define UWB_MODULE_DWT_H

#include "stdint.h"

#include "deca_port.h"
#include "deca_device_api.h"
#include "linked_list.h"
#include "adc.h"

#include "uwb_module_config.h"

#ifdef __cplusplus
extern "C" {
#endif
#define DISTANCE_FILTER_LENGTH 4

typedef enum {
    ANCHOR,
    TAG,
    UNDEFINED
} uwb_mode_t;


typedef struct _anchor_info_t {
    uint8_t id;
    float x, y;
    float dists[DISTANCE_FILTER_LENGTH];
    float dist;
    uint8_t dist_head;
    uint64_t timestamp_ms; /* Systick timestamp, not DWM. 0 means not received. */
} anchor_info_t;

typedef struct _tag_info_t{
    float d1;
    float d2;
    float x;
    float y;
}tag_info_t;

uwb_mode_t JudgeModeFromID(uint8_t module_id);

extern uint8_t ctrl_msgs[8];
extern uint8_t ctrl_msg_type;
extern uint8_t ctrl_id;

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

#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

//#define POLL_RX_TO_RESP_TX_DLY_UUS 8000
#define RESP_RX_TO_FINAL_TX_DLY_UUS 3500
#define RX_TIMEOUT_UUS 3000
#define RANDOM_TIMEOUT_DELAY random_generator(4500, 8000) // random delay
// #define RANDOM_TIMEOUT_DELAY  1000000 // random delay

#define SPEED_OF_LIGHT 299702547

// define some states for the anchors and tags.
#define TAG_FREE 0
#define TAG_RESPONSE 1
#define TAG_FINAL 2
#define ANCHOR_FREE 0
#define ANCHOR_POLL 1
//#define ANCHOR_EXCHANGING 2
#define ANCHOR_FINAL 3
static uint8_t const MAX_TLE_TIMES = 3;

static tag_info_t tag_storage;
static uint8_t current_state = 0; // 当前状态
static uint8_t current_connect_id = 0; // no use.
static uint8_t tle_times = 0; // 超时次数
static Node n1, n2, n3;
static Node* current_node;

static uint32_t tag_resp_tx = 0;
static uint32_t tag_poll_rx = 0;
static uint32_t anchor_poll_tx = 0;
static uint32_t anchor_resp_rx = 0;

void put_distance(tag_info_t* self, uint8_t anchor_id, float d);

uint32_t random_generator(uint32_t low, uint32_t high);

uint8_t Initialize();

void EventHandler();

#ifdef __cplusplus
}
#endif

#endif //UWB_MODULE_DWT_H
