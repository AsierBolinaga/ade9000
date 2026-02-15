/*
 * pl_event.h
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_EVENT_H_
#define PL_EVENT_H_

#include "pl_config.h"
#ifdef PL_EVENT
#include "pl_types.h" 
#if defined(PL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#elif defined(PL_LINUX)
#include <signal.h>
#endif

#define PL_EVENT_NO_TIMEOUT 	0xFFFFFFFFUL
#define PL_EVENT_NO_WAIT		0x00000000UL

typedef enum pl_event_rv
{
	PL_EVENT_RV_OK = 0,
	PL_EVENT_RV_ERROR,
	PL_EVENT_RV_NO_EVENT,
	PL_EVENT_RV_NO_INITIALIZED,
}pl_event_rv_t;

typedef struct pl_event
{
#if defined(PL_OS_FREE_RTOS)
	EventGroupHandle_t event_group; 	/* FreeRTOS Event group handler */
#elif defined(PL_LINUX)
	pthread_cond_t 	event_cond;
	pthread_mutex_t event_mutex;
	uint32_t		event_mask;
#endif
}pl_event_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_event_create			pl_event_create_freertos
#define pl_event_set			pl_event_set_freertos
#define pl_event_set_fromISR    pl_event_set_fromISR_freertos
#define pl_event_wait			pl_event_wait_freertos
#define pl_event_timed_wait		pl_event_timed_wait_freertos
#define pl_event_clear_events	pl_event_clear_events_freertos
#define pl_event_clear_all		pl_event_clear_all_freertos
#elif defined(PL_LINUX)
#define pl_event_create			pl_event_create_posix
#define pl_event_set			pl_event_set_posix
#define pl_event_set_fromISR    pl_event_set_fromISR_posix
#define pl_event_wait			pl_event_wait_posix
#define pl_event_timed_wait		pl_event_timed_wait_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Creates the event group
 *
 * @param *_pl_event  Pointer to pl_event instance
 *
 * @return pl_event_rv_t 	 PL_EVENT_RV_OK 	if event group was correctly created
 * 							 PL_EVENT_RV_ERROR 	if there was an error on creation
 */
pl_event_rv_t pl_event_create(pl_event_t* _pl_event);

/*!
 * @brief Sets in the event group, the flags given in the event mask
 *
 * @param *_pl_event  	Pointer to pl_event instance
 * @param _events_mask	Mask with the flags to be set

 */
void pl_event_set(pl_event_t* _pl_event, uint32_t _events_mask);

/*!
 * @brief Sets in the event group, the flags given in the event mask. This function
 * 		  is to be used in ISRs.
 *
 * @param *_pl_event  	Pointer to pl_event instance
 * @param _events_mask	Mask with the flags to be set
 *
 * @return pl_event_rv_t 	 PL_EVENT_RV_OK 	if event was correctly set
 * 							 PL_EVENT_RV_ERROR 	if event was not set
 */
pl_event_rv_t pl_event_set_fromISR(pl_event_t* _pl_event, uint32_t _events_mask);

/*!
 * @brief waits to the specified event flags indefinitely
 *
 * @param *_pl_event  			Pointer to pl_event instance
 * @param _events_mask_to_wait	Mask with the flags to wait
 * @param _events				Pointer to the events that are activated
 */
pl_event_rv_t pl_event_wait(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events);

/*!
 * @brief waits to the specified event flags indefinitely
 *
 * @param *_pl_event  			Pointer to pl_event instance
 * @param _events_mask_to_wait	Mask with the flags to wait
 * @param _events				Pointer to the events that are activated
 * @param _wait_ms				Time to wait for the event in milliseconds.
 *
 * @return pl_event_rv_t 	 PL_EVENT_RV_OK 	if event occurred
 * 							 PL_EVENT_NO_EVENT  if no event occurred
 */
pl_event_rv_t pl_event_timed_wait(pl_event_t* _pl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms);


/*!
 * @brief Clears the bits of specific events
 *
 * @param *_pl_event  			Pointer to pl_event instance
 * @param _events_to_clear		Event bits to clear
 *
 */
void pl_event_clear_events(pl_event_t* _pl_event, uint32_t _events_to_clear);

/*!
 * @brief Clears the bits of specific events
 *
 * @param *_pl_event  			Pointer to pl_event instance
 *
 */
void pl_event_clear_all(pl_event_t* _pl_event);

#endif /* PL_EVENT */
#endif /* PL_EVENT_H_ */
