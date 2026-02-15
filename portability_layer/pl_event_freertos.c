/*
 * pl_event.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_event.h"
#ifdef PL_EVENT
#ifdef PL_OS_FREE_RTOS

pl_event_rv_t pl_event_create_freertos(pl_event_t* _event)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_ERROR;

	_event->event_group = xEventGroupCreate();
	if( _event->event_group == NULL )
	{
		/* The event group was not created because there was insufficient
		FreeRTOS heap available. */
	}
	else
	{
		/* The event group was created. */
		event_rv = PL_EVENT_RV_OK;
	}

	return event_rv;
}

void pl_event_set_freertos(pl_event_t* _pl_event, uint32_t _events_mask)
{
	xEventGroupSetBits(_pl_event->event_group, _events_mask);
}

pl_event_rv_t pl_event_set_fromISR_freertos(pl_event_t* _pl_event, uint32_t _events_mask)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_ERROR;

	BaseType_t xHigherPriorityTaskWoken, xResult;

	if(0 != _pl_event->event_group)
	{
		xHigherPriorityTaskWoken = pdFALSE;
		xResult = xEventGroupSetBitsFromISR(_pl_event->event_group, _events_mask, &xHigherPriorityTaskWoken);

		if( xResult != pdFAIL )
		{
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			event_rv = PL_EVENT_RV_OK;
		}
	}
	else
	{
		event_rv = PL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

pl_event_rv_t pl_event_wait_freertos(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_ERROR;

	if(0 != _pl_event->event_group)
	{
		*_events = 0;
		*_events = xEventGroupWaitBits(_pl_event->event_group, _events_mask_to_wait, pdFALSE, pdFALSE, portMAX_DELAY);
		xEventGroupClearBits(_pl_event->event_group, *_events);

		event_rv = PL_EVENT_RV_OK;
	}
	else
	{
		event_rv = PL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

pl_event_rv_t pl_event_timed_wait_freertos(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms)
{
	pl_event_rv_t event_rv = PL_EVENT_RV_NO_EVENT;

	TickType_t xTicksToWait;

	if(0 != _pl_event->event_group)
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
		*_events = xEventGroupWaitBits(_pl_event->event_group, _events_mask_to_wait, pdFALSE, pdFALSE, xTicksToWait);
		xEventGroupClearBits(_pl_event->event_group, *_events);

		if(*_events)
		{
			event_rv = PL_EVENT_RV_OK;
		}
	}
	else
	{
		event_rv = PL_EVENT_RV_NO_INITIALIZED;
	}

	return event_rv;
}

void pl_event_clear_events_freertos(pl_event_t* _pl_event, uint32_t _events_to_clear)
{
	xEventGroupClearBits(_pl_event->event_group, _events_to_clear);
}


void pl_event_clear_all_freertos(pl_event_t* _pl_event)
{
	xEventGroupClearBits(_pl_event->event_group, 0x00FFFFFF);
}

#endif /* PL_EVENT */
#endif
