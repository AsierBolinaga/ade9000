/*
 * absl_time.h
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_TIME_H_
#define ABSL_TIME_H_

#include "absl_config.h"
#ifdef ABSL_TIME

#include "absl_types.h"

#if  defined(ABSL_PTPD_TIME)
#include "ptpd.h"
#endif

typedef struct absl_time
{
	uint64_t	seconds;
	uint32_t	nseconds;
}absl_time_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_time_init		absl_time_init_freertos
#define absl_time_gettime		absl_time_gettime_freertos
#define absl_time_settime		absl_time_settime_freertos
#define absl_time_to_ms		absl_time_to_ms_freertos
#define absl_time_to_us		absl_time_to_us_freertos
#define absl_time_to_ns		absl_time_to_ns_freertos
#define absl_time_ms_to_time	absl_time_ms_to_time_freertos
#define absl_time_us_to_time	absl_time_us_to_time_freertos
#define absl_time_ns_to_time	absl_time_ns_to_time_freertos
#define absl_time_diff		absl_time_diff_freertos
#define absl_time_diff_ns		absl_time_diff_ns_freertos
#define absl_time_diff_us		absl_time_diff_us_freertos
#define absl_time_diff_ms		absl_time_diff_ms_freertos
#elif defined (ABSL_LINUX)
#define absl_time_init		absl_time_init_posix
#define absl_time_gettime		absl_time_gettime_posix
#else
#error Platform not defined
#endif

void absl_time_init(void);

absl_time_t absl_time_gettime(void);

void absl_time_settime(uint64_t _timestamp);

uint64_t absl_time_to_ms(absl_time_t time);

uint64_t absl_time_to_us(absl_time_t time);

uint64_t absl_time_to_ns(absl_time_t time);

absl_time_t absl_time_us_to_time(uint64_t _time_us);

absl_time_t absl_time_ns_to_time(uint64_t _time_ns);

absl_time_t absl_time_ms_to_time(uint64_t _time_ms);

absl_time_t absl_time_diff(absl_time_t _time_a, absl_time_t _time_b);

uint64_t absl_time_diff_ns(absl_time_t _time_a, absl_time_t _time_b);

uint64_t absl_time_diff_us(absl_time_t _time_a, absl_time_t _time_b);

uint64_t absl_time_diff_ms(absl_time_t _time_a, absl_time_t _time_b);

#endif /* ABSL_TIME */
#endif /* ABSL_TIME_H_ */
