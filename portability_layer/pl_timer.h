/*
 * pl_timer.h
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_TIMER_H_
#define PL_TIMER_H_

#include "pl_config.h"
#ifdef PL_TIMER

#include "pl_types.h"
#include "pl_time.h"

#if defined(PL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "timers.h"
#elif defined(PL_LINUX)
#include <time.h>
#endif

typedef enum pl_timer_rv
{
	PL_TIMER_RV_OK = 0,
	PL_TIMER_RV_ERROR
}pl_timer_rv_t;

typedef void (*timer_cb_t)(void* arg);

typedef struct pl_timer
{
#if defined(PL_OS_FREE_RTOS)
	TimerHandle_t sw_timer_handle;
#elif defined(PL_LINUX)
	timer_t 	timer;
#endif
	timer_cb_t	timer_cb;
	void*		arg;
	pl_time_t   period;
	bool		reload;
}pl_timer_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_timer_create			pl_timer_create_freertos
#define pl_timer_start			pl_timer_start_freertos
#define pl_timer_stop			pl_timer_stop_freertos
#define pl_timer_change    		pl_timer_change_freertos
#elif defined(PL_LINUX)
#define pl_timer_create			pl_timer_create_posix
#define pl_timer_start			pl_timer_start_posix
#else
#error Platform not defined
#endif


pl_timer_rv_t pl_timer_create(pl_timer_t* _pl_timer, timer_cb_t _timer_cb, void* _arg, pl_time_t _period, bool _reload, bool _auto_start);

void pl_timer_start(pl_timer_t* _pl_timer);

void pl_timer_stop(pl_timer_t* _pl_timer);

void pl_timer_change(pl_timer_t* _pl_timer, pl_time_t _period, bool _auto_start);



#endif /* PL_TIMER */
#endif /* PL_TIMER_H_ */
