/*
 * absl_queue_freertos.c
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_queue.h"
#ifdef ABSL_QUEUE
#ifdef ABSL_OS_FREE_RTOS

absl_queue_rv_t absl_queue_create_freertos(absl_queue_t* _absl_queue, uint32_t _item_size, uint32_t _max_item_length)
{
	absl_queue_rv_t queue_rv = ABSL_QUEUE_RV_ERROR;

	_absl_queue->queue = xQueueCreate(_max_item_length, _item_size);
	if(0 != _absl_queue->queue)
	{
		queue_rv = ABSL_QUEUE_RV_OK;
	}

	return queue_rv;
}

absl_queue_rv_t absl_queue_send_freertos(absl_queue_t* _absl_queue, void* _queue_item, float _try_ms)
{
	absl_queue_rv_t queue_rv = ABSL_QUEUE_RV_FULL;
	TickType_t xTicksToWait;

	if(ABSL_QUEUE_MAX_DELAY != _try_ms)
	{
		xTicksToWait = _try_ms / portTICK_PERIOD_MS;
	}
	else
	{
		xTicksToWait = portMAX_DELAY;
	}

	if( errQUEUE_FULL != xQueueSend(_absl_queue->queue, _queue_item, xTicksToWait))
	{
		queue_rv = ABSL_QUEUE_RV_OK;
	}

	return queue_rv;
}

absl_queue_rv_t absl_queue_send_fromISR(absl_queue_t* _absl_queue, queue_item_t* _queue_item)
{
	absl_queue_rv_t queue_rv = ABSL_QUEUE_RV_ERROR;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( errQUEUE_FULL == xQueueSendFromISR(_absl_queue->queue, _queue_item, &xHigherPriorityTaskWoken))
	{
		queue_rv = ABSL_QUEUE_RV_FULL;
	}
	else
	{
		queue_rv = ABSL_QUEUE_RV_OK;
	}

	return queue_rv;
}

absl_queue_rv_t absl_queue_receive_freertos(absl_queue_t* _absl_queue, void* _received_item, uint32_t _delay_ms)
{
	absl_queue_rv_t queue_rv = ABSL_QUEUE_RV_NO_ITEM;
	TickType_t    ticks_ms;

	if(ABSL_QUEUE_MAX_DELAY != _delay_ms)
	{
		ticks_ms = _delay_ms / portTICK_PERIOD_MS;
	}
	else
	{
		ticks_ms = portMAX_DELAY;
	}

	if (pdTRUE == xQueueReceive(_absl_queue->queue, _received_item, ticks_ms))
	{
		queue_rv = ABSL_QUEUE_RV_OK;
	}

	return queue_rv;
}

absl_queue_rv_t absl_queue_receive_freertos_fromISR(absl_queue_t* _absl_queue, void* _received_item)
{
	absl_queue_rv_t queue_rv = ABSL_QUEUE_RV_NO_ITEM;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;


	if (pdTRUE == xQueueReceiveFromISR(_absl_queue->queue, _received_item, &xHigherPriorityTaskWoken))
	{
		queue_rv = ABSL_QUEUE_RV_OK;
	}

	return queue_rv;
}

#endif /* ABSL_QUEUE */
#endif
