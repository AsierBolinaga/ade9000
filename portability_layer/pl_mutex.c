/*
 * pl_mutex.c
 *
 *  Created on: Jun 30, 2023
 *      Author: abolinaga
 */
#include "pl_mutex.h"
#ifdef PL_MUTEX
#ifdef PL_OS_FREE_RTOS

pl_mutex_rv_t pl_mutex_create_freertos(pl_mutex_t* _mutex)
{
	pl_mutex_rv_t mutex_rv = PL_MUTEX_RV_ERROR;

	_mutex->mutex = xSemaphoreCreateMutex();

	if( _mutex->mutex != NULL )
	{
		mutex_rv = PL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

pl_mutex_rv_t pl_mutex_take_freertos(pl_mutex_t* _mutex)
{
	pl_mutex_rv_t mutex_rv = PL_MUTEX_RV_ERROR;

	if (xSemaphoreTake(_mutex->mutex, portMAX_DELAY) == pdTRUE)
	{
		mutex_rv = PL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

pl_mutex_rv_t pl_mutex_give_freertos(pl_mutex_t* _mutex)
{
	pl_mutex_rv_t mutex_rv = PL_MUTEX_RV_ERROR;

	if (xSemaphoreGive(_mutex->mutex) == pdTRUE)
	{
		mutex_rv = PL_MUTEX_RV_OK;
	}

	return mutex_rv;
}

#endif
#endif
