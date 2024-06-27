#include "ostask_controller_module_port.h"

#include "string.h"

#include "can.h"


bool sendCANMessage(uint8_t *data, uint16_t len, uint32_t id, uint16_t timeout = 10) {
    uint8_t num_full_frames = len / 8;
    uint8_t remaining_bytes = len % 8;
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t TxMailbox;

    tx_header.StdId = id;
    tx_header.ExtId = 0;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.TransmitGlobalTime = DISABLE;

    uint16_t index = 0;

    while (num_full_frames > 0) {
        tx_header.DLC = 8;
        for (uint8_t i = 0; i < 8; i++) {
            tx_data[i] = data[index++];
        }
//        memcpy(tx_data, data + index, tx_header.DLC);
//        index += 8;

        if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &TxMailbox) != HAL_OK) {
            return false;
        }

        uint32_t start_tick = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(&hcan1, TxMailbox)) {
            if ((HAL_GetTick() - start_tick) > pdMS_TO_TICKS(timeout)) {
                HAL_CAN_AbortTxRequest(&hcan1, TxMailbox);
                return false;
            }
        }

        num_full_frames --;
    }

    if (remaining_bytes > 0) {
        tx_header.DLC = remaining_bytes;
        for (uint8_t i = 0; i < remaining_bytes; i++) {
            tx_data[i] = data[index++];
        }
//        memcpy(tx_data, data + index, remaining_bytes);

        if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &TxMailbox) != HAL_OK) {
            return false;
        }

        uint32_t start_tick = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(&hcan1, TxMailbox)) {
            if ((HAL_GetTick() - start_tick) > pdMS_TO_TICKS(timeout)) {
                HAL_CAN_AbortTxRequest(&hcan1, TxMailbox);
                return false;
            }
        }
    }

    return true;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (hcan == (CAN_HandleTypeDef *) &hcan1) {
        CAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8];
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data);
        rx_data[7] = 0;
    }
}

void controllerModulePortTask(void *argument) {
    CAN_FilterTypeDef filter;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterIdHigh = 0;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow = 0;
    filter.FilterFIFOAssignment = 0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14;
    HAL_CAN_ConfigFilter(&hcan1,&filter);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&hcan1);

    uint16_t id = 0x061A; // 0x0600 ~ 0x0637

    uint8_t expire_1_data[18] = {0x5A, 0xF0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x6E, 0xE8, 0x03, 0x00, 0x00, 0xB7, 0xB9};
    uint8_t expire_2_data[18] = {0x5A, 0xF0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x6E, 0xD0, 0x07, 0x00, 0x00, 0xFC, 0x73};
    uint8_t vx_p10_data[26] = {0x5A, 0xF0, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xBD};
    uint8_t vx_n10_data[26] = {0x5A, 0xF0, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B, 0x00, 0x00, 0x20, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x79};

    sendCANMessage(expire_2_data, sizeof(expire_2_data), id);
    osDelay(1000);

    while (1) {
        sendCANMessage(vx_p10_data, sizeof(vx_p10_data), id);
        osDelay(1000);
        sendCANMessage(vx_n10_data, sizeof(vx_n10_data), id);
        osDelay(1000);
    }
}
