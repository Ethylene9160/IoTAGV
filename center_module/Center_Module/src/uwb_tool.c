#include "uwb_tool.h"
// #include "uwb_transceiver.h"
#include "math.h"

cart_point dis2cart(float d1, float d2, float d) {
    float cos_angle = (d1 * d1 + d * d - d2 * d2) / (2 * d1 * d);
    float angle_rad = (float) acosf(cos_angle);

    float x = d1 * cosf(angle_rad) - 0.5f * d;
    float y = d1 * sinf(angle_rad);
    cart_point p = {x, y};
    return p;
}
