/*
 * debug_task.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#include "debug_shell.h"

#include "absl_system.h"
#include "absl_debug.h"
#include "absl_thread.h"
#include "absl_macros.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEBUG_SHELL_MAX_CMD_AMOUNT 		100

/*******************************************************************************
 * type definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static debug_shell_config_t* debug_shell_config;

static uint32_t				registered_cmd_amount = 0;
static debug_shell_cmd_t* 	debug_shell_cmds[DEBUG_SHELL_MAX_CMD_AMOUNT];

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

bool debug_shell_initialize(debug_shell_config_t* _debug_shell_config)
{
	bool return_value = false;

	if(NULL != _debug_shell_config)
	{
		debug_shell_config = _debug_shell_config;
		absl_debug_shell_init(debug_shell_config->prompt);

		debug_shell_config->cmd_initialized = true;
		return_value = true;
	}
	else
	{

	}

	return return_value;
}

void debug_shell_add_commads(debug_shell_cmd_t* _debug_shell_commads, uint32_t _cmd_amount)
{
	/* Add new command to commands list */
	for(uint8_t cmd_index = 0; cmd_index < _cmd_amount; cmd_index++)
	{
		debug_shell_cmds[registered_cmd_amount + cmd_index] = &_debug_shell_commads[cmd_index];
		absl_debug_shell_add_cmd(_debug_shell_commads[cmd_index].command, _debug_shell_commads[cmd_index].descriptor);
	}
	registered_cmd_amount += _cmd_amount;
}

void debug_shell(void* arg)
{
	char** 	argv = 0;

	ABSL_UNUSED_ARG(arg);

	if(!debug_shell_config->cmd_initialized)
	{
		absl_debug_printf("Debug shell was not initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	absl_debug_enable_prompt();
	absl_debug_print_prompt();

	while(1)
	{
		while(1)
		{
			absl_debug_shell_wait_commad(argv);

			for(uint32_t arg_index = 0; arg_index < registered_cmd_amount; arg_index++)
			{
				if(!strcmp(argv[0], debug_shell_cmds[arg_index]->command))
				{
					debug_shell_cmds[arg_index]->arg_cb(argv);
				}
			}
		}
	}
}

void debug_shell_finish(void)
{
	absl_debug_disable_prompt();
}
