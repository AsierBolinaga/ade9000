/*
 * pl_thread.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_THREAD_H_
#define PL_THREAD_H_

#include "pl_config.h"
#ifdef PL_THREAD
#include "pl_types.h"

#if defined(PL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "task.h"
#elif defined(PL_LINUX)
#include <pthread.h>
#endif

typedef enum pl_thread_rv
{
    PL_THREAD_RV_OK = 0x0U,
    PL_THREAD_RV_ERROR
} pl_thread_rv_t;


typedef void (*pl_thread_entry_t)(void* arg);

typedef struct pl_thread
{
#if defined(PL_LINUX)
	pthread_t  			pthread;
#endif
	char*	 			pl_thread_name;
	uint32_t 			pl_thread_priority;
	uint32_t			pl_thread_stack_size;
	void* 				args;
	pl_thread_entry_t	pl_thread_entry;
	TaskHandle_t 		task_handle;
} pl_thread_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_thread_run				pl_thread_run_freertos
#define pl_thread_create 			pl_thread_create_freertos
#define pl_thread_sleep				pl_thread_sleep_freertos
#define pl_thread_delete			pl_thread_delete_freertos
#define pl_thread_actual_delete		pl_thread_actual_delete_freertos
#elif defined(PL_LINUX)
#define pl_thread_run		pl_thread_run_posix
#define pl_thread_create	pl_thread_create_posix
#define pl_thread_sleep		pl_thread_sleep_posix
#else
#error Platform not defined
#endif

/*!
 * @brief Executes the OS scheduler
 *
 */
void pl_thread_run(void);

/*!
 * @brief Sleep the actual thread for the specified time
 *
 * @param _ms  	Time that the thread will be sleeping
 *
 */
void pl_thread_sleep(uint32_t _ms);

/*!
 * @brief Creates the thread with the parameters specified in pl_thread_configuration_t instance
 *
 * @param *_pl_thread_config  	Pointer to thread configuration instance
 *
 * @return pl_event_rv_t 	 PL_THREAD_RV_OK 	 if thread was correctly created
 * 							 PL_THREAD_RV_ERROR  if an error occurred creating the thread
 */
pl_thread_rv_t pl_thread_create(pl_thread_t* _pl_thread, char* _pl_thread_name, pl_thread_entry_t _pl_thread_entry,
								uint32_t _pl_thread_priority, uint32_t _pl_thread_stack_size, void* _args);

void pl_thread_delete(pl_thread_t* _pl_thread);

void pl_thread_actual_delete(void);

#endif /* PL_I2C */
#endif /* PL_THREAD_H_ */
