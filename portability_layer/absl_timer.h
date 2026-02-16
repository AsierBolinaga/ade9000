/*
 * absl_timer.h
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_TIMER_H_
#define ABSL_TIMER_H_

#include "absl_config.h"
#ifdef ABSL_TIMER

#include "absl_types.h"
#include "absl_time.h"

#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "timers.h"
#elif defined(ABSL_LINUX)
#include <time.h>
#endif

typedef enum absl_timer_rv
{
	ABSL_TIMER_RV_OK = 0,
	ABSL_TIMER_RV_ERROR
}absl_timer_rv_t;

typedef void (*timer_cb_t)(void* arg);

typedef struct absl_timer
{
#if defined(ABSL_OS_FREE_RTOS)
	TimerHandle_t sw_timer_handle;
#elif defined(ABSL_LINUX)
	timer_t 	timer;
#endif
	timer_cb_t	timer_cb;
	void*		arg;
	absl_time_t   period;
	bool		reload;
}absl_timer_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_timer_create			absl_timer_create_freertos
#define absl_timer_start			absl_timer_start_freertos
#define absl_timer_stop			absl_timer_stop_freertos
#define absl_timer_change    		absl_timer_change_freertos
#elif defined(ABSL_LINUX)
#define absl_timer_create			absl_timer_create_posix
#define absl_timer_start			absl_timer_start_posix
#else
#error Platform not defined
#endif


absl_timer_rv_t absl_timer_create(absl_timer_t* _absl_timer, timer_cb_t _timer_cb, void* _arg, absl_time_t _period, bool _reload, bool _auto_start);

void absl_timer_start(absl_timer_t* _absl_timer);

void absl_timer_stop(absl_timer_t* _absl_timer);

void absl_timer_change(absl_timer_t* _absl_timer, absl_time_t _period, bool _auto_start);



#endif /* ABSL_TIMER */
#endif /* ABSL_TIMER_H_ */
