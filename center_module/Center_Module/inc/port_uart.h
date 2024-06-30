#ifndef CENTER_MODULE_USART_CAN_H_
#define CENTER_MODULE_USART_CAN_H_

#include <cstdint>
#include <queue>

#include "can.h"
#include "cmsis_os.h"
#include "msgs.h"
#include "usart.h"

class PortUART {
public:
    explicit PortUART(uint16_t module_id = 0, UART_HandleTypeDef &huart = huart1) : _module_id(module_id), _huart(huart) {
        uwb_data_queue_mutex = osMutexNew(nullptr);
    }
    bool init(uint16_t module_id, UART_HandleTypeDef &huart = huart1);
    bool sendBytes(uint8_t data[], uint16_t len, uint16_t timeout = 10);

    UART_HandleTypeDef &getHuart() { return this->_huart; }
    uint16_t getModuleID() { return this->_module_id; }
    void setModuleID(uint16_t id) { this->_module_id = id; }

    bool pushUWBData(const msgs::uwb_data &data){
        if (uwb_data_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(uwb_data_queue_mutex, osWaitForever);
            if (status == osOK) {
                uwb_data_queue.push(data);
                osMutexRelease(uwb_data_queue_mutex);
                return true;
            }
        }
        return false;
    }
    msgs::uwb_data popUWBData() {
        if (uwb_data_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(uwb_data_queue_mutex, osWaitForever);
            if (status == osOK) {
                if (!uwb_data_queue.empty()) {
                    msgs::uwb_data data = uwb_data_queue.front();
                    uwb_data_queue.pop();
                    osMutexRelease(uwb_data_queue_mutex);
                    return data;
                }
            }
        }
        return {0, 0, 0};
    }

    bool isDataQueueEmpty() {
        if (uwb_data_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(uwb_data_queue_mutex, osWaitForever);
            if (status == osOK) {
                bool empty = uwb_data_queue.empty();
                osMutexRelease(uwb_data_queue_mutex);
                return empty;
            }
        }
        return true; // TODO
    }

    bool clearDataQueue() {
        if (uwb_data_queue_mutex != nullptr) {
            osStatus_t status = osMutexAcquire(uwb_data_queue_mutex, osWaitForever);
            if (status == osOK) {
                while (!uwb_data_queue.empty()) {
                    uwb_data_queue.pop();
                }
                osMutexRelease(uwb_data_queue_mutex);
                return true;
            }
        }
        return false;
    }

    void release_mutex() {
        osMutexDelete(uwb_data_queue_mutex);
    }

    void startReceiving() {
        // 使能接收中断
        // todo.
    }

private:
    uint16_t _module_id;
    UART_HandleTypeDef _huart;
    osMutexId_t uwb_data_queue_mutex = nullptr;
    std::queue<msgs::uwb_data> uwb_data_queue;
};

#endif // CENTER_MODULE_USART_CAN_H_