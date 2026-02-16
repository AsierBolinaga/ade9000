/*
 * absl_watchdog.h
 *
 *  Created on: May 19, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_WATCHDOG_H_
#define ABSL_WATCHDOG_H_

#include "absl_config.h"
#ifdef ABSL_WATCHDOG

#include "absl_types.h"
#if defined(ABSL_IMX_RT10XX)
#include "fsl_wdog.h"
#endif

typedef enum absl_watchdog_rv
{
	ABSL_WATCHDOG_OK = 0,
	ABSL_WATCHDOG_NO_CONFIG,
	ABSL_WATCHDOG_NO_INIT,
	ABSL_WATCHDOG_ERROR
}absl_watchdog_rv_t;

typedef struct absl_watchdog_config
{
#if defined(ABSL_IMX_RT10XX)
	WDOG_Type *		wdog;
#endif
	uint32_t		reset_time_s;
	uint32_t		reload_interrupt_time_s;
}absl_watchdog_config_t;

typedef struct absl_watchdog
{
	absl_watchdog_config_t* absl_wdog_config;
#if defined(ABSL_IMX_RT10XX)
	wdog_config_t 		  config;
#endif
}absl_watchdog_t;

#if defined(ABSL_IMX_RT10XX)
#define absl_watchdog_init				absl_watchdog_init_imxrt10xx
#define absl_watchdog_get_reset_reason	absl_watchdog_get_reset_reason_imxrt10xx
#define absl_watchdog_run					absl_watchdog_run_imxrt10xx
#define absl_watchdog_sw_reset			absl_watchdog_sw_reset_imxrt10xx
#else
#error Platform not defined
#endif

absl_watchdog_rv_t absl_watchdog_init(absl_watchdog_t* _wdog, absl_watchdog_config_t* _wdog_config);
void absl_watchdog_run(absl_watchdog_t* _wdog);
void absl_watchdog_sw_reset(absl_watchdog_t* _wdog);

void desrefrescar(void);

#endif
#endif /* ABSL_WATCHDOG_H_ */
