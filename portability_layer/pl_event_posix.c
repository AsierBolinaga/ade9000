/*
 * pl_event.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_event.h"
#ifdef PL_EVENT
#ifdef PL_LINUX
#include <pthread.h>
#include <errno.h>

pl_event_rv_t pl_event_create_posix(pl_event_t* _event)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_ERROR;

	if(!pthread_mutex_init(&_event->event_mutex, NULL))
	{
		if(-1 != pthread_cond_init(&_event->event_cond, NULL))
		{
			_event->event_mask = 0;
			/* The event group was created. */
			event_rv = PL_EVENT_RV_OK;
		}
	}

	return event_rv;
}

void pl_event_set_posix(pl_event_t* _pl_event, uint32_t _events_mask)
{
	_pl_event->event_mask |= _events_mask;
	pthread_cond_signal(&_pl_event->event_cond);
}

pl_event_rv_t pl_event_set_fromISR_posix(pl_event_t* _pl_event, uint32_t _events_mask)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_ERROR;

	_pl_event->event_mask |= _events_mask;
	pthread_cond_signal(&_pl_event->event_cond);

	return PL_EVENT_RV_OK;
}

void pl_event_wait_posix(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events)
{
	*_events  = 0;
	pthread_mutex_lock(&_pl_event->event_mutex);

	while(0 == *_events)
	{
		pthread_cond_wait(&_pl_event->event_cond, &_pl_event->event_mutex);
		*_events = _pl_event->event_mask & _events_mask_to_wait;
	}
	_pl_event->event_mask  &= ~*_events;

	pthread_mutex_unlock(&_pl_event->event_mutex);
}

pl_event_rv_t pl_event_timed_wait_posix(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_NO_EVENT;
	struct timespec timeout;
	int rv = 0;

	timeout.tv_sec =   _wait_ms % 1000;
	timeout.tv_nsec =  (_wait_ms - (timeout.tv_sec *1000)) * 10000000;

	*_events  = 0;
	pthread_mutex_lock(&_pl_event->event_mutex);

	while((0 == *_events) && (ETIMEDOUT != rv))
	{
		rv = pthread_cond_timedwait(&_pl_event->event_cond, &_pl_event->event_mutex, &timeout);
		*_events = _pl_event->event_mask & _events_mask_to_wait;
	}

	if(ETIMEDOUT != rv)
	{
		_pl_event->event_mask  &= ~*_events;
		event_rv = PL_EVENT_RV_OK;
	}

	pthread_mutex_unlock(&_pl_event->event_mutex);

	return event_rv;
}

#endif /* PL_LINUX */
#endif
