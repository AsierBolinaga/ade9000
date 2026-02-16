/*
 * absl_queue.h
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_QUEUE_H_
#define ABSL_QUEUE_H_

#include "absl_config.h"
#ifdef ABSL_QUEUE
#include "absl_types.h"
#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "queue.h"
#elif defined(ABSL_LINUX)
#include <mqueue.h>
#endif

#define ABSL_QUEUE_MAX_DELAY 	0xFFFFFFFF
#define ABSL_QUEUE_NO_DELAY 	0x00000000

typedef enum absl_queue_rv
{
	ABSL_QUEUE_RV_OK = 0,
	ABSL_QUEUE_RV_FULL,
	ABSL_QUEUE_RV_NO_ITEM,
	ABSL_QUEUE_RV_ERROR
}absl_queue_rv_t;

typedef struct queue_item
{
	uint32_t	data_identifier;	/* Assign an ID to the data (if needed) */
	void*		data;				/* Data to queue */
}queue_item_t;

typedef struct absl_queue
{
#if defined(ABSL_OS_FREE_RTOS)
	QueueHandle_t 	queue;		/* FreeRTOS queue item */
#elif defined(ABSL_LINUX)
	mqd_t 		queue;
	uint32_t	item_size;
#endif
}absl_queue_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_queue_create			absl_queue_create_freertos
#define absl_queue_send			absl_queue_send_freertos
#define absl_queue_send_fromISR   absl_queue_send_fromISR_freertos
#define absl_queue_receive		absl_queue_receive_freertos
#define absl_queue_receive_fromISR	absl_queue_receive_fromISR_freertos
#elif defined(ABSL_LINUX)
#define absl_queue_create			absl_queue_create_posix
#define absl_queue_send			absl_queue_send_posix
#define absl_queue_send_fromISR   absl_queue_send_fromISR_posix
#define absl_queue_receive		absl_queue_receive_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Creates the queue
 *
 * @param *_absl_queue  		Pointer to absl_queue instance
 * @param _item_size 		Size of each queue item
 * @param _max_item_length 	Max amount of items in the queue
 *
 * @return absl_queue_rv_t 	 ABSL_QUEUE_RV_OK 	if queue was correctly created
 * 							 ABSL_QUEUE_RV_ERROR 	if there was an error on creation
 */
absl_queue_rv_t absl_queue_create(absl_queue_t* _absl_queue, uint32_t _item_size, uint32_t _max_item_length);

/*!
 * @brief Add an item to the queue
 *
 * @param *_absl_queue  		Pointer to absl_queue instance
 * @param *_queue_item 		Pointer to queue item to be added in the queue
 * @Param _try_ms			Time in ms to wait to make room if queue is full
 *
 * @return absl_queue_rv_t 	 ABSL_QUEUE_RV_OK 	if item was correctly added to queue
 * 							 ABSL_QUEUE_RV_FULL 	if there was no room to add the item
 */
absl_queue_rv_t absl_queue_send(absl_queue_t* _absl_queue, void* _queue_item, float _try_ms);

/*!
 * @brief Add an item to the queue from an ISR
 *
 * @param *_absl_queue  		Pointer to absl_queue instance
 * @param *_queue_item 		Pointer to queue item to be added in the queue
 *
 * @return absl_queue_rv_t 	 ABSL_QUEUE_RV_OK 	if item was correctly added to queue
 * 							 ABSL_QUEUE_RV_FULL 	if there was no room to add the item
 */
absl_queue_rv_t absl_queue_send_fromISR(absl_queue_t* _absl_queue, queue_item_t* _queue_item);

/*!
 * @brief Add an item to the queue
 *
 * @param *_absl_queue  		Pointer to absl_queue instance
 * @param *_queue_item 		Pointer to obtained item
 * @Param _delay_ms			Time in ms to wait for an item to be received
 *
 * @return absl_queue_rv_t 	 ABSL_QUEUE_RV_OK 		if item was obtained from queue
 * 							 ABSL_QUEUE_RV_NO_ITEM 	if no tiem was obtained from queue
 */
absl_queue_rv_t absl_queue_receive(absl_queue_t* _absl_queue, void* _received_item, uint32_t _delay_ms);

/*!
 * @brief Add an item to the queue
 *
 * @param *_absl_queue  		Pointer to absl_queue instance
 * @param *_queue_item 		Pointer to obtained item
 * @Param _delay_ms			Time in ms to wait for an item to be received
 *
 * @return absl_queue_rv_t 	 absl_QUEUE_RV_OK 		if item was obtained from queue
 * 							 absl_QUEUE_RV_NO_ITEM 	if no tiem was obtained from queue
 */
absl_queue_rv_t absl_queue_receive_fromISR(absl_queue_t* _absl_queue, void* _received_item);

#endif /* ABSL_QUEUE */
#endif /* ABSL_QUEUE_H_ */
