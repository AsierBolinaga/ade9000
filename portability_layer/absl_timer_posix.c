/*
 * absl_timer_freertos.c
 *
 *  Created on: 11 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_timer.h"

#if defined(ABSL_LINUX) && defined(ABSL_TIMER)
 #include <signal.h>

void absl_timer_expired(union sigval timer_data)
{
	absl_timer_t* absl_timer = (absl_timer_t*)timer_data.sival_ptr;
	if(absl_timer->reload)
	{
		absl_timer_start_posix(absl_timer);
	}
	absl_timer->timer_cb(absl_timer->arg);
}

absl_timer_rv_t absl_timer_create_posix(absl_timer_t* _absl_timer, timer_cb_t _timer_cb, void* _arg, absl_time_t _period, bool _reload, bool _auto_start)
{
	absl_timer_rv_t timer_rv = ABSL_TIMER_RV_ERROR;
	
    struct sigevent sev = { 0 };

    _absl_timer->timer_cb = _timer_cb;
	_absl_timer->arg = _arg;
	_absl_timer->period = _period;
	_absl_timer->reload = _reload;

	sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = absl_timer_expired;
	sev.sigev_value.sival_ptr = _absl_timer;
    sev.sigev_notify_attributes = NULL;
	
    if(!timer_create(CLOCK_MONOTONIC, &sev, &_absl_timer->timer))
	{
		// if(_auto_start)
		// {
			absl_timer_start_posix(_absl_timer);
		// }
		
       timer_rv = ABSL_TIMER_RV_OK;
    }

	return timer_rv;
}

void absl_timer_start_posix(absl_timer_t* _absl_timer)
{
	struct itimerspec trigger;

	trigger.it_value.tv_sec = _absl_timer->period.seconds;
	trigger.it_value.tv_nsec = _absl_timer->period.nseconds;
	trigger.it_interval.tv_sec = _absl_timer->period.seconds;
	trigger.it_interval.tv_nsec = _absl_timer->period.seconds;

	timer_settime(_absl_timer->timer, 0, &trigger, NULL);
}

void absl_timer_stop_posix(absl_timer_t* _absl_timer)
{
	struct itimerspec trigger;

	trigger.it_value.tv_sec = 0;
	trigger.it_value.tv_nsec = 0;
	trigger.it_interval.tv_sec = 0;
	trigger.it_interval.tv_nsec = 0;
	
	timer_settime(_absl_timer->timer, 0, &trigger, NULL);
}

#endif
