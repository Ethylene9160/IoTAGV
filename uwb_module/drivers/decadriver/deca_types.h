/*! ----------------------------------------------------------------------------
 *  @file 	deca_types.h
 *  @brief 	DecaWave general type definitions
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef _DECA_TYPES_H_
#define _DECA_TYPES_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef uint8
#ifndef _DECA_UINT8_
#define _DECA_UINT8_
//typedef unsigned char uint8;
typedef uint8_t uint8;
#endif
#endif

#ifndef uint16
#ifndef _DECA_UINT16_
#define _DECA_UINT16_
//typedef unsigned short uint16;
typedef uint16_t uint16;
#endif
#endif

#ifndef uint32
#ifndef _DECA_UINT32_
#define _DECA_UINT32_
//typedef unsigned int uint32;
typedef uint32_t uint32;
#endif
#endif

#ifndef int8
#ifndef _DECA_INT8_
#define _DECA_INT8_
//typedef signed char int8;
typedef int8_t int8;
#endif
#endif

#ifndef int16
#ifndef _DECA_INT16_
#define _DECA_INT16_
//typedef signed short int16;
typedef int16_t int16;
#endif
#endif

#ifndef int32
#ifndef _DECA_INT32_
#define _DECA_INT32_
//typedef signed int int32;
typedef int32_t int32;
#endif
#endif

#ifndef NULL
#define NULL ((void *)0UL)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DECA_TYPES_H_ */


