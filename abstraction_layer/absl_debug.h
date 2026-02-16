/*
 * absl_debug.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_DEBUG_H_
#define ABSL_DEBUG_H_

#include "absl_config.h"
#ifdef ABSL_DEBUG
#include "absl_types.h"

#ifdef ABSL_OS_FREE_RTOS
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#endif

#define DEBUG_SHELL_COMMAND_RECIEVED 	0x00000001

#ifdef ABSL_OS_FREE_RTOS
shell_status_t absl_debug_shell_handler(shell_handle_t shellHandle, int32_t argc, char **argv);

#define absl_debug_shell_init					absl_debug_shell_init_freertos
#define absl_debug_shell_add_cmd				absl_debug_shell_add_cmd_freertos
#define absl_debug_printf                     absl_debug_printf_freertos
#define absl_debug_shell_wait_commad			absl_debug_shell_wait_commad_freertos
#define absl_debug_print_prompt				absl_debug_print_prompt_freertos
#define absl_debug_enable_prompt				absl_debug_enable_prompt_freertos
#define absl_debug_disable_prompt				absl_debug_disable_prompt_freertos
#elif defined(ABSL_LINUX)
#define absl_debug_printf                     printf
#define absl_debug_printf_with_var_list       absl_debug_printf_with_var_list_linux
#else
#error Platform nos defined
#endif

void 	 absl_debug_shell_init(char* _prompt);
void 	 absl_debug_shell_add_cmd(char* _command, char* _descriptor);
int      absl_debug_printf(const char *fmt_s, ...);
uint32_t absl_debug_shell_wait_commad(char** _argv);

void absl_debug_print_prompt(void);
void absl_debug_enable_prompt(void);
void absl_debug_disable_prompt(void);

#endif /* ABSL_DEBUG */


#endif /* ABSL_DEBUG_H_ */
