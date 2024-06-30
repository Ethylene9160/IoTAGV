#include "msgs.h"

#include <utility>

msgs::uwb_data::uwb_data(uint16_t self_id, uint16_t target_id, uint8_t CRC8):
                        header(0x5A), self_id(self_id),target_id(target_id), CRC8(CRC8), ender(0x7F){
    this->data = std::vector<uint8_t>(8);
    this->CRC16 = 0x0F;
}

msgs::serials msgs::uwb_data::serialize() {
    uint16_t data_len = this->data.size();
    auto data = std::shared_ptr<uint8_t[]>(new uint8_t[10 + data_len]);
    uint32_t current_size = 0;

    auto pos = data.get();
    current_size = (sizeof(uint16_t) + sizeof(uint8_t)) << 1;
    memcpy(pos, &(this->header), current_size);
    pos += current_size;

    current_size = sizeof(uint16_t);
    memcpy(pos, &data_len, current_size);
    pos += current_size;

    current_size = data_len * sizeof(uint8_t);
    memcpy(pos, this->data.data(), current_size);
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
    std::memcpy(&data[0], &x, sizeof(float));
    std::memcpy(&data[4], &y, sizeof(float));
}