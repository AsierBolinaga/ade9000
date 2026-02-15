/*
 * pl_time.c
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_time.h"

#ifdef PL_TIME
#ifdef PL_LINUX
#include <time.h>

void pl_time_init_posix(void)
{

}

void pl_time_gettime_posix(pl_time_t* time)
{
	struct timespec timestamp;
	clock_gettime(CLOCK_MONOTONIC , &timestamp);

	time->seconds = timestamp.tv_sec;
	time->nseconds = timestamp.tv_nsec;
}

#endif /* PL_TIME */
#endif
