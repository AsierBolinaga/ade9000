/*
 * absl_event.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_event.h"
#ifdef ABSL_EVENT
#ifdef ABSL_LINUX
#include <pthread.h>
#include <errno.h>

absl_event_rv_t absl_event_create_posix(absl_event_t* _event)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_ERROR;

	if(!pthread_mutex_init(&_event->event_mutex, NULL))
	{
		if(-1 != pthread_cond_init(&_event->event_cond, NULL))
		{
			_event->event_mask = 0;
			/* The event group was created. */
			event_rv = ABSL_EVENT_RV_OK;
		}
	}

	return event_rv;
}

void absl_event_set_posix(absl_event_t* _absl_event, uint32_t _events_mask)
{
	_absl_event->event_mask |= _events_mask;
	pthread_cond_signal(&_absl_event->event_cond);
}

absl_event_rv_t absl_event_set_fromISR_posix(absl_event_t* _absl_event, uint32_t _events_mask)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_ERROR;

	_absl_event->event_mask |= _events_mask;
	pthread_cond_signal(&_absl_event->event_cond);

	return ABSL_EVENT_RV_OK;
}

void absl_event_wait_posix(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events)
{
	*_events  = 0;
	pthread_mutex_lock(&_absl_event->event_mutex);

	while(0 == *_events)
	{
		pthread_cond_wait(&_absl_event->event_cond, &_absl_event->event_mutex);
		*_events = _absl_event->event_mask & _events_mask_to_wait;
	}
	_absl_event->event_mask  &= ~*_events;

	pthread_mutex_unlock(&_absl_event->event_mutex);
}

absl_event_rv_t absl_event_timed_wait_posix(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_NO_EVENT;
	struct timespec timeout;
	int rv = 0;

	timeout.tv_sec =   _wait_ms % 1000;
	timeout.tv_nsec =  (_wait_ms - (timeout.tv_sec *1000)) * 10000000;

	*_events  = 0;
	pthread_mutex_lock(&_absl_event->event_mutex);

	while((0 == *_events) && (ETIMEDOUT != rv))
	{
		rv = pthread_cond_timedwait(&_absl_event->event_cond, &_absl_event->event_mutex, &timeout);
		*_events = _absl_event->event_mask & _events_mask_to_wait;
	}

	if(ETIMEDOUT != rv)
	{
		_absl_event->event_mask  &= ~*_events;
		event_rv = ABSL_EVENT_RV_OK;
	}

	pthread_mutex_unlock(&_absl_event->event_mutex);

	return event_rv;
}

#endif /* ABSL_LINUX */
#endif
