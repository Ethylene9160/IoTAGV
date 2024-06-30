#include "port_uart.h"

#include <cstring>

#include "cmsis_os.h"

bool PortUSART::init(uint16_t module_id, USART_TypeDef &usart) {
    this->_module_id = module_id;
    this->_usart = usart;

    bool state = true;

    return state;
}

bool PortUSART::sendBytes(uint8_t data[], uint16_t len, uint16_t timeout) {
// todo.
    uint8_t num_full_frames = len / 8;
    uint8_t remaining_bytes = len % 8;

    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t tx_mailbox;

    tx_header.StdId = PortUSART::module_id;
    tx_header.ExtId = 0;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.TransmitGlobalTime = DISABLE;

    uint16_t index = 0;

    while (num_full_frames > 0) {
        tx_header.DLC = 8;
        memcpy(tx_data, data + index, tx_header.DLC);
        index += tx_header.DLC;

        if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
            return false;
        }

        uint32_t start_tick = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(&hcan1, tx_mailbox)) {
            if ((HAL_GetTick() - start_tick) > pdMS_TO_TICKS(timeout)) {
                HAL_CAN_AbortTxRequest(&hcan1, tx_mailbox);
                return false;
            }
        }

        num_full_frames --;
    }

    if (remaining_bytes > 0) {
        tx_header.DLC = remaining_bytes;
        memcpy(tx_data, data + index, remaining_bytes);

        if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) != HAL_OK) {
            return false;
        }

        uint32_t start_tick = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(&hcan1, tx_mailbox)) {
            if ((HAL_GetTick() - start_tick) > pdMS_TO_TICKS(timeout)) {
                HAL_CAN_AbortTxRequest(&hcan1, tx_mailbox);
                return false;
            }
        }
    }

    return true;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (hcan->Instance == PortCAN::getHcan().Instance) {
        CAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8];
        HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, rx_data); // 必须收, 否则似乎会阻塞住系统.
    }
}
