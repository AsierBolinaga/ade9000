/*
 * absl_timer_freertos.c
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_timer.h"

#if defined(ABSL_OS_FREE_RTOS) && defined(ABSL_TIMER)

static void absl_timer_freertos_callback(TimerHandle_t xTimer)
{
	absl_timer_t* absl_timer = (absl_timer_t*)pvTimerGetTimerID(xTimer);

	absl_timer->timer_cb(absl_timer->arg);
}


absl_timer_rv_t absl_timer_create_freertos(absl_timer_t* _absl_timer, timer_cb_t _timer_cb, void* _arg, absl_time_t _period, bool _reload, bool _auto_start)
{
	absl_timer_rv_t timer_rv = ABSL_TIMER_RV_ERROR;
	TickType_t timer_period_ticks = absl_time_to_ms(_period) / portTICK_PERIOD_MS;

	_absl_timer->timer_cb = _timer_cb;
	_absl_timer->arg = _arg;

	_absl_timer->sw_timer_handle = xTimerCreate("SwTimer",           /* Text name. */
								 timer_period_ticks, 			   /* Timer period. */
								 _reload,             			   /* Enable auto reload. */
								 _absl_timer,
								 absl_timer_freertos_callback);      /* The callback function. */

	if(NULL != _absl_timer->sw_timer_handle)
	{
		if(_auto_start)
		{
			/* Start timer. */
			xTimerStart(_absl_timer->sw_timer_handle, 0);
		}
		timer_rv = ABSL_TIMER_RV_OK;
	}

	return timer_rv;
}

void absl_timer_start_freertos(absl_timer_t* _absl_timer)
{
	xTimerStart(_absl_timer->sw_timer_handle, 0);
}

void absl_timer_stop_freertos(absl_timer_t* _absl_timer)
{
	xTimerStop(_absl_timer->sw_timer_handle, 0);
}

void absl_timer_change_freertos(absl_timer_t* _absl_timer, absl_time_t _period, bool _auto_start)
{
	uint64_t period_ms = absl_time_to_ms(_period);
	if(period_ms != 0)
	{
		TickType_t timer_period_ticks = period_ms / portTICK_PERIOD_MS;
		xTimerChangePeriod( _absl_timer->sw_timer_handle, timer_period_ticks, 0);

		if(_auto_start)
		{
			/* Start timer. */
			xTimerStart(_absl_timer->sw_timer_handle, 0);
		}
	}
	else
	{
		xTimerStop(_absl_timer->sw_timer_handle, 0);
	}
}

#endif
