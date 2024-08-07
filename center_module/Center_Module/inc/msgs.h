#ifndef CENTER_MODULE_MSGS_H_
#define CENTER_MODULE_MSGS_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>
#include "usart.h"

#include "crc.h"
#include "vehicle_controller.h"

#define NONE_CTRL 0
#define POSITION_CTRL 1
#define VELOCITY_CTRL 2
#define STOP_CTRL 3
#define START_CTRL 4

namespace msgs {
    typedef struct _serials {
        uint16_t len;
        std::shared_ptr<uint8_t[]> data_ptr;
    } serials;

    class Serializable {
    public:
        virtual serials serialize() = 0;

        virtual ~Serializable() = default;
    };

    template<typename T>
    class Value : public Serializable {
    public:
        explicit Value(T value) : value(value) {
        }

        serials serialize() override {
            auto data = std::shared_ptr<uint8_t[]>(new uint8_t[sizeof(T)]);
            memcpy(data.get(), &value, sizeof(T));
            return serials{
                .len = static_cast<uint16_t>(sizeof(T)),
                .data_ptr = data
            };
        }

    public:
        T value;
    };

    class Twist2D : public Serializable {
    public:
        Twist2D() : linear_x(0.0f), linear_y(0.0f), angular_z(0.0f) {
        }

        Twist2D(float linear_x, float linear_y, float angular_z) : linear_x(linear_x), linear_y(linear_y),
                                                                   angular_z(angular_z) {
            if (linear_x > 75.0f) {
                this->linear_x = 75.0f;
            }
            if (linear_y > 75.0f) {
                this->linear_y = 75.0f;
            }
            if (angular_z > 75.0f) {
                this->angular_z = 75.0f;
            }

            if (linear_x < -75.0f) {
                this->linear_x = -75.0f;
            }
            if (linear_y < -75.0f) {
                this->linear_y = -75.0f;
            }
            if (angular_z < -75.0f) {
                this->angular_z = -75.0f;
            }
        }

        serials serialize() override {
            auto data = std::shared_ptr<uint8_t[]>(new uint8_t[12]);
            memcpy(data.get(), &linear_x, sizeof(float));
            memcpy(data.get() + 4, &linear_y, sizeof(float));
            memcpy(data.get() + 8, &angular_z, sizeof(float));
            return serials{
                .len = 12,
                .data_ptr = data
            };
        }

    public:
        float linear_x, linear_y, angular_z;
    };

    /**
     * Command will not delete the inner pointer Serializable* data_ref_ !!!
     */
    class Command : public Serializable {
    public:
        Command(uint16_t cmd_id, Serializable* data) : cmd_id_(cmd_id), data_ptr_(data) {}

        serials serialize() override {
            serials data_serialized = data_ptr_->serialize();
            auto data = std::shared_ptr<uint8_t[]>(new uint8_t[data_serialized.len + 2]);
            data[0] = cmd_id_ & 0xFF;
            data[1] = (cmd_id_ >> 8) & 0xFF;
            memcpy(data.get() + 2, data_serialized.data_ptr.get(), data_serialized.len);
            return serials{
                .len = static_cast<uint16_t>(data_serialized.len + 2),
                .data_ptr = data
            };
        }

    public:
        uint16_t cmd_id_;
        Serializable* data_ptr_;
    };

    /**
     * Packet will not delete the inner pointer Serializable* data_ref_ !!!
     */
    class Packet : public Serializable {
    public:
        Packet(uint8_t recv_type, uint8_t recv_id, uint8_t send_type, uint8_t send_id, uint16_t seq, uint16_t cmd_id,
               Serializable* data) : recv_type_(recv_type), recv_id_(recv_id), send_type_(send_type),
                                           send_id_(send_id), seq_(seq),
                                           cmd_id_(cmd_id), data_ref_(data) {
        }

        Packet(uint8_t recv_type, uint8_t recv_id, uint8_t send_type, uint8_t send_id, uint16_t seq, Command cmd)
            : recv_type_(recv_type), recv_id_(recv_id), send_type_(send_type), send_id_(send_id), seq_(seq),
              cmd_id_(cmd.cmd_id_), data_ref_(cmd.data_ptr_) {
        }

        serials
        makePacketBySerials(uint8_t recv_type, uint8_t recv_id, uint8_t send_type, uint8_t send_id, uint16_t seq,
                            uint16_t cmd_id, const serials &data_serialized) {
            auto data = std::shared_ptr<uint8_t[]>(new uint8_t[data_serialized.len + 14]);

            data[0] = 0x5A;
            data[1] = recv_type;
            data[2] = recv_id;
            data[3] = send_type;
            data[4] = send_id;
            data[5] = data_serialized.len & 0xFF;
            data[6] = (data_serialized.len >> 8) & 0xFF;
            data[7] = seq & 0xFF;
            data[8] = (seq >> 8) & 0xFF;
            data[9] = cmd_id & 0xFF;
            data[10] = (cmd_id >> 8) & 0xFF;

            uint8_t crc8 = Get_CRC8_Check_Sum(data.get(), 11, CRC8_INIT);
            data[11] = crc8;

            memcpy(data.get() + 12, data_serialized.data_ptr.get(), data_serialized.len);

            uint16_t crc16 = Get_CRC16_Check_Sum(data.get(), data_serialized.len + 12, CRC16_INIT);
            data[data_serialized.len + 12] = crc16 & 0xFF;
            data[data_serialized.len + 13] = (crc16 >> 8) & 0xFF;

            return serials{
                .len = static_cast<uint16_t>(data_serialized.len + 14),
                .data_ptr = data
            };
        }

        serials serialize() override {
            auto ser = data_ref_->serialize();
            return makePacketBySerials(recv_type_, recv_id_, send_type_, send_id_, seq_, cmd_id_,
                                       ser);
        }

    public:
        uint8_t recv_type_, recv_id_, send_type_, send_id_;
        uint16_t seq_, cmd_id_;
        Serializable* data_ref_;
    };

    class uwb_data : public msgs::Serializable {
    public:
        uwb_data(uint16_t self_id, uint16_t target_id, uint64_t timestamp, uint8_t CRC8);

        uwb_data(uint8_t *data, uint16_t size = 27) {
            memcpy(&header, data, 1);
            memcpy(&self_id, data + 1, 2);
            memcpy(&target_id, data + 3, 2);
            memcpy(&CRC8, data + 5, 1);
            // uint16_t len = (uint16_t) (*(uint16_t *) (data + 6));
            uint16_t len = 8;
            this->data = std::vector<uint8_t>(len);
            memcpy(this->data.data(), data + 8, len);
            memcpy(&this->timestamp, data + 8 + len, sizeof(uint64_t));
            memcpy(&CRC16, data + 16 + len, 2);
            memcpy(&ender, data + len + 18, 1);
        }

        msgs::serials serialize() override;

        void set_data(std::vector<uint8_t> data);

        void set_data(const float x, const float y);

        cart_point get_cart_point() {
            float x = (float) (*(float *) (this->data.data()));
            float y = (float) (*(float *) (this->data.data() + 4));
            return cart_point{x, y};
        }

        uint16_t get_self_id() {
            return this->self_id;
        }

        uint16_t get_target_id() {
            return this->target_id;
        }

        std::vector<uint8_t> get_data() {
            return this->data;
        }

    public:
        uint8_t header;
        uint16_t self_id;
        uint16_t target_id;
        uint8_t CRC8;
        std::vector<uint8_t> data;
        uint64_t timestamp;
        uint16_t CRC16;
        uint8_t ender;
    };


    // inline void _send_tx_msg(UART_HandleTypeDef *huart,uint8_t type, uint8_t id, float f1, float f2) {
    //     host_buffer[8] = type;
    //     host_buffer[9] = id;
    //     memcpy(host_buffer+10, &f1, 4);
    //     memcpy(host_buffer+14, &f2, 4);
    //     HAL_UART_Transmit(huart, host_buffer, 19, HAL_MAX_DELAY);
    // }

    inline void send_msg_to_host(uint8_t type, uint8_t id, float f1, float f2) {
        uint8_t host_buffer[19] = {0x5A, 0x5A,0x5A,0x5A,0x5A,0xFF, 0x0A,0,0,0,0,0,0,0,0,0,0,0,0x7F};
        host_buffer[8] = type;
        host_buffer[9] = id;
        memcpy(host_buffer+10, &f1, 4);
        memcpy(host_buffer+14, &f2, 4);
        HAL_UART_Transmit(&huart2, host_buffer, 19, HAL_MAX_DELAY);
    }

    inline void send_msg_to_uwb(uint8_t type, uint8_t id, float f1, float f2) {
        static uint8_t uwb_buffer[12] = {0x5A, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x7F};
        uwb_buffer[1] = type;
        uwb_buffer[2] = id;
        memcpy(uwb_buffer+3, &f1, 4);
        memcpy(uwb_buffer+7, &f2, 4);
        char str[32];
        snprintf(str, sizeof(str), "send uwb: %d, %d, %f, %f\r\n", type, id, f1, f2);
        HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, uwb_buffer, 12, HAL_MAX_DELAY);
    }
}

#endif // CENTER_MODULE_MSGS_H_
