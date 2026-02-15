/*
 * pl_queue_freertos.c
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_queue.h"
#ifdef PL_QUEUE
#ifdef PL_OS_FREE_RTOS

pl_queue_rv_t pl_queue_create_freertos(pl_queue_t* _pl_queue, uint32_t _item_size, uint32_t _max_item_length)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_ERROR;

	_pl_queue->queue = xQueueCreate(_max_item_length, _item_size);
	if(0 != _pl_queue->queue)
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_send_freertos(pl_queue_t* _pl_queue, void* _queue_item, float _try_ms)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_FULL;
	TickType_t xTicksToWait;

	if(PL_QUEUE_MAX_DELAY != _try_ms)
	{
		xTicksToWait = _try_ms / portTICK_PERIOD_MS;
	}
	else
	{
		xTicksToWait = portMAX_DELAY;
	}

	if( errQUEUE_FULL != xQueueSend(_pl_queue->queue, _queue_item, xTicksToWait))
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_send_fromISR(pl_queue_t* _pl_queue, queue_item_t* _queue_item)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_ERROR;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( errQUEUE_FULL == xQueueSendFromISR(_pl_queue->queue, _queue_item, &xHigherPriorityTaskWoken))
	{
		queue_rv = PL_QUEUE_RV_FULL;
	}
	else
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_receive_freertos(pl_queue_t* _pl_queue, void* _received_item, uint32_t _delay_ms)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_NO_ITEM;
	TickType_t    ticks_ms;

	if(PL_QUEUE_MAX_DELAY != _delay_ms)
	{
		ticks_ms = _delay_ms / portTICK_PERIOD_MS;
	}
	else
	{
		ticks_ms = portMAX_DELAY;
	}

	if (pdTRUE == xQueueReceive(_pl_queue->queue, _received_item, ticks_ms))
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

#endif /* PL_QUEUE */
#endif
