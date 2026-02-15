/*
 * pl_macros.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef PL_MACROS_H_
#define PL_MACROS_H_

#include "pl_config.h"

#define PL_CONTAINER_OF(ptr, type, member) ({ \
        (void *)ptr - offsetof(type, member);})

#define BITS_PER_LONG     32

#define BIT(nr)			(1UL << (nr))
#define GENMASK(h, l) \
	    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#if defined(PL_IMX_RT10XX)
#define ALLOC_DATA_SEC  	__attribute__((section(".data_section")))
#define PL_NON_VOLTILE_VAR	__attribute__((section(".noinit_data")))
#else
#endif

#define PL_UNUSED_ARG(x) (void)x

#define PL_DEC_INDEX(a, b) (((a - 1) < 0) ? (b - 1) : (a - 1))
#define PL_INC_INDEX(a, b) (((a + 1) == b) ? 0 : (a + 1))

#endif /* PL_MACROS_H_ */
