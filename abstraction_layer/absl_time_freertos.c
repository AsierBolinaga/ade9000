/*
 * absl_time.c
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_time.h"

#ifdef ABSL_TIME
#ifdef ABSL_OS_FREE_RTOS

//#ifdef ABSL_ENET_EVENT
#include "ethernetif.h"
//#endif

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL
#define MILISECONDS_IN_SECOND	1000ULL

void absl_time_init_freertos(void)
{

}

absl_time_t absl_time_gettime_freertos(void)
{
	absl_time_t time;
//#ifdef ABSL_ENET_EVENT
	enet_ptp_time_t ptp_time;

	enet_1588_gettime(&ptp_time);

	time.seconds = ptp_time.second;
	time.nseconds = ptp_time.nanosecond;
//#else
//	// XXX - TODO: REVIEW THIS
//	time.seconds = 0;
//	time.nseconds = 0;
//#endif
	return time;
}

void absl_time_settime_freertos(uint64_t _timestamp)
{
#ifdef ABSL_ENET_EVENT
	enet_ptp_time_t time;

	time.second = _timestamp / 1000000000;
	time.nanosecond  = _timestamp % 1000000000;

	enet_1588_settime(&time);
#endif
}


uint64_t absl_time_to_ms_freertos(absl_time_t time)
{
	return (time.seconds * MILISECONDS_IN_SECOND) + (time.nseconds / 1000000);
}

uint64_t absl_time_to_us_freertos(absl_time_t time)
{
	return (time.seconds * MICROSECONDS_IN_SECOND) + (time.nseconds / 1000);
}

uint64_t absl_time_to_ns_freertos(absl_time_t time)
{
	return (time.seconds * NANOSECONDS_IN_SECOND) + time.nseconds;
}

absl_time_t absl_time_ms_to_time_freertos(uint64_t _time_ms)
{
	absl_time_t time;

	time.seconds = _time_ms / MILISECONDS_IN_SECOND;
	time.nseconds = (_time_ms % MILISECONDS_IN_SECOND) * 1000000;

	return time;
}

absl_time_t absl_time_us_to_time_freertos(uint64_t _time_us)
{
	absl_time_t time;

	time.seconds = _time_us / MICROSECONDS_IN_SECOND;
	time.nseconds = (_time_us % MICROSECONDS_IN_SECOND) * 1000;

	return time;
}

absl_time_t absl_time_ns_to_time_freertos(uint64_t _time_ns)
{
	absl_time_t time;

	time.seconds = _time_ns / NANOSECONDS_IN_SECOND;
	time.nseconds = _time_ns % NANOSECONDS_IN_SECOND;

	return time;
}

absl_time_t absl_time_diff_freertos(absl_time_t _time_a, absl_time_t _time_b)
{
	uint64_t diff_ns = absl_time_diff_ns_freertos(_time_a, _time_b);

	return absl_time_ns_to_time_freertos(diff_ns);
}

uint64_t absl_time_diff_ns_freertos(absl_time_t _time_a, absl_time_t _time_b)
{
	uint64_t time_a_ns = absl_time_to_ns_freertos(_time_a);
	uint64_t time_b_ns = absl_time_to_ns_freertos(_time_b);

	return (time_a_ns - time_b_ns);
}

uint64_t absl_time_diff_us_freertos(absl_time_t _time_a, absl_time_t _time_b)
{
	return absl_time_diff_ns_freertos(_time_a, _time_b) / 1000ULL;
}

uint64_t absl_time_diff_ms_freertos(absl_time_t _time_a, absl_time_t _time_b)
{
	return absl_time_diff_ns_freertos(_time_a, _time_b) / 1000000ULL;
}

#endif /* ABSL_TIME */
#endif
