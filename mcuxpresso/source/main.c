/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020,2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "lwipopts.h"

#include "pin_mux.h"
#include "board.h"

#include "fsl_silicon_id.h"
#include "fsl_iomuxc.h"

#include "system_handler.h"

#include "pl_system.h"
#include "pl_thread.h"
#include "pl_debug.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSTEM_THREAD_PRIO 			14
#define SYSTEM_THREAD_STACKSIZE 	1500

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static pl_thread_t system_init_thread;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
    CLOCK_InitEnetPll(&config);
}

/*!
 * @brief Main function
 */
int main(void)
{

#if defined(MIMXRT1060_EVKB)
	gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
#endif

    BOARD_ConfigMPU();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();
    SCB_DisableDCache();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);
#if defined(ADSN4)
    /* pull up the ENET_INT before RESET. */
	GPIO_WritePinOutput(GPIO2, 14, 0);	// PHY_RST
	SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
	GPIO_WritePinOutput(GPIO2, 14, 1);
#elif defined(MIMXRT1060_EVKB)
	GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO1, 9, 1);
#else
#error ERROR! Valid platform not defined
#endif

	mdio_init();

    pl_debug_printf("\r\n\n******** System Start ********\r\n");

    if (PL_THREAD_RV_OK != pl_thread_create(&system_init_thread, "System init", system_handler,
       		               SYSTEM_THREAD_PRIO, SYSTEM_THREAD_STACKSIZE, NULL))
	{
    	pl_hardfault_handler(0);
	}

	pl_thread_run();

	pl_hardfault_handler(0);

	return 0;
}

