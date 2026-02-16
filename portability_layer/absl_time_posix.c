/*
 * absl_time.c
 *
 *  Created on: 19 may. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_time.h"

#ifdef ABSL_TIME
#ifdef ABSL_LINUX
#include <time.h>

void absl_time_init_posix(void)
{

}

void absl_time_gettime_posix(absl_time_t* time)
{
	struct timespec timestamp;
	clock_gettime(CLOCK_MONOTONIC , &timestamp);

	time->seconds = timestamp.tv_sec;
	time->nseconds = timestamp.tv_nsec;
}

#endif /* ABSL_TIME */
#endif
