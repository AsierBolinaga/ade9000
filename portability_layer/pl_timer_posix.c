/*
 * pl_timer_freertos.c
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_timer.h"

#if defined(PL_LINUX) && defined(PL_TIMER)
 #include <signal.h>

void pl_timer_expired(union sigval timer_data)
{
	pl_timer_t* pl_timer = (pl_timer_t*)timer_data.sival_ptr;
	if(pl_timer->reload)
	{
		pl_timer_start_posix(pl_timer);
	}
	pl_timer->timer_cb(pl_timer->arg);
}

pl_timer_rv_t pl_timer_create_posix(pl_timer_t* _pl_timer, timer_cb_t _timer_cb, void* _arg, pl_time_t _period, bool _reload, bool _auto_start)
{
	pl_timer_rv_t timer_rv = PL_TIMER_RV_ERROR;
	
    struct sigevent sev = { 0 };

    _pl_timer->timer_cb = _timer_cb;
	_pl_timer->arg = _arg;
	_pl_timer->period = _period;
	_pl_timer->reload = _reload;

	sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = pl_timer_expired;
	sev.sigev_value.sival_ptr = _pl_timer;
    sev.sigev_notify_attributes = NULL;
	
    if(!timer_create(CLOCK_MONOTONIC, &sev, &_pl_timer->timer))
	{
		// if(_auto_start)
		// {
			pl_timer_start_posix(_pl_timer);
		// }
		
       timer_rv = PL_TIMER_RV_OK;
    }

	return timer_rv;
}

void pl_timer_start_posix(pl_timer_t* _pl_timer)
{
	struct itimerspec trigger;

	trigger.it_value.tv_sec = _pl_timer->period.seconds;
	trigger.it_value.tv_nsec = _pl_timer->period.nseconds;
	trigger.it_interval.tv_sec = _pl_timer->period.seconds;
	trigger.it_interval.tv_nsec = _pl_timer->period.seconds;

	timer_settime(_pl_timer->timer, 0, &trigger, NULL);
}

void pl_timer_stop_posix(pl_timer_t* _pl_timer)
{
	struct itimerspec trigger;

	trigger.it_value.tv_sec = 0;
	trigger.it_value.tv_nsec = 0;
	trigger.it_interval.tv_sec = 0;
	trigger.it_interval.tv_nsec = 0;
	
	timer_settime(_pl_timer->timer, 0, &trigger, NULL);
}

#endif
