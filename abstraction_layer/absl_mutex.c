/*
 * absl_mutex.c
 *
 *  Created on: Jun 30, 2023
 *      Author: abolinaga
 */
#include "absl_mutex.h"
#ifdef ABSL_MUTEX
#ifdef ABSL_OS_FREE_RTOS

absl_mutex_rv_t absl_mutex_create_freertos(absl_mutex_t* _mutex)
{
	absl_mutex_rv_t mutex_rv = ABSL_MUTEX_RV_ERROR;

	_mutex->mutex = xSemaphoreCreateMutex();

	if( _mutex->mutex != NULL )
	{
		mutex_rv = ABSL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

absl_mutex_rv_t absl_mutex_take_freertos(absl_mutex_t* _mutex)
{
	absl_mutex_rv_t mutex_rv = ABSL_MUTEX_RV_ERROR;

	if (xSemaphoreTake(_mutex->mutex, portMAX_DELAY) == pdTRUE)
	{
		mutex_rv = ABSL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

absl_mutex_rv_t absl_mutex_give_freertos(absl_mutex_t* _mutex)
{
	absl_mutex_rv_t mutex_rv = ABSL_MUTEX_RV_ERROR;

	if (xSemaphoreGive(_mutex->mutex) == pdTRUE)
	{
		mutex_rv = ABSL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

#endif
#endif
