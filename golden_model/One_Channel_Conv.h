
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>

#define DSIZE       1024
#define IMAGE_WIDTH  62
#define IMAGE_HEIGHT  12

#define KERNEL_3x3 1
//#define KERNEL_5x5 1

#ifndef  q7_t 
    #define q7_t 	int8_t
#endif

#ifndef  q15_t
    #define q15_t 	int16_t
#endif

#ifndef  q31_t
    #define q31_t 	int32_t
#endif

#ifdef KERNEL_3x3
        #define KERNEL_WIDTH  3
        #define KERNEL_HEIGHT  3
#endif

#ifdef KERNEL_5x5
        #define KERNEL_WIDTH  5
        #define KERNEL_HEIGHT  5
#endif


#define STRIDE_X  1
#define STRIDE_Y  1

#define RESULT_WIDTH  ((IMAGE_WIDTH - KERNEL_WIDTH) / STRIDE_X + 1)
#define RESULT_HEIGHT ((IMAGE_HEIGHT - KERNEL_HEIGHT) / STRIDE_Y + 1)

// SSAT implementation with C code
#ifndef __NNOM_SSAT
static inline int __NNOM_SSAT(int32_t value, int32_t bit) {
    int32_t min = -(1<<(bit-1));
    int32_t max = (1<<(bit-1)) - 1;
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}
#endif

