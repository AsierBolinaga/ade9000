/*
 * pl_time.c
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_time.h"

#ifdef PL_TIME
#ifdef PL_OS_FREE_RTOS

#ifdef PL_ENET_EVENT
#include "ethernetif.h"
#endif

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL
#define MILISECONDS_IN_SECOND	1000ULL

void pl_time_init_freertos(void)
{

}

pl_time_t pl_time_gettime_freertos(void)
{
	pl_time_t time;
#ifdef PL_ENET_EVENT
	enet_ptp_time_t ptp_time;

	enet_1588_gettime(&ptp_time);

	time.seconds = ptp_time.second;
	time.nseconds = ptp_time.nanosecond;
#else
	// XXX - TODO: REVIEW THIS
	time.seconds = 0;
	time.nseconds = 0;
#endif
	return time;
}

void pl_time_settime_freertos(uint64_t _timestamp)
{
#ifdef PL_ENET_EVENT
	enet_ptp_time_t time;

	time.second = _timestamp / 1000000000;
	time.nanosecond  = _timestamp % 1000000000;

	enet_1588_settime(&time);
#endif
}


uint64_t pl_time_to_ms_freertos(pl_time_t time)
{
	return (time.seconds * MILISECONDS_IN_SECOND) + (time.nseconds / 1000000);
}

uint64_t pl_time_to_us_freertos(pl_time_t time)
{
	return (time.seconds * MICROSECONDS_IN_SECOND) + (time.nseconds / 1000);
}

uint64_t pl_time_to_ns_freertos(pl_time_t time)
{
	return (time.seconds * NANOSECONDS_IN_SECOND) + time.nseconds;
}

pl_time_t pl_time_ms_to_time_freertos(uint64_t _time_ms)
{
	pl_time_t time;

	time.seconds = _time_ms / MILISECONDS_IN_SECOND;
	time.nseconds = (_time_ms % MILISECONDS_IN_SECOND) * 1000000;

	return time;
}

pl_time_t pl_time_us_to_time_freertos(uint64_t _time_us)
{
	pl_time_t time;

	time.seconds = _time_us / MICROSECONDS_IN_SECOND;
	time.nseconds = (_time_us % MICROSECONDS_IN_SECOND) * 1000;

	return time;
}

pl_time_t pl_time_ns_to_time_freertos(uint64_t _time_ns)
{
	pl_time_t time;

	time.seconds = _time_ns / NANOSECONDS_IN_SECOND;
	time.nseconds = _time_ns % NANOSECONDS_IN_SECOND;

	return time;
}

pl_time_t pl_time_diff_freertos(pl_time_t _time_a, pl_time_t _time_b)
{
	uint64_t diff_ns = pl_time_diff_ns_freertos(_time_a, _time_b);

	return pl_time_ns_to_time_freertos(diff_ns);
}

uint64_t pl_time_diff_ns_freertos(pl_time_t _time_a, pl_time_t _time_b)
{
	uint64_t time_a_ns = pl_time_to_ns_freertos(_time_a);
	uint64_t time_b_ns = pl_time_to_ns_freertos(_time_b);

	return (time_a_ns - time_b_ns);
}

uint64_t pl_time_diff_us_freertos(pl_time_t _time_a, pl_time_t _time_b)
{
	return pl_time_diff_ns_freertos(_time_a, _time_b) / 1000ULL;
}

uint64_t pl_time_diff_ms_freertos(pl_time_t _time_a, pl_time_t _time_b)
{
	return pl_time_diff_ns_freertos(_time_a, _time_b) / 1000000ULL;
}

#endif /* PL_TIME */
#endif
