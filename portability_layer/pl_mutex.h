/*
 * pl_mutex.h
 *
 *  Created on: Jun 30, 2023
 *      Author: abolinaga
 */

#ifndef PL_MUTEX_H_
#define PL_MUTEX_H_

#include "pl_config.h"
#ifdef PL_MUTEX

#include "FreeRTOS.h"
#include "semphr.h"

typedef enum pl_mutex_rv
{
	PL_MUTEX_RV_OK = 0,
	PL_MUTEX_RV_ERROR
}pl_mutex_rv_t;

typedef struct pl_mutex
{
	SemaphoreHandle_t 	mutex;
}pl_mutex_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_mutex_create		pl_mutex_create_freertos
#define pl_mutex_take		pl_mutex_take_freertos
#define pl_mutex_give		pl_mutex_give_freertos
#else
#error Platform not defined
#endif

pl_mutex_rv_t pl_mutex_create(pl_mutex_t* _mutex);

pl_mutex_rv_t pl_mutex_take(pl_mutex_t* _mutex);

pl_mutex_rv_t pl_mutex_give(pl_mutex_t* _mutex);

#endif
#endif /* PL_MUTEX_H_ */
