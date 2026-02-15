/*
 * pl_temperature.c
 *
 *  Created on: Oct 30, 2023
 *      Author: abolinaga
 */

#include "pl_temperature.h"

#ifdef PL_TEMPERATURE
#include "pl_types.h"

#ifdef PL_OS_FREE_RTOS
#include "fsl_tempmon.h"


pl_temperature_rv_t pl_temperature_init_freertos(void)
{
	tempmon_config_t config;

	TEMPMON_GetDefaultConfig(&config);

	TEMPMON_Init(TEMPMON, &config);
	TEMPMON_StartMeasure(TEMPMON);

	return PL_TEMPERATURE_RV_OK;
}

float pl_temperature_get_freertos(void)
{
	return TEMPMON_GetCurrentTemperature(TEMPMON);
}

#endif
#endif
