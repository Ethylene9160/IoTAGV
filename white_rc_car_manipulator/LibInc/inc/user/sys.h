#ifndef __SYS_H__
#define __SYS_H__


#include "stm32f4xx_hal.h"

#define VAL_LIMIT(val, min, max)\
if((val) <= (min))\
{\
  (val) = (min);\
}\
else if((val) >= (max))\
{\
  (val) = (max);\
}\



#endif
