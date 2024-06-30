#ifndef CENTER_MODULE_MSGS_H_
#define CENTER_MODULE_MSGS_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "crc.h"

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
        explicit Value(T value) : value(value) {}

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
        Twist2D() : linear_x(0.0f), linear_y(0.0f), angular_z(0.0f) {}

        Twist2D(float linear_x, float linear_y, float angular_z) : linear_x(linear_x), linear_y(linear_y),
                                                                   angular_z(angular_z) {}

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

    class Command : public Serializable {
    public:
        Command(uint16_t cmd_id, const Serializable &data) : cmd_id_(cmd_id),
                                                             data_ref_(const_cast<Serializable &>(data)) {}

        serials serialize() override {
            serials data_serialized = data_ref_.serialize();
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
        Serializable &data_ref_;
    };

    class Packet : public Serializable {
    public:
        Packet(uint8_t recv_type, uint8_t recv_id, uint8_t send_type, uint8_t send_id, uint16_t seq, uint16_t cmd_id,
               const Serializable &data) :
            recv_type_(recv_type), recv_id_(recv_id), send_type_(send_type), send_id_(send_id), seq_(seq),
            cmd_id_(cmd_id), data_ref_(const_cast<Serializable &>(data)) {}

        Packet(uint8_t recv_type, uint8_t recv_id, uint8_t send_type, uint8_t send_id, uint16_t seq, const Command &cmd)
            :
            recv_type_(recv_type), recv_id_(recv_id), send_type_(send_type), send_id_(send_id), seq_(seq),
            cmd_id_(cmd.cmd_id_), data_ref_(cmd.data_ref_) {}

        static serials
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
            return makePacketBySerials(recv_type_, recv_id_, send_type_, send_id_, seq_, cmd_id_,
                                       data_ref_.serialize());
        }

    public:
        uint8_t recv_type_, recv_id_, send_type_, send_id_;
        uint16_t seq_, cmd_id_;
        Serializable &data_ref_;
    };

    class uwb_data : public msgs::Serializable {
    public:
        uwb_data(uint16_t self_id, uint16_t target_id, uint8_t CRC8);
        msgs::serials serialize() override;
        void set_data(std::vector<uint8_t> data);
        void set_data(const float x, const float y);

    private:
        uint8_t header;
        uint16_t self_id;
        uint16_t target_id;
        uint8_t CRC8;
        std::vector<uint8_t> data;
        uint16_t CRC16;
    };

}

#endif // CENTER_MODULE_MSGS_H_
