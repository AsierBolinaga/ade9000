/*
 * absl_temperature.c
 *
 *  Created on: Oct 30, 2023
 *      Author: abolinaga
 */

#include "absl_temperature.h"

#ifdef ABSL_TEMPERATURE
#include "absl_types.h"

#ifdef ABSL_OS_FREE_RTOS
#include "fsl_tempmon.h"


absl_temperature_rv_t absl_temperature_init_freertos(void)
{
	tempmon_config_t config;

	TEMPMON_GetDefaultConfig(&config);

	TEMPMON_Init(TEMPMON, &config);
	TEMPMON_StartMeasure(TEMPMON);

	return ABSL_TEMPERATURE_RV_OK;
}

float absl_temperature_get_freertos(void)
{
	return TEMPMON_GetCurrentTemperature(TEMPMON);
}

#endif
#endif
