/*
 * pl_watchdog.h
 *
 *  Created on: May 19, 2023
 *      Author: abolinaga
 */

#ifndef PL_WATCHDOG_H_
#define PL_WATCHDOG_H_

#include "pl_config.h"
#ifdef PL_WATCHDOG

#include "pl_types.h"
#if defined(PL_IMX_RT10XX)
#include "fsl_wdog.h"
#endif

typedef enum pl_watchdog_rv
{
	PL_WATCHDOG_OK = 0,
	PL_WATCHDOG_NO_CONFIG,
	PL_WATCHDOG_NO_INIT,
	PL_WATCHDOG_ERROR
}pl_watchdog_rv_t;

typedef struct pl_watchdog_config
{
#if defined(PL_IMX_RT10XX)
	WDOG_Type *		wdog;
#endif
	uint32_t		reset_time_s;
	uint32_t		reload_interrupt_time_s;
}pl_watchdog_config_t;

typedef struct pl_watchdog
{
	pl_watchdog_config_t* pl_wdog_config;
#if defined(PL_IMX_RT10XX)
	wdog_config_t 		  config;
#endif
}pl_watchdog_t;

#if defined(PL_IMX_RT10XX)
#define pl_watchdog_init				pl_watchdog_init_imxrt10xx
#define pl_watchdog_get_reset_reason	pl_watchdog_get_reset_reason_imxrt10xx
#define pl_watchdog_run					pl_watchdog_run_imxrt10xx
#define pl_watchdog_sw_reset			pl_watchdog_sw_reset_imxrt10xx
#else
#error Platform not defined
#endif

pl_watchdog_rv_t pl_watchdog_init(pl_watchdog_t* _wdog, pl_watchdog_config_t* _wdog_config);
void pl_watchdog_run(pl_watchdog_t* _wdog);
void pl_watchdog_sw_reset(pl_watchdog_t* _wdog);

void desrefrescar(void);

#endif
#endif /* PL_WATCHDOG_H_ */
