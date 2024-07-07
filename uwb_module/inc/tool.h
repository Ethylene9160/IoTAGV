#ifndef UWB_MODULE_VELOCITY_CONTROLLER_H
#define UWB_MODULE_VELOCITY_CONTROLLER_H

typedef struct {
    float x;
    float y;
} cart_point;

/**
* @brief  Calculate the cartesian coordinates of a point based on the distance between the point and two other anchors.
* @param  d1: distance between the point and anchor 1
* @param  d2: distance between the point and anchor 2
* @param  d: distance between anchor 1 and anchor 2
* @retval The cartesian coordinates of the point.
*/
cart_point dis2cart(float d1, float d2, float d);

#endif //UWB_MODULE_VELOCITY_CONTROLLER_H
