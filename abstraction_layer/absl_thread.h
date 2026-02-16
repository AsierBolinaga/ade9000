/*
 * absl_thread.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_THREAD_H_
#define ABSL_THREAD_H_

#include "absl_config.h"
#ifdef ABSL_THREAD
#include "absl_types.h"

#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#elif defined(ABSL_LINUX)
#include <pthread.h>
#endif

typedef enum absl_thread_rv
{
    ABSL_THREAD_RV_OK = 0x0U,
    ABSL_THREAD_RV_ERROR
} absl_thread_rv_t;


typedef void (*absl_thread_entry_t)(void* arg);

typedef struct absl_thread
{
#if defined(ABSL_LINUX)
	pthread_t  			pthread;
#endif
	char*	 			absl_thread_name;
	uint32_t 			absl_thread_priority;
	uint32_t			absl_thread_stack_size;
	void* 				args;
	absl_thread_entry_t	absl_thread_entry;
	TaskHandle_t 		task_handle;
} absl_thread_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_thread_run				absl_thread_run_freertos
#define absl_thread_create 			absl_thread_create_freertos
#define absl_thread_sleep				absl_thread_sleep_freertos
#define absl_thread_delete			absl_thread_delete_freertos
#define absl_thread_actual_delete		absl_thread_actual_delete_freertos
#elif defined(ABSL_LINUX)
#define absl_thread_run		absl_thread_run_posix
#define absl_thread_create	absl_thread_create_posix
#define absl_thread_sleep		absl_thread_sleep_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Executes the OS scheduler
 *
 */
void absl_thread_run(void);

/*!
 * @brief Sleep the actual thread for the specified time
 *
 * @param _ms  	Time that the thread will be sleeping
 *
 */
void absl_thread_sleep(uint32_t _ms);

/*!
 * @brief Creates the thread with the parameters specified in absl_thread_configuration_t instance
 *
 * @param *_absl_thread_config  	Pointer to thread configuration instance
 *
 * @return absl_event_rv_t 	 ABSL_THREAD_RV_OK 	 if thread was correctly created
 * 							 ABSL_THREAD_RV_ERROR  if an error occurred creating the thread
 */
absl_thread_rv_t absl_thread_create(absl_thread_t* _absl_thread, char* _absl_thread_name, absl_thread_entry_t _absl_thread_entry,
								uint32_t _absl_thread_priority, uint32_t _absl_thread_stack_size, void* _args);

void absl_thread_delete(absl_thread_t* _absl_thread);

void absl_thread_actual_delete(void);

#endif /* ABSL_I2C */
#endif /* ABSL_THREAD_H_ */
