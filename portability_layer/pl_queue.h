/*
 * pl_queue.h
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_QUEUE_H_
#define PL_QUEUE_H_

#include "pl_config.h"
#ifdef PL_QUEUE
#include "pl_types.h"
#if defined(PL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "queue.h"
#elif defined(PL_LINUX)
#include <mqueue.h>
#endif

#define PL_QUEUE_MAX_DELAY 	0xFFFFFFFF
#define PL_QUEUE_NO_DELAY 	0x00000000

typedef enum pl_queue_rv
{
	PL_QUEUE_RV_OK = 0,
	PL_QUEUE_RV_FULL,
	PL_QUEUE_RV_NO_ITEM,
	PL_QUEUE_RV_ERROR
}pl_queue_rv_t;

typedef struct queue_item
{
	uint32_t	data_identifier;	/* Assign an ID to the data (if needed) */
	void*		data;				/* Data to queue */
}queue_item_t;

typedef struct pl_queue
{
#if defined(PL_OS_FREE_RTOS)
	QueueHandle_t 	queue;		/* FreeRTOS queue item */
#elif defined(PL_LINUX)
	mqd_t 		queue;
	uint32_t	item_size;
#endif
}pl_queue_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_queue_create			pl_queue_create_freertos
#define pl_queue_send			pl_queue_send_freertos
#define pl_queue_send_fromISR   pl_queue_send_fromISR_freertos
#define pl_queue_receive		pl_queue_receive_freertos
#elif defined(PL_LINUX)
#define pl_queue_create			pl_queue_create_posix
#define pl_queue_send			pl_queue_send_posix
#define pl_queue_send_fromISR   pl_queue_send_fromISR_posix
#define pl_queue_receive		pl_queue_receive_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Creates the queue
 *
 * @param *_pl_queue  		Pointer to pl_queue instance
 * @param _item_size 		Size of each queue item
 * @param _max_item_length 	Max amount of items in the queue
 *
 * @return pl_queue_rv_t 	 PL_QUEUE_RV_OK 	if queue was correctly created
 * 							 PL_QUEUE_RV_ERROR 	if there was an error on creation
 */
pl_queue_rv_t pl_queue_create(pl_queue_t* _pl_queue, uint32_t _item_size, uint32_t _max_item_length);

/*!
 * @brief Add an item to the queue
 *
 * @param *_pl_queue  		Pointer to pl_queue instance
 * @param *_queue_item 		Pointer to queue item to be added in the queue
 * @Param _try_ms			Time in ms to wait to make room if queue is full
 *
 * @return pl_queue_rv_t 	 PL_QUEUE_RV_OK 	if item was correctly added to queue
 * 							 PL_QUEUE_RV_FULL 	if there was no room to add the item
 */
pl_queue_rv_t pl_queue_send(pl_queue_t* _pl_queue, void* _queue_item, float _try_ms);

/*!
 * @brief Add an item to the queue from an ISR
 *
 * @param *_pl_queue  		Pointer to pl_queue instance
 * @param *_queue_item 		Pointer to queue item to be added in the queue
 *
 * @return pl_queue_rv_t 	 PL_QUEUE_RV_OK 	if item was correctly added to queue
 * 							 PL_QUEUE_RV_FULL 	if there was no room to add the item
 */
pl_queue_rv_t pl_queue_send_fromISR(pl_queue_t* _pl_queue, queue_item_t* _queue_item);

/*!
 * @brief Add an item to the queue
 *
 * @param *_pl_queue  		Pointer to pl_queue instance
 * @param *_queue_item 		Pointer to obtained item
 * @Param _delay_ms			Time in ms to wait for an item to be received
 *
 * @return pl_queue_rv_t 	 PL_QUEUE_RV_OK 		if item was obtained from queue
 * 							 PL_QUEUE_RV_NO_ITEM 	if no tiem was obtained from queue
 */
pl_queue_rv_t pl_queue_receive(pl_queue_t* _pl_queue, void* _received_item, uint32_t _delay_ms);

#endif /* PL_QUEUE */
#endif /* PL_QUEUE_H_ */
