#ifndef UWB_MODULE_TOOL_H
#define UWB_MODULE_TOOL_H

#include "stdint.h"

#define EPS 1e-5

typedef struct {
    float x;
    float y;
} Point2d;

/**
* @brief  Calculate the cartesian coordinates of a point based on the distance between the point and two other anchors.
* @param  d1: distance between the point and anchor 1
* @param  d2: distance between the point and anchor 2
* @param  d: distance between anchor 1 and anchor 2
* @retval The cartesian coordinates of the point.
*/

Point2d dis2cart(float d1, float d2, float d);

/* 保留 x, y 为正的解 */
int two_point_localization(Point2d c1, float r1, Point2d c2, float r2, Point2d *p);

#endif
