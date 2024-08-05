#ifndef CENTER_MODULE_FILTER_H
#define CENTER_MODULE_FILTER_H
#include <cstdint>
#include <list>
#include <vector>

#include "linked_list.h"

namespace center_filter {
    class Filter {
    public:
        Filter() {
        }
        virtual ~Filter() {

        }
        virtual void filter(float value) = 0;
        float value() {
            return _val;
        }
    protected:
        float _val;
    };

    class LBF : public Filter {
    public:
        LBF(float alpha, float init_val=0.0f):Filter() {
            this->alpha = alpha;
            this->_val = init_val;
        }

        void filter(float value) override {
            this -> _val = alpha * value + (1.0f - alpha) * _val;
        }

        // float value() override {
        //     return _val;
        // }
    private:
        float alpha;
    };

    class AverageFilter : public Filter {
    public:
        AverageFilter(uint32_t len, float init_val = 0.0f):Filter(){
            this->max_len = len;
            this->index = 0;
            this->buffer = std::vector<float>(len, init_val);
            this->_val = init_val;
        }
        virtual ~AverageFilter() {
        }

        // float value() override {
        //     return _val
        // }

        void filter(float value) override {
            this->_val -= buffer[++index] / max_len;
            this->_val += value / max_len;
            buffer[index] = value;
        }
    private:
        uint32_t index;
        uint32_t max_len;
        float last_val;
        std::vector<float> buffer;
        // std::list<float> buffer;
        // float init_val;
    };

}
#endif //CENTER_MODULE_FILTER_H
