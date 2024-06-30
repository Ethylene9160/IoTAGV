/*! ----------------------------------------------------------------------------
 * @file    deca_sleep.h
 * @brief   platform dependent sleep implementation
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#ifndef DECA_SLEEP_H_
#define DECA_SLEEP_H_

#ifdef __cplusplus
extern "C" {
#endif

void deca_sleep(unsigned int time_ms);

#define sleep_ms deca_sleep

#ifdef __cplusplus
}
#endif

#endif /* DECA_SLEEP_H_ */
