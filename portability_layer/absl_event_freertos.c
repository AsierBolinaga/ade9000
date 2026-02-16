/*
 * absl_event.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_event.h"
#ifdef ABSL_EVENT
#ifdef ABSL_OS_FREE_RTOS

absl_event_rv_t absl_event_create_freertos(absl_event_t* _event)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_ERROR;

	_event->event_group = xEventGroupCreate();
	if( _event->event_group == NULL )
	{
		/* The event group was not created because there was insufficient
		FreeRTOS heap available. */
	}
	else
	{
		/* The event group was created. */
		event_rv = ABSL_EVENT_RV_OK;
	}

	return event_rv;
}

void absl_event_set_freertos(absl_event_t* _absl_event, uint32_t _events_mask)
{
	xEventGroupSetBits(_absl_event->event_group, _events_mask);
}

absl_event_rv_t absl_event_set_fromISR_freertos(absl_event_t* _absl_event, uint32_t _events_mask)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_ERROR;

	BaseType_t xHigherPriorityTaskWoken, xResult;

	if(0 != _absl_event->event_group)
	{
		xHigherPriorityTaskWoken = pdFALSE;
		xResult = xEventGroupSetBitsFromISR(_absl_event->event_group, _events_mask, &xHigherPriorityTaskWoken);

		if( xResult != pdFAIL )
		{
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			event_rv = ABSL_EVENT_RV_OK;
		}
	}
	else
	{
		event_rv = ABSL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

absl_event_rv_t absl_event_wait_freertos(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_ERROR;

	if(0 != _absl_event->event_group)
	{
		*_events = 0;
		*_events = xEventGroupWaitBits(_absl_event->event_group, _events_mask_to_wait, pdFALSE, pdFALSE, portMAX_DELAY);
		xEventGroupClearBits(_absl_event->event_group, *_events);

		event_rv = ABSL_EVENT_RV_OK;
	}
	else
	{
		event_rv = ABSL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

absl_event_rv_t absl_event_timed_wait_freertos(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms)
{
	absl_event_rv_t event_rv = ABSL_EVENT_RV_NO_EVENT;

	TickType_t xTicksToWait;

	if(0 != _absl_event->event_group)
	{
		if(_wait_ms)
		{
			xTicksToWait = _wait_ms / portTICK_PERIOD_MS;
		}
		else
		{
			xTicksToWait = 0;
		}

		*_events = 0;
		*_events = xEventGroupWaitBits(_absl_event->event_group, _events_mask_to_wait, pdFALSE, pdFALSE, xTicksToWait);
		xEventGroupClearBits(_absl_event->event_group, *_events);

		if(*_events)
		{
			event_rv = ABSL_EVENT_RV_OK;
		}
	}
	else
	{
		event_rv = ABSL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

void absl_event_clear_events_freertos(absl_event_t* _absl_event, uint32_t _events_to_clear)
{
	xEventGroupClearBits(_absl_event->event_group, _events_to_clear);
}


void absl_event_clear_all_freertos(absl_event_t* _absl_event)
{
	xEventGroupClearBits(_absl_event->event_group, 0x00FFFFFF);
}

#endif /* ABSL_EVENT */
#endif
