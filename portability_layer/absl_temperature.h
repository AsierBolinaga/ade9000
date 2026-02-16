/*
 * absl_temperature.h
 *
 *  Created on: Oct 30, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_TEMPERATURE_H_
#define ABSL_TEMPERATURE_H_

#include "absl_config.h"
#ifdef ABSL_TEMPERATURE

typedef enum absl_temperature_rv
{
    ABSL_TEMPERATURE_RV_OK = 0x0U,
	ABSL_TEMPERATURE_RV_ERROR
} absl_temperature_rv_t;


#if defined(ABSL_OS_FREE_RTOS)
#define absl_temperature_init			absl_temperature_init_freertos
#define absl_temperature_get			absl_temperature_get_freertos
#else
#error Platform not defined
#endif

absl_temperature_rv_t absl_temperature_init(void);

float absl_temperature_get(void);

#endif
#endif /* ABSL_TEMPERATURE_H_ */
