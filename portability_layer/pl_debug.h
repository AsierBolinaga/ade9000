/*
 * pl_debug.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef PL_DEBUG_H_
#define PL_DEBUG_H_

#include "pl_config.h"
#ifdef PL_DEBUG
#include "pl_types.h"

#ifdef PL_OS_FREE_RTOS
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#endif

#define DEBUG_SHELL_COMMAND_RECIEVED 	0x00000001

#ifdef PL_OS_FREE_RTOS
shell_status_t pl_debug_shell_handler(shell_handle_t shellHandle, int32_t argc, char **argv);

#define pl_debug_shell_init					pl_debug_shell_init_freertos
#define pl_debug_shell_add_cmd				pl_debug_shell_add_cmd_freertos
#define pl_debug_printf                     pl_debug_printf_freertos
#define pl_debug_shell_wait_commad			pl_debug_shell_wait_commad_freertos
#define pl_debug_print_prompt				pl_debug_print_prompt_freertos
#define pl_debug_enable_prompt				pl_debug_enable_prompt_freertos
#define pl_debug_disable_prompt				pl_debug_disable_prompt_freertos
#elif defined(PL_LINUX)
#define pl_debug_printf                     printf
#define pl_debug_printf_with_var_list       pl_debug_printf_with_var_list_linux
#else
#error Platform nos defined
#endif

void 	 pl_debug_shell_init(char* _prompt);
void 	 pl_debug_shell_add_cmd(char* _command, char* _descriptor);
int      pl_debug_printf(const char *fmt_s, ...);
uint32_t pl_debug_shell_wait_commad(char** _argv);

void pl_debug_print_prompt(void);
void pl_debug_enable_prompt(void);
void pl_debug_disable_prompt(void);

#endif /* PL_DEBUG */


#endif /* PL_DEBUG_H_ */
