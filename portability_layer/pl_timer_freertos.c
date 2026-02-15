/*
 * pl_timer_freertos.c
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_timer.h"

#if defined(PL_OS_FREE_RTOS) && defined(PL_TIMER)

static void pl_timer_freertos_callback(TimerHandle_t xTimer)
{
	pl_timer_t* pl_timer = (pl_timer_t*)pvTimerGetTimerID(xTimer);

	pl_timer->timer_cb(pl_timer->arg);
}


pl_timer_rv_t pl_timer_create_freertos(pl_timer_t* _pl_timer, timer_cb_t _timer_cb, void* _arg, pl_time_t _period, bool _reload, bool _auto_start)
{
	pl_timer_rv_t timer_rv = PL_TIMER_RV_ERROR;
	TickType_t timer_period_ticks = pl_time_to_ms(_period) / portTICK_PERIOD_MS;

	_pl_timer->timer_cb = _timer_cb;
	_pl_timer->arg = _arg;

	_pl_timer->sw_timer_handle = xTimerCreate("SwTimer",           /* Text name. */
								 timer_period_ticks, 			   /* Timer period. */
								 _reload,             			   /* Enable auto reload. */
								 _pl_timer,
								 pl_timer_freertos_callback);      /* The callback function. */

	if(NULL != _pl_timer->sw_timer_handle)
	{
		if(_auto_start)
		{
			/* Start timer. */
			xTimerStart(_pl_timer->sw_timer_handle, 0);
		}
		timer_rv = PL_TIMER_RV_OK;
	}

	return timer_rv;
}

void pl_timer_start_freertos(pl_timer_t* _pl_timer)
{
	xTimerStart(_pl_timer->sw_timer_handle, 0);
}

void pl_timer_stop_freertos(pl_timer_t* _pl_timer)
{
	xTimerStop(_pl_timer->sw_timer_handle, 0);
}

void pl_timer_change_freertos(pl_timer_t* _pl_timer, pl_time_t _period, bool _auto_start)
{
	uint64_t period_ms = pl_time_to_ms(_period);
	if(period_ms != 0)
	{
		TickType_t timer_period_ticks = period_ms / portTICK_PERIOD_MS;
		xTimerChangePeriod( _pl_timer->sw_timer_handle, timer_period_ticks, 0);

		if(_auto_start)
		{
			/* Start timer. */
			xTimerStart(_pl_timer->sw_timer_handle, 0);
		}
	}
	else
	{
		xTimerStop(_pl_timer->sw_timer_handle, 0);
	}
}

#endif
