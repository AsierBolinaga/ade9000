/*
 * pl_temperature.h
 *
 *  Created on: Oct 30, 2023
 *      Author: abolinaga
 */

#ifndef PL_TEMPERATURE_H_
#define PL_TEMPERATURE_H_

#include "pl_config.h"
#ifdef PL_TEMPERATURE

typedef enum pl_temperature_rv
{
    PL_TEMPERATURE_RV_OK = 0x0U,
	PL_TEMPERATURE_RV_ERROR
} pl_temperature_rv_t;


#if defined(PL_OS_FREE_RTOS)
#define pl_temperature_init			pl_temperature_init_freertos
#define pl_temperature_get			pl_temperature_get_freertos
#else
#error Platform not defined
#endif

pl_temperature_rv_t pl_temperature_init(void);

float pl_temperature_get(void);

#endif
#endif /* PL_TEMPERATURE_H_ */
