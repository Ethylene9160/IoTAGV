#ifndef CENTER_MODULE_UWB_TOOL_H_
#define CENTER_MODULE_UWB_TOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float r;
    float t;
} polar_point;

typedef struct {
    float x;
    float y;
} cart_point;

/**
 * Apply the cosine theorem to calculate the cartation coordinate of the point.
 * @param d1 distance to the anthor 0
 * @param d2 distance to the anthor 1
 * @param d distance between the anthor 1 and anthor 2
 * @return the cartation coordinate of the point.
 */
cart_point dis2cart(float d1, float d2, float d);

#ifdef __cplusplus
}
#endif
#endif //TOOL_H
