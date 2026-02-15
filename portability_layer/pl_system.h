/*
 * pl_system.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_SYSTEM_H_
#define PL_SYSTEM_H_

#include "pl_config.h"
#ifdef PL_SYSTEM

#include "pl_types.h"
#if defined(PL_IMX_RT10XX)
#define	HARDFAULT_ERROR				0xEA671DB6
#define NMI_HARDFAULT_ERROR			0xC1FC46F7
#define SVC_HARDFAULT_ERROR			0x6B54E2A3
#define DEBUGMON_HARDFAULT_ERROR	0x97507CAE
#define PEND_SV_HARDFAULT_ERROR		0xCF515D51
#define SYSTICK_HARDFAULT_ERROR		0x0F6EA600
#define INT_HANDLER_HARDFAULT_ERROR	0x6741022B

#define MEM_MAN_HARDFAULT_ERROR		0xF1EAD5C2
#define MEM_MAN_HASDFAULT_MLSPERR	0x4E9248AE
#define MEM_MAN_HARDFAULT_MSTKERR	0xB5B35A8C
#define MEM_MAN_HARDFAULT_MUNSTKERR	0x9A8B66AE
#define MEM_MAN_HARDFAULT_DACCVIOL	0xA02393E4
#define MEM_MAN_HARDFAULT_IACCVIOL	0x525F482E

#define BUS_HARDFAULT_ERROR			0x4CF57D1D
#define BUS_HARDFAULT_LSPERR		0xE524B62E
#define BUS_HARDFAULT_STKERR		0x637B648F
#define BUS_HARDFAULT_UNSTKERR		0xE342EC16
#define BUS_HARDFAULT_IMPRECISERR	0x4FE37C1F
#define BUS_HARDFAULT_PRECISERR		0x5C3BA50C
#define BUS_HARDFAULT_IBUSERR		0xB21E031C

#define USAGE_HARDFAULT_ERROR		0x7C662E78
#define USAGE_HARDFAULT_DIVBYZERO	0xA47B392A
#define USAGE_HARDFAULT_UNALIGNED	0x5331C428
#define USAGE_HARDFAULT_NOCP		0xBC33321C
#define USAGE_HARDFAULT_INVPC		0x17A77D11
#define USAGE_HARDFAULT_INVSTATE	0xEF47462F
#define USAGE_HARDFAULT_UNDEFINSTR	0x07EBE78E

#define STACK_OVERFLOW_FLAG			0xD33D74B0
#define	MALLOC_FAILED_FLAG			0xA6616A25
#define	ASSERTION_ERROR				0x2805F76A

#define	THREAD_CREATE_ERROR			0xB7D070CA
#define	THREAD_INIT_ERROR			0x24AF9CC5
#define	THREAD_NOT_INIT_ERROR		0x0AB850FB
#define WDOG_INIT_ERROR				0xF90CE20A
#define WDOG_NOT_INIT_ERROR			0x79B1F93A
#define MANU_NOT_INIT_ERROR			0xCBFAB3FD
#define DEBUG_SHELL_NOT_INIT_ERROR	0x57A7C153
#define UNKNOWN_SWITCH_CASE_ERROR	0xE2175BEE
#define QUEUE_CREATE_ERROR			0x051FA3E1
#define EVENT_CREATE_ERROR			0x56649D4D
#define UNKNOWN_EVENT_ERROR			0x646EDF5D
#endif

typedef enum pl_system_reset
{
	PL_SYSTEM_POWERUP_RESET = 0,
	PL_SYSTEM_SECURITY_RESET,
	PL_SYSTEM_SOFTWARE_RESET,
	PL_SYSTEM_WDOG_TIMEOUT_RESET,
	PL_SYSTEM_JTAG_HW_RESET,
	PL_SYSTEM_JTAG_SW_RESET,
	PL_SYSTEM_TEMPSENSE_RESET,
	PL_SYSTEM_NOT_DETECTED_RESET
}pl_system_reset_t;


#if defined(PL_IMX_RT10XX)
#define pl_system_get_reset_reason				pl_system_get_reset_reason_imxrt10xx
#define pl_system_init 							pl_system_init_imxrt10xx
#define pl_system_reboot						pl_system_reboot_imxrt10xx
#define pl_system_supervisor_get_CPU_load		pl_system_supervisor_get_CPU_load_imxrt10xx
#define pl_system_supervisor_get_heap_usage		pl_system_supervisor_get_heap_usage_imxrt10xx
#define pl_system_set_fwu_flag					pl_system_set_fwu_flag_imxrt10xx
#define pl_system_get_fwu_flag					pl_system_get_fwu_flag_imxrt10xx
#define pl_system_get_fwu_clear_flag			pl_system_get_fwu_clear_flag_imxrt10xx
#define pl_system_set_hf_error_flag				pl_system_set_hf_error_flag_imxrt10xx
#define	pl_system_get_hf_error_flag				pl_system_get_hf_error_flag_imxrt10xx
#define pl_system_get_hf_info_flag				pl_system_get_hf_info_flag_imxrt10xx
#define	pl_hardfault_handler					pl_hardfault_handler_imxrt10xx
#define pl_hardfault_mem_man					pl_hardfault_mem_man_imxrt10xx
#define pl_hardfault_bus						pl_hardfault_bus_imxrt10xx
#define pl_hardfault_usage						pl_hardfault_usage_imxrt10xx
#else
#error Platform not defined
#endif

void pl_system_init(void);

void pl_system_reboot(void);

float pl_system_supervisor_get_CPU_load(void);
float pl_system_supervisor_get_heap_usage(void);
pl_system_reset_t pl_system_get_reset_reason(void);
void pl_system_set_fwu_flag(uint32_t _flag);
uint32_t pl_system_get_fwu_flag(void);
void pl_system_get_fwu_clear_flag(void);
void pl_system_set_hf_error_flag(uint32_t _flag);
uint32_t pl_system_get_hf_error_flag(void);
uint32_t pl_system_get_hf_info_flag(void);

void pl_hardfault_handler(uint32_t _error_flag);

void pl_hardfault_mem_man(void);
void pl_hardfault_bus(void);
void pl_hardfault_usage(void);

#endif
#endif /* PL_SYSTEM_H_ */
