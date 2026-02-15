/*
 * pl_time.h
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_TIME_H_
#define PL_TIME_H_

#include "pl_config.h"
#ifdef PL_TIME

#include "pl_types.h"

#if  defined(PL_PTPD_TIME)
#include "ptpd.h"
#endif

typedef struct pl_time
{
	uint64_t	seconds;
	uint32_t	nseconds;
}pl_time_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_time_init		pl_time_init_freertos
#define pl_time_gettime		pl_time_gettime_freertos
#define pl_time_settime		pl_time_settime_freertos
#define pl_time_to_ms		pl_time_to_ms_freertos
#define pl_time_to_us		pl_time_to_us_freertos
#define pl_time_to_ns		pl_time_to_ns_freertos
#define pl_time_ms_to_time	pl_time_ms_to_time_freertos
#define pl_time_us_to_time	pl_time_us_to_time_freertos
#define pl_time_ns_to_time	pl_time_ns_to_time_freertos
#define pl_time_diff		pl_time_diff_freertos
#define pl_time_diff_ns		pl_time_diff_ns_freertos
#define pl_time_diff_us		pl_time_diff_us_freertos
#define pl_time_diff_ms		pl_time_diff_ms_freertos
#elif defined (PL_LINUX)
#define pl_time_init		pl_time_init_posix
#define pl_time_gettime		pl_time_gettime_posix
#else
#error Platform not defined
#endif

void pl_time_init(void);

pl_time_t pl_time_gettime(void);

void pl_time_settime(uint64_t _timestamp);

uint64_t pl_time_to_ms(pl_time_t time);

uint64_t pl_time_to_us(pl_time_t time);

uint64_t pl_time_to_ns(pl_time_t time);

pl_time_t pl_time_us_to_time(uint64_t _time_us);

pl_time_t pl_time_ns_to_time(uint64_t _time_ns);

pl_time_t pl_time_ms_to_time(uint64_t _time_ms);

pl_time_t pl_time_diff(pl_time_t _time_a, pl_time_t _time_b);

uint64_t pl_time_diff_ns(pl_time_t _time_a, pl_time_t _time_b);

uint64_t pl_time_diff_us(pl_time_t _time_a, pl_time_t _time_b);

uint64_t pl_time_diff_ms(pl_time_t _time_a, pl_time_t _time_b);

#endif /* PL_TIME */
#endif /* PL_TIME_H_ */
