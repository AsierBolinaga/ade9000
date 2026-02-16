/*
 * debug_task.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef DEBUG_SHELL_H_
#define DEBUG_SHELL_H_

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "absl_event.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
typedef enum debug_shell_events
{
	DS_EVENTS_INIT_NOT_DONE = 0,
	DS_EVENTS_THREAD_NO_CONFIG,
	DS_EVENTS_EVENTGROUP_NO_INITIALIZED,
	DS_EVENTS_MAXVALUE
}debug_shell_events_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef void (*arg_cb_t)(char** _arg);

typedef struct debug_shell_cmd
{
	char* 		command;
	char*	 	descriptor;
	arg_cb_t	arg_cb;
}debug_shell_cmd_t;

typedef struct debug_shell_config
{
	char *					prompt;
	bool					cmd_initialized;
}debug_shell_config_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool debug_shell_initialize(debug_shell_config_t* _debug_shell_config);

void debug_shell_add_commads(debug_shell_cmd_t* _debug_shell_commads, uint32_t _cmd_amount);

void debug_shell(void *arg);

void debug_shell_finish(void);

#endif /* DEBUG_SHELL_H_ */
