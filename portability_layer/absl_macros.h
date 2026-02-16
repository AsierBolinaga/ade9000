/*
 * ABSL_macros.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_MACROS_H_
#define ABSL_MACROS_H_

#include "absl_config.h"

#define ABSL_CONTAINER_OF(ptr, type, member) ({ \
        (void *)ptr - offsetof(type, member);})

#define BITS_PER_LONG     32

#define BIT(nr)			(1UL << (nr))
#define GENMASK(h, l) \
	    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#if defined(ABSL_IMX_RT10XX)
#define ABSL_ALLOC_DATA_SEC  	__attribute__((section(".data_section"))) ABSL_ALIGNED(16)

#define ABSL_ALLOC_IRQ_FCN 	__attribute__((section(".ramfunc.$SRAM_ITC"))) __attribute__((noinline))
#define ABSL_FAST_DATA		__attribute__((section(".noinit.$SRAM_ITC")))
#define ABSL_ALIGNED(x) 		__attribute__((aligned(x)))

#define ABSL_ALLOC_NCACHE_DATA   __attribute__((section(".data.$NCACHE_REGION"))) __attribute__((aligned(16)))
#else
#endif

#if defined(ABSL_IMX_RT10XX)
#define ABSL_SET_FLOAT(dst, val)  (*(float*)(dst) = (val))
#define ABSL_SET_DOUBLE(dst, val) (*(double*)(dst) = (val))
#define ABSL_SET_UINT32(dst, val) (*(uint32_t*)(dst) = (val))
#else
#define ABSL_SET_FLOAT(dst, val)  memcpy((dst), &(val), sizeof(float))
#define ABSL_SET_DOUBLE(dst, val) memcpy((dst), &(val), sizeof(double))
#define ABSL_SET_UINT32(dst, val) memcpy((dst), &(val), sizeof(uint32_t))
#endif

#define ABSL_UNUSED_ARG(x) (void)x

#define ABSL_DEC_INDEX(a, b) (((a - 1) < 0) ? (b - 1) : (a - 1))
#define ABSL_INC_INDEX(a, b) (((a + 1) == b) ? 0 : (a + 1))

#endif /* ABSL_MACROS_H_ */
