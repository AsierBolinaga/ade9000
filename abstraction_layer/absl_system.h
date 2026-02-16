/*
 * absl_system.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_SYSTEM_H_
#define ABSL_SYSTEM_H_

#include "absl_config.h"
#ifdef ABSL_SYSTEM

#include "absl_types.h"
#if defined(ABSL_IMX_RT10XX)
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
#define ERROR_SENDING_MQTT			0x7A9D3C42
#define ERROR_FLASH_ERASE_FAILED	0xF134B8AE
#endif

typedef enum absl_system_reset
{
	ABSL_SYSTEM_POWERUP_RESET = 0,
	ABSL_SYSTEM_SECURITY_RESET,
	ABSL_SYSTEM_SOFTWARE_RESET,
	ABSL_SYSTEM_WDOG_TIMEOUT_RESET,
	ABSL_SYSTEM_JTAG_HW_RESET,
	ABSL_SYSTEM_JTAG_SW_RESET,
	ABSL_SYSTEM_TEMPSENSE_RESET,
	ABSL_SYSTEM_NOT_DETECTED_RESET
}absl_system_reset_t;


#if defined(ABSL_IMX_RT10XX)
#define absl_system_get_reset_reason				absl_system_get_reset_reason_imxrt10xx
#define absl_system_init 							absl_system_init_imxrt10xx
#define absl_system_reboot						absl_system_reboot_imxrt10xx
#define absl_system_supervisor_get_CPU_load		absl_system_supervisor_get_CPU_load_imxrt10xx
#define absl_system_supervisor_get_heap_usage		absl_system_supervisor_get_heap_usage_imxrt10xx
#define absl_system_set_fwu_flag					absl_system_set_fwu_flag_imxrt10xx
#define absl_system_get_fwu_flag					absl_system_get_fwu_flag_imxrt10xx
#define absl_system_get_fwu_clear_flag			absl_system_get_fwu_clear_flag_imxrt10xx
#define absl_system_set_hf_error_flag				absl_system_set_hf_error_flag_imxrt10xx
#define	absl_system_get_hf_error_flag				absl_system_get_hf_error_flag_imxrt10xx
#define absl_system_get_hf_info_flag				absl_system_get_hf_info_flag_imxrt10xx
#define	absl_hardfault_handler					absl_hardfault_handler_imxrt10xx
#define absl_hardfault_mem_man					absl_hardfault_mem_man_imxrt10xx
#define absl_hardfault_bus						absl_hardfault_bus_imxrt10xx
#define absl_hardfault_usage						absl_hardfault_usage_imxrt10xx
#else
#error Platform not defined
#endif

void absl_system_init(void);

void absl_system_reboot(void);

float absl_system_supervisor_get_CPU_load(void);
float absl_system_supervisor_get_heap_usage(void);
absl_system_reset_t absl_system_get_reset_reason(void);
void absl_system_set_fwu_flag(uint32_t _flag);
uint32_t absl_system_get_fwu_flag(void);
void absl_system_get_fwu_clear_flag(void);
void absl_system_set_hf_error_flag(uint32_t _flag);
uint32_t absl_system_get_hf_error_flag(void);
uint32_t absl_system_get_hf_info_flag(void);

void absl_hardfault_handler(uint32_t _error_flag);

void absl_hardfault_mem_man(void);
void absl_hardfault_bus(void);
void absl_hardfault_usage(void);

#endif
#endif /* ABSL_SYSTEM_H_ */
