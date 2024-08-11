#include "msgs.h"

#include <utility>


msgs::uwb_data::uwb_data(uint16_t self_id, uint16_t target_id, uint64_t timestamp, uint8_t CRC8):
                        header(0x5A), self_id(self_id),target_id(target_id), timestamp(timestamp), CRC8(CRC8), ender(0x7F){
    this->data = std::vector<uint8_t>(8);
    this->CRC16 = 0x0F;
}

msgs::serials msgs::uwb_data::serialize() {
    uint16_t data_len = this->data.size();
    auto data = std::shared_ptr<uint8_t[]>(new uint8_t[11 + data_len]);
    uint32_t current_size = 0;

    auto pos = data.get();
    current_size = (sizeof(uint16_t) + sizeof(uint8_t)) << 1;

    memcpy(pos, &(this->header), sizeof(uint8_t));
    memcpy(pos+1, &(this->self_id), sizeof(uint16_t));
    memcpy(pos+3, &(this->target_id), sizeof(uint16_t));
    memcpy(pos+5, &(this->CRC8), sizeof(uint8_t));

    pos += current_size;

    current_size = sizeof(uint16_t);
    memcpy(pos, &data_len, current_size);
    pos += current_size;

    current_size = data_len * sizeof(uint8_t);
    memcpy(pos, this->data.data(), current_size);
    pos += current_size;

    current_size = sizeof(uint64_t);
    memcpy(pos, &(this->timestamp), current_size);
    pos += current_size;

    current_size = sizeof(uint16_t);
    memcpy(pos, &(this->CRC16), current_size);
    pos += current_size;

    current_size = sizeof(uint8_t);
    memcpy(pos, &(this->ender), current_size);
    pos += current_size;

    return msgs::serials{
        .len = static_cast<uint16_t>(data_len + 11),
        .data_ptr = data
    };
}

void msgs::uwb_data::set_data(std::vector<uint8_t> data) {
    this->data = std::move(data);
}

void msgs::uwb_data::set_data(const float x, const float y) {
    data = std::vector<uint8_t>(8);
    memcpy(&data[0], &x, sizeof(float));
    memcpy(&data[4], &y, sizeof(float));
}

void msgs::send_msg_to_host(uint8_t type, uint8_t id, float f1, float f2) {
    uint8_t host_buffer[19] = {0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0xFF, 0x0A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F};
    // 目前指令长度均为 0x0A, 0x00 (小端, 即 10)
    host_buffer[8] = type;
    host_buffer[9] = id;
    memcpy(host_buffer + 10, &f1, 4);
    memcpy(host_buffer + 14, &f2, 4);
    HAL_UART_Transmit(&huart2, host_buffer, 19, HAL_MAX_DELAY);
}

void msgs::send_msg_to_uwb(uint8_t type, uint8_t id, float f1, float f2) {
    static uint8_t uwb_buffer[12] = {0x5A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F};
    uwb_buffer[1] = type;
    uwb_buffer[2] = id;
    memcpy(uwb_buffer + 3, &f1, 4);
    memcpy(uwb_buffer + 7, &f2, 4);
    HAL_UART_Transmit(&huart1, uwb_buffer, 12, HAL_MAX_DELAY);

    // char str[64];
    // snprintf(str, sizeof(str), "send uwb: %d, %d, [%d%d%d%d], [%d%d%d%d], %.2f, %.2f", type, id,
    //     uwb_buffer[3], uwb_buffer[4], uwb_buffer[5], uwb_buffer[6], uwb_buffer[7], uwb_buffer[8], uwb_buffer[9], uwb_buffer[10],
    //     *(float*)(uwb_buffer+3), *(float*)(uwb_buffer+7));
    // HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
    // HAL_UART_Transmit(&huart2, uwb_buffer, 12, HAL_MAX_DELAY);
    // HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
    // for (int i = 0; i < 12; ++i) {
    // snprintf(str, sizeof(str), "%d, ", uwb_buffer[i]);
    // 通过usart1 一个字节一个字节的发送。
    //     HAL_UART_Transmit(&huart1, uwb_buffer + i, 1, HAL_MAX_DELAY);
    // }
}
