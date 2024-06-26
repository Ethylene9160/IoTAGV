#ifndef IOT_CAR_RAND_DIST
#define IOT_CAR_RAND_DIST 1

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
static uint8_t haveSpare = 0; //deprecated
static float rand1, rand2;

/*
  ��������ɡ�
*/

typedef struct {
    float mean = 1.0f;
    float stddev = 1.0f;
    uint8_t flag = 0;
}gaussian_dist;

typedef struct {
    float min = -1.0f;
    float max = -1.0f;
}uniform_dist;

float next_gaussian(gaussian_dist* gd) {
    if (gd->flag) {
        gd->flag = 0;
        return gd->mean + gd->stddev * sqrt(rand1) * sin(rand2);
    }

    gd->flag = 1;

    rand1 = rand() / ((float)RAND_MAX);
    if (rand1 < 1e-100) rand1 = 1e-100; // ��ֹ log(0)
    rand1 = -2 * log(rand1);
    rand2 = (rand() / ((float)RAND_MAX)) * 2 * M_PI;

    return mean + stddev * sqrt(rand1) * cos(rand2);
}

float next_uniform(uniform_dist* ud) {
    // ���� [0, 1) ֮��������
    float rand_val = rand() / ((float)RAND_MAX + 1.0f);
    return ud->min + rand_val * (ud->max - ud->min);
}

// ���ɸ�˹�ֲ���������� ���á�
float generate_gaussian(float mean, float stddev) {
    if (haveSpare) {
        haveSpare = false;
        return mean + stddev * sqrt(rand1) * sin(rand2);
    }

    haveSpare = 1;

    rand1 = rand() / ((float)RAND_MAX);
    if (rand1 < 1e-100) rand1 = 1e-100; // ��ֹ log(0)
    rand1 = -2 * log(rand1);
    rand2 = (rand() / ((float)RAND_MAX)) * 2 * M_PI;

    return mean + stddev * sqrt(rand1) * cos(rand2);
}

// ���ɾ��ȷֲ�����������á�
float generate_uniform(float min, float max) {
    // ���� [0, 1) ֮��������
    float rand_val = rand() / ((float)RAND_MAX + 1.0f);
    return min + rand_val * (max - min);
}



#endif