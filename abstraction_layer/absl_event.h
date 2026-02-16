/*
 * absl_event.h
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_EVENT_H_
#define ABSL_EVENT_H_

#include "absl_config.h"
#ifdef ABSL_EVENT
#include "absl_types.h" 
#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#elif defined(ABSL_LINUX)
#include <signal.h>
#endif

#define ABSL_EVENT_NO_TIMEOUT 	0xFFFFFFFFUL
#define ABSL_EVENT_NO_WAIT		0x00000000UL

typedef enum absl_event_rv
{
	ABSL_EVENT_RV_OK = 0,
	ABSL_EVENT_RV_ERROR,
	ABSL_EVENT_RV_NO_EVENT,
	ABSL_EVENT_RV_NO_INITIALIZED,
}absl_event_rv_t;

typedef struct absl_event
{
#if defined(ABSL_OS_FREE_RTOS)
	EventGroupHandle_t event_group; 	/* FreeRTOS Event group handler */
#elif defined(ABSL_LINUX)
	pthread_cond_t 	event_cond;
	pthread_mutex_t event_mutex;
	uint32_t		event_mask;
#endif
}absl_event_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_event_create			absl_event_create_freertos
#define absl_event_set			absl_event_set_freertos
#define absl_event_set_fromISR    absl_event_set_fromISR_freertos
#define absl_event_wait			absl_event_wait_freertos
#define absl_event_timed_wait		absl_event_timed_wait_freertos
#define absl_event_clear_events	absl_event_clear_events_freertos
#define absl_event_clear_all		absl_event_clear_all_freertos
#elif defined(ABSL_LINUX)
#define absl_event_create			absl_event_create_posix
#define absl_event_set			absl_event_set_posix
#define absl_event_set_fromISR    absl_event_set_fromISR_posix
#define absl_event_wait			absl_event_wait_posix
#define absl_event_timed_wait		absl_event_timed_wait_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Creates the event group
 *
 * @param *_absl_event  Pointer to absl_event instance
 *
 * @return absl_event_rv_t 	 ABSL_EVENT_RV_OK 	if event group was correctly created
 * 							 ABSL_EVENT_RV_ERROR 	if there was an error on creation
 */
absl_event_rv_t absl_event_create(absl_event_t* _absl_event);

/*!
 * @brief Sets in the event group, the flags given in the event mask
 *
 * @param *_absl_event  	Pointer to absl_event instance
 * @param _events_mask	Mask with the flags to be set

 */
void absl_event_set(absl_event_t* _absl_event, uint32_t _events_mask);

/*!
 * @brief Sets in the event group, the flags given in the event mask. This function
 * 		  is to be used in ISRs.
 *
 * @param *_absl_event  	Pointer to absl_event instance
 * @param _events_mask	Mask with the flags to be set
 *
 * @return absl_event_rv_t 	 ABSL_EVENT_RV_OK 	if event was correctly set
 * 							 ABSL_EVENT_RV_ERROR 	if event was not set
 */
absl_event_rv_t absl_event_set_fromISR(absl_event_t* _absl_event, uint32_t _events_mask);

/*!
 * @brief waits to the specified event flags indefinitely
 *
 * @param *_absl_event  			Pointer to absl_event instance
 * @param _events_mask_to_wait	Mask with the flags to wait
 * @param _events				Pointer to the events that are activated
 */
absl_event_rv_t absl_event_wait(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events);

/*!
 * @brief waits to the specified event flags indefinitely
 *
 * @param *_absl_event  			Pointer to absl_event instance
 * @param _events_mask_to_wait	Mask with the flags to wait
 * @param _events				Pointer to the events that are activated
 * @param _wait_ms				Time to wait for the event in milliseconds.
 *
 * @return absl_event_rv_t 	 ABSL_EVENT_RV_OK 	if event occurred
 * 							 ABSL_EVENT_NO_EVENT  if no event occurred
 */
absl_event_rv_t absl_event_timed_wait(absl_event_t* _absl_event, uint32_t _events_mask_to_wait, uint32_t* _events, uint32_t _wait_ms);


/*!
 * @brief Clears the bits of specific events
 *
 * @param *_absl_event  			Pointer to absl_event instance
 * @param _events_to_clear		Event bits to clear
 *
 */
void absl_event_clear_events(absl_event_t* _absl_event, uint32_t _events_to_clear);

/*!
 * @brief Clears the bits of specific events
 *
 * @param *_absl_event  			Pointer to absl_event instance
 *
 */
void absl_event_clear_all(absl_event_t* _absl_event);

#endif /* ABSL_EVENT */
#endif /* ABSL_EVENT_H_ */
