/*
 * absl_mutex.h
 *
 *  Created on: Jun 30, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_MUTEX_H_
#define ABSL_MUTEX_H_

#include "absl_config.h"
#ifdef ABSL_MUTEX

#include "FreeRTOS.h"
#include "semphr.h"

typedef enum absl_mutex_rv
{
	ABSL_MUTEX_RV_OK = 0,
	ABSL_MUTEX_RV_ERROR
}absl_mutex_rv_t;

typedef struct absl_mutex
{
	SemaphoreHandle_t 	mutex;
}absl_mutex_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_mutex_create		absl_mutex_create_freertos
#define absl_mutex_take		absl_mutex_take_freertos
#define absl_mutex_give		absl_mutex_give_freertos
#else
#error Platform not defined
#endif

absl_mutex_rv_t absl_mutex_create(absl_mutex_t* _mutex);

absl_mutex_rv_t absl_mutex_take(absl_mutex_t* _mutex);

absl_mutex_rv_t absl_mutex_give(absl_mutex_t* _mutex);

#endif
#endif /* ABSL_MUTEX_H_ */
