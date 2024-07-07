#include "tool.h"

#include <math.h>

Point2d dis2cart(float d1, float d2, float d){
    float cos_angle = (d1 * d1 + d * d - d2 * d2) / (2 * d1 * d);
    float angle_rad = (float) acosf(cos_angle);

    float x = d1 * cosf(angle_rad) - 0.5f * d;
    float y = d1 * sinf(angle_rad);
    Point2d p = {x, y};
    return p;
}

int two_point_localization(Point2d c1, float r1, Point2d c2, float r2, Point2d *p) {
    float dx = c2.x - c1.x;
    float dy = c2.y - c1.y;
    float d = sqrt(dx * dx + dy * dy);

    if (d >= r1 + r2 || d <= fabs(r1 - r2)) {
        return 0;
    }

    float a = (r1 * r1 - r2 * r2 + d * d) / (2 * d);
    float h = sqrt(r1 * r1 - a * a);
    float x2 = c1.x + a * (c2.x - c1.x) / d;
    float y2 = c1.y + a * (c2.y - c1.y) / d;
    float x3 = dy * h / d;
    float y3 = -dx * h / d;

    float p1x, p1y, p2x, p2y;

    if (fabs(dx) > fabs(dy)) {
        p1x = x2 + x3;
        p1y = y2 + y3;
        p2x = x2 - x3;
        p2y = y2 - y3;
    } else {
        p1x = x2 + y3;
        p1y = y2 + x3;
        p2x = x2 - y3;
        p2y = y2 - x3;
    }

    if (p1x >= 0 && p1y >= 0) {
        p->x = p1x;
        p->y = p1y;
        return 2;
    } else if (p2x >= 0 && p2y >= 0) {
        p->x = p2x;
        p->y = p2y;
        return 2;
    } else {
        return 0;
    }
}
