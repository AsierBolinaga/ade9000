
#include "pl_system.h"

#ifdef PL_SYSTEM

#if defined(PL_IMX_RT10XX) && defined(PL_OS_FREE_RTOS)
#include "MIMXRT1062.h"

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_gpt.h"
#include "fsl_snvs_lp.h"

#include "pl_thread.h"
#include "pl_debug.h"
#include "pl_macros.h"

/* configurable fault register masks */
#define UFSR		GENMASK(31,16)
#define BFSR		GENMASK(15, 8)
#define MMFSR		GENMASK( 7, 0)

/* User fault bits */
#define DIVBYZERO 	BIT(25)
#define UNALIGNED	BIT(24)
#define NOCP 		BIT(19)
#define INVPC		BIT(18)
#define INVSTATE 	BIT(17)
#define UNDEFINSTR 	BIT(16)

/* Bus fault bits */
#define BFARVALID 	BIT(15)
#define LSPERR	 	BIT(13)
#define STKERR	 	BIT(12)
#define UNSTKERR 	BIT(11)
#define IMPRECISERR	BIT(10)
#define PRECISERR 	BIT(9)
#define IBUSERR 	BIT(8)

/* Memory management fault bits */
#define MMARVALID 	BIT(7)
#define MLSPERR 	BIT(5)
#define MSTKERR 	BIT(4)
#define MUNSTKERR 	BIT(3)
#define DACCVIOL 	BIT(1)
#define IACCVIOL 	BIT(0)

typedef enum lp_flag_reg_positions
{
	LP_FLAGS_FWU_POS = 0,
	LP_FLAGS_HF_ERRORS_POS,
	LP_FLAGS_HF_INFO,
	LP_FLAGS_MAXVALUE
}lp_flag_reg_positions_t;

volatile uint32_t g_systickCounter;

static uint32_t	reboot_countdown_value = 100;

uint32_t McuRTOS_RunTimeCounter = 0; /* runtime counter, used for configGENERATE_RUNTIME_STATS */

void GPT2_IRQHandler(void) {
  /* Clear interrupt flag.*/
  GPT_ClearStatusFlags(GPT2, kGPT_OutputCompare1Flag);
  McuRTOS_RunTimeCounter++; /* increment runtime counter */
#if defined __CORTEX_M && (__CORTEX_M == 4U || __CORTEX_M == 7U)
  __DSB();
#endif
}

void pl_system_init_imxrt10xx(void)
{
    snvs_lp_srtc_config_t snvsSrtcConfig;

    SNVS_LP_SRTC_GetDefaultConfig(&snvsSrtcConfig);
    SNVS_LP_SRTC_Init(SNVS, &snvsSrtcConfig);

    /* activate default deactivated fault handlers */
    SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk | SCB_SHCSR_USGFAULTENA_Msk);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
	PL_UNUSED_ARG(xTask);

	pl_debug_printf("ERROR!! Stack overflow of thread %s!\n", pcTaskName);
	pl_hardfault_handler(STACK_OVERFLOW_FLAG);
}

void vApplicationMallocFailedHook(void)
{
	pl_debug_printf("ERROR!! malloc failed! Heap full or memory corruption\n");
	pl_hardfault_handler(MALLOC_FAILED_FLAG);
}

void pl_system_reboot_imxrt10xx(void)
{
	pl_debug_printf("system is rebooting...\r\n\r\n");
	pl_thread_sleep(500);
	NVIC_SystemReset();
}

void pl_system_configure_timer_for_runtime_stats_freertos( void )
{
	uint32_t gptFreq;
	gpt_config_t gptConfig;

	GPT_GetDefaultConfig(&gptConfig);

	/* Initialize GPT module */
	GPT_Init(GPT2, &gptConfig);

	/* Divide GPT clock source frequency by 3 inside GPT module */
	GPT_SetClockDivider(GPT2, 3);

	/* Get GPT clock frequency */
	gptFreq = CLOCK_GetFreq(kCLOCK_PerClk);

	/* GPT frequency is divided by 3 inside module */
	gptFreq /= 3;

	/* Set GPT module to 10x of the FreeRTOS tick counter */
	gptFreq = USEC_TO_COUNT(100, gptFreq); /* FreeRTOS tick is 1 kHz */
	GPT_SetOutputCompareValue(GPT2, kGPT_OutputCompare_Channel1, gptFreq);

	/* Enable GPT Output Compare1 interrupt */
	GPT_EnableInterrupts(GPT2, kGPT_OutputCompare1InterruptEnable);

	/* Enable at the Interrupt and start timer */
	EnableIRQ(GPT2_IRQn);
	GPT_StartTimer(GPT2);
}

unsigned long pl_get_runtime_counter_value_freertos( void )
{
	return McuRTOS_RunTimeCounter;
}

float pl_system_supervisor_get_CPU_load_imxrt10xx(void)
{
	return 100.0 - ulTaskGetIdleRunTimePercent();
}

float pl_system_supervisor_get_heap_usage_imxrt10xx(void)
{
	uint32_t heap_free_size = xPortGetFreeHeapSize();
	uint32_t used_size = configTOTAL_HEAP_SIZE - heap_free_size;
	return (used_size * 100.0) / configTOTAL_HEAP_SIZE;
}

pl_system_reset_t pl_system_get_reset_reason_imxrt10xx(void)
{
	pl_system_reset_t reset_reason;
	uint32_t resetStatus = SRC->SRSR;

	reset_reason = PL_SYSTEM_NOT_DETECTED_RESET;

	if((resetStatus & SRC_SRSR_WDOG_RST_B_MASK) || (resetStatus & SRC_SRSR_WDOG3_RST_B_MASK))
	{
		reset_reason = PL_SYSTEM_WDOG_TIMEOUT_RESET;
		SRC->SRSR |= (SRC_SRSR_WDOG3_RST_B_MASK & SRC_SRSR_WDOG3_RST_B_MASK);  // Clear flags
	}
	else if (resetStatus & SRC_SRSR_CSU_RESET_B_MASK)
	{
		reset_reason = PL_SYSTEM_SECURITY_RESET;
		SRC->SRSR |= SRC_SRSR_CSU_RESET_B_MASK;  // Clear flag
	}
	else if (resetStatus & SRC_SRSR_JTAG_RST_B_MASK)
	{
		reset_reason = PL_SYSTEM_JTAG_HW_RESET;
		SRC->SRSR |= SRC_SRSR_JTAG_RST_B_MASK;  // Clear flag
	}
	else if (resetStatus & SRC_SRSR_JTAG_SW_RST_MASK)
	{
		reset_reason = PL_SYSTEM_JTAG_SW_RESET;
		SRC->SRSR |= SRC_SRSR_JTAG_SW_RST_MASK;  // Clear flag
	}
	else if (resetStatus & SRC_SRSR_TEMPSENSE_RST_B_MASK)
	{
		reset_reason = PL_SYSTEM_TEMPSENSE_RESET;
		SRC->SRSR |= SRC_SRSR_TEMPSENSE_RST_B_MASK;  // Clear flag
	}
	else if ((resetStatus & SRC_SRSR_IPP_RESET_B_MASK) || (resetStatus & SRC_SRSR_IPP_USER_RESET_B_MASK))
	{
		reset_reason = PL_SYSTEM_POWERUP_RESET;
		SRC->SRSR |= (SRC_SRSR_IPP_RESET_B_MASK & SRC_SRSR_IPP_USER_RESET_B_MASK);  // Clear flags
	}
	else if (resetStatus & SRC_SRSR_LOCKUP_SYSRESETREQ_MASK)
	{
		reset_reason = PL_SYSTEM_SOFTWARE_RESET;
		SRC->SRSR |= SRC_SRSR_LOCKUP_SYSRESETREQ_MASK;  // Clear flag
	}

	return reset_reason;
}

void pl_system_set_fwu_flag_imxrt10xx(uint32_t _flag)
{
	SNVS->LPGPR[LP_FLAGS_FWU_POS] = _flag;
}

uint32_t pl_system_get_fwu_flag_imxrt10xx(void)
{
	return SNVS->LPGPR[LP_FLAGS_FWU_POS];
}

void pl_system_get_fwu_clear_flag_imxrt10xx(void)
{
	SNVS->LPGPR[LP_FLAGS_FWU_POS] = 0;
}

void pl_system_set_hf_error_flag_imxrt10xx(uint32_t _flag)
{
	SNVS->LPGPR[LP_FLAGS_HF_ERRORS_POS] = _flag;
}

uint32_t pl_system_get_hf_error_flag_imxrt10xx(void)
{
	uint32_t error_flag = SNVS->LPGPR[LP_FLAGS_HF_ERRORS_POS];

	SNVS->LPGPR[LP_FLAGS_HF_ERRORS_POS] = 0;

	return error_flag;
}

uint32_t pl_system_get_hf_info_flag_imxrt10xx(void)
{
	uint32_t info_data = SNVS->LPGPR[LP_FLAGS_HF_INFO];

	SNVS->LPGPR[LP_FLAGS_HF_INFO] = 0;

	return info_data;
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    	g_systickCounter--;
    }
}

void pl_hardfault_handler_imxrt10xx(uint32_t _error_flag)
{
    PRINTF("HARDFAULT!\n");

    pl_system_set_hf_error_flag_imxrt10xx(_error_flag);

#if defined(ADSN4)
    ((GPIO_Type*)0x401b8000)->DR_SET = (1UL << 20);
    while(1)
    {
    	SysTick_DelayTicks(500000U);
        ((GPIO_Type*)0x401b8000)->DR_TOGGLE =  (1u << 21);
        reboot_countdown_value--;

        if(0 == reboot_countdown_value)
        {
        	NVIC_SystemReset();
        }
    }
#elif defined(MIMXRT1060_EVKB)
    while(1)
    {
    	SysTick_DelayTicks(500000U);
        ((GPIO_Type*)0x401b8000)->DR_TOGGLE =  (1u << 8);
    }
#else
#error ERROR! Valid platform not defined
#endif
}

void pl_hardfault_mem_man_imxrt10xx(void)
{
	uint32_t mem_man_fault_reg = SCB->CFSR & MMFSR;
	/* in case specific memory management fault is not detected,
	 * it will indicate generic mem man fault */
	uint32_t mem_man_fault = MEM_MAN_HARDFAULT_ERROR;

	/* reset configurable fault flags */
	SCB->CFSR = SCB->CFSR;

	if(mem_man_fault_reg && BFARVALID) /* if is set the value of the address where the fault occurred is stored in BFAR */
	{
		SNVS->LPGPR[LP_FLAGS_HF_INFO] = SCB->BFAR;
		pl_debug_printf("Bus fault in address: %d\n", SCB->BFAR);
	}
	else
	{
		SNVS->LPGPR[LP_FLAGS_HF_INFO] = 0;
	}

	if(mem_man_fault_reg && MLSPERR)
	{
		mem_man_fault = MEM_MAN_HASDFAULT_MLSPERR;
	}
	else if(mem_man_fault_reg && MSTKERR)
	{
		mem_man_fault = MEM_MAN_HARDFAULT_MSTKERR;
	}
	else if(mem_man_fault_reg && MUNSTKERR)
	{
		mem_man_fault = MEM_MAN_HARDFAULT_MUNSTKERR;
	}
	else if(mem_man_fault_reg && DACCVIOL)
	{
		mem_man_fault = MEM_MAN_HARDFAULT_DACCVIOL;
	}
	else if(mem_man_fault_reg && IACCVIOL)
	{
		mem_man_fault = MEM_MAN_HARDFAULT_IACCVIOL;
	}

	pl_hardfault_handler(mem_man_fault);
}

void pl_hardfault_bus_imxrt10xx(void)
{
	uint32_t bus_fault_reg = SCB->CFSR & BFSR;
	/* in case specific bus fault is not detected,
	 * it will indicate generic bus fault */
	uint32_t bus_fault = BUS_HARDFAULT_ERROR;

	/* reset configurable fault flags */
	SCB->CFSR = SCB->CFSR;

	if(bus_fault_reg && MMARVALID) /* if is set the value of the address where the fault occurred is stored in MMFAR */
	{
		SNVS->LPGPR[LP_FLAGS_HF_INFO] = SCB->MMFAR;
		pl_debug_printf("Mem management fault in address: %d\n", SCB->MMFAR);
	}
	else
	{
		SNVS->LPGPR[LP_FLAGS_HF_INFO] = 0;
	}

	if(bus_fault_reg && LSPERR)
	{
		bus_fault = BUS_HARDFAULT_LSPERR;
	}
	else if(bus_fault_reg && STKERR)
	{
		bus_fault = BUS_HARDFAULT_STKERR;
	}
	else if(bus_fault_reg && UNSTKERR)
	{
		bus_fault = BUS_HARDFAULT_UNSTKERR;
	}
	else if(bus_fault_reg && IMPRECISERR)
	{
		bus_fault = BUS_HARDFAULT_IMPRECISERR;
	}
	else if(bus_fault_reg && PRECISERR)
	{
		bus_fault = BUS_HARDFAULT_PRECISERR;
	}
	else if(bus_fault_reg && IBUSERR)
	{
		bus_fault = BUS_HARDFAULT_IBUSERR;
	}

	pl_hardfault_handler(bus_fault);
}

void pl_hardfault_usage_imxrt10xx(void)
{
	uint32_t usage_fault_reg = SCB->CFSR & UFSR;
	/* in case specific usage fault is not detected,
	 * it will indicate generic usage fault */
	uint32_t usage_fault = USAGE_HARDFAULT_ERROR;

	/* reset configurable fault flags */
	SCB->CFSR = SCB->CFSR;

	if(usage_fault_reg && DIVBYZERO)
	{
		usage_fault = USAGE_HARDFAULT_DIVBYZERO;
	}
	else if(usage_fault_reg && UNALIGNED)
	{
		usage_fault = USAGE_HARDFAULT_UNALIGNED;
	}
	else if(usage_fault_reg && NOCP)
	{
		usage_fault = USAGE_HARDFAULT_NOCP;
	}
	else if(usage_fault_reg && INVPC)
	{
		usage_fault = USAGE_HARDFAULT_INVPC;
	}
	else if(usage_fault_reg && INVSTATE)
	{
		usage_fault = USAGE_HARDFAULT_INVSTATE;
	}
	else if(usage_fault_reg && UNDEFINSTR)
	{
		usage_fault = USAGE_HARDFAULT_UNDEFINSTR;
	}


	pl_hardfault_handler(usage_fault);
}

#endif
#endif
