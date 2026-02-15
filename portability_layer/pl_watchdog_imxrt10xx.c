/*
 * pl_watchdog_imxrt10xx.c
 *
 *  Created on: May 19, 2023
 *      Author: abolinaga
 */

#include "pl_watchdog.h"

#ifdef PL_WATCHDOG
#ifdef PL_IMX_RT10XX

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define pl_watchdog1_IRQHandler WDOG1_IRQHandler
#define pl_watchdog2_IRQHandler WDOG2_IRQHandler

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bool watchdog_test __attribute__((section(".noinit_RAM3")));

/*******************************************************************************
 * Code
 ******************************************************************************/
void pl_watchdog1_IRQHandler(void)
{
	WDOG_ClearInterruptStatus(WDOG1, kWDOG_InterruptFlag);
	WDOG_Refresh(WDOG1);
}

void pl_watchdog2_IRQHandler(void)
{
	WDOG_ClearInterruptStatus(WDOG2, kWDOG_InterruptFlag);
	WDOG_Refresh(WDOG2);
}

pl_watchdog_rv_t pl_watchdog_init_imxrt10xx(pl_watchdog_t* _wdog, pl_watchdog_config_t* _wdog_config)
{
	pl_watchdog_rv_t wdog_rv = PL_WATCHDOG_ERROR;

	if(NULL != _wdog_config)
	{
		_wdog->pl_wdog_config = _wdog_config;
		/*
		 * wdogConfig->enableWdog = true;
		 * wdogConfig->workMode.enableWait = true;
		 * wdogConfig->workMode.enableStop = false;
		 * wdogConfig->workMode.enableDebug = false;
		 * wdogConfig->enableInterrupt = false;
		 * wdogConfig->enablePowerdown = false;
		 * wdogConfig->resetExtension = flase;
		 * wdogConfig->timeoutValue = 0xFFU;
		 * wdogConfig->interruptTimeValue = 0x04u;
		 */
		WDOG_GetDefaultConfig(&_wdog->config);
		_wdog->config.timeoutValue       = (_wdog_config->reset_time_s * 2) - 1; /* Timeout value is (0xF+1)/2 = 8 sec. */
		_wdog->config.enableInterrupt    = true;
		_wdog->config.interruptTimeValue = (_wdog_config->reset_time_s  -_wdog_config->reload_interrupt_time_s) * 2; /* Interrupt occurred (0x4)/2 = 2 sec before WDOG timeout. */
		_wdog->config.workMode.enableDebug = true;

		wdog_rv = PL_WATCHDOG_OK;
	}
	else
	{
		wdog_rv = PL_WATCHDOG_NO_CONFIG;
	}

	return wdog_rv;
}

void pl_watchdog_run_imxrt10xx(pl_watchdog_t* _wdog)
{
    watchdog_test = false;
	WDOG_Init(_wdog->pl_wdog_config->wdog, &_wdog->config);
}

void pl_watchdog_sw_reset_imxrt10xx(pl_watchdog_t* _wdog)
{
	WDOG_TriggerSystemSoftwareReset(_wdog->pl_wdog_config->wdog);
}

#endif
#endif

