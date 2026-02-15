
#include "pl_debug.h"
#ifdef PL_DEBUG

#include "pl_event.h"
#include "pl_timer.h"
#include "pl_macros.h"

#ifdef PL_OS_FREE_RTOS
#include "fsl_debug_console_conf.h"
#include "fsl_str.h"

#define MAX_PL_DEBUG_SHELL_CMD_AMOUNT	50

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);

static shell_handle_t s_shellHandle;

static shell_command_t shellCommand = {NULL, NULL, pl_debug_shell_handler, SHELL_IGNORE_PARAMETER_COUNT, {NULL}};
static shell_command_t shellCommands[MAX_PL_DEBUG_SHELL_CMD_AMOUNT];
static uint32_t	used_cmd_amount = 0;

static pl_event_t debug_shell_event;

static int32_t last_arg_count = 0;
static char* arguments[10];

static bool		  prompt_enabled = false;
static bool		  prompt_written = false;
static pl_timer_t prompt_timer;

static void pl_debug_print_prompt_timeout(void* arg)
{
	PL_UNUSED_ARG(arg);

	if(!prompt_written)
	{
		SHELL_PrintPrompt(s_shellHandle);
		prompt_written = true;
	}
}

shell_status_t pl_debug_shell_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	PL_UNUSED_ARG(shellHandle);

	last_arg_count = argc;

	for(uint32_t arg_index = 0; arg_index < 5; arg_index++)
	{
		arguments[arg_index] = argv[arg_index];
	}

	pl_event_set_fromISR(&debug_shell_event, DEBUG_SHELL_COMMAND_RECIEVED);
	prompt_written = false;

	return kStatus_SHELL_Success;
}

void pl_debug_shell_init(char* _prompt)
{
	pl_time_t prompt_print_timeout = {1, 0};
	s_shellHandle = &s_shellHandleBuffer[0];
	SHELL_Init(s_shellHandle, g_serialHandle, _prompt);

	pl_event_create(&debug_shell_event);
	pl_timer_create(&prompt_timer, pl_debug_print_prompt_timeout, NULL, prompt_print_timeout, false, false);
}


void pl_debug_shell_add_cmd_freertos(char* _command, char* _descriptor)
{
	memcpy(&shellCommands[used_cmd_amount], &shellCommand, sizeof(shell_command_t));
	shellCommands[used_cmd_amount].pcCommand = _command;
	shellCommands[used_cmd_amount].pcHelpString = _descriptor;
	SHELL_RegisterCommand(s_shellHandle, &shellCommands[used_cmd_amount]);
	used_cmd_amount++;
}

int pl_debug_printf_freertos(const char *fmt_s, ...)
{
	va_list ap;
	int result = 0;

	if(prompt_enabled && !prompt_written)
	{
		pl_timer_stop(&prompt_timer);
	}

	if(prompt_written)
	{
		PRINTF("\n\n");
		prompt_written = false;
	}

	if (NULL != g_serialHandle)
	{
		va_start(ap, fmt_s);
		result = DbgConsole_Vprintf(fmt_s, ap);
		va_end(ap);
	}

	if(prompt_enabled)
	{
		pl_timer_start(&prompt_timer);
	}
	return result;
}

uint32_t pl_debug_shell_wait_commad_freertos(char** _argv)
{
	uint32_t event;

	pl_event_wait(&debug_shell_event, DEBUG_SHELL_COMMAND_RECIEVED, &event);
	memcpy(_argv, arguments, sizeof(arguments));

	return last_arg_count;
}

void pl_debug_print_prompt_freertos(void)
{
	SHELL_PrintPrompt(s_shellHandle);
	prompt_enabled = true;
	prompt_written = true;
}

void pl_debug_enable_prompt_freertos(void)
{
	prompt_enabled = true;
}

void pl_debug_disable_prompt_freertos(void)
{
	prompt_enabled = false;
}

#endif /* PL_OS_FREE_RTOS */
#endif /* PL_DEBUG */
