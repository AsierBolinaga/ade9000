
#include "absl_debug.h"
#ifdef ABSL_DEBUG

#include "absl_event.h"
#include "absl_timer.h"
#include "absl_macros.h"

#ifdef ABSL_OS_FREE_RTOS
#include "fsl_debug_console_conf.h"
#include "fsl_str.h"

#define MAX_ABSL_DEBUG_SHELL_CMD_AMOUNT	50

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);

static shell_handle_t s_shellHandle;

static shell_command_t shellCommand = {NULL, NULL, absl_debug_shell_handler, SHELL_IGNORE_PARAMETER_COUNT, {NULL}};
static shell_command_t shellCommands[MAX_ABSL_DEBUG_SHELL_CMD_AMOUNT];
static uint32_t	used_cmd_amount = 0;

static absl_event_t debug_shell_event;

static int32_t last_arg_count = 0;
static char* arguments[10];

static bool		  prompt_enabled = false;
static bool		  prompt_written = false;
static absl_timer_t prompt_timer;

static void absl_debug_print_prompt_timeout(void* arg)
{
	ABSL_UNUSED_ARG(arg);

	if(!prompt_written)
	{
		SHELL_PrintPrompt(s_shellHandle);
		prompt_written = true;
	}
}

shell_status_t absl_debug_shell_handler(shell_handle_t shellHandle, int32_t argc, char **argv)
{
	ABSL_UNUSED_ARG(shellHandle);

	last_arg_count = argc;

	for(uint32_t arg_index = 0; arg_index < 5; arg_index++)
	{
		arguments[arg_index] = argv[arg_index];
	}

	absl_event_set_fromISR(&debug_shell_event, DEBUG_SHELL_COMMAND_RECIEVED);
	prompt_written = false;

	return kStatus_SHELL_Success;
}

void absl_debug_shell_init(char* _prompt)
{
	absl_time_t prompt_print_timeout = {1, 0};
	s_shellHandle = &s_shellHandleBuffer[0];
	SHELL_Init(s_shellHandle, g_serialHandle, _prompt);

	absl_event_create(&debug_shell_event);
	absl_timer_create(&prompt_timer, absl_debug_print_prompt_timeout, NULL, prompt_print_timeout, false, false);
}


void absl_debug_shell_add_cmd_freertos(char* _command, char* _descriptor)
{
	memcpy(&shellCommands[used_cmd_amount], &shellCommand, sizeof(shell_command_t));
	shellCommands[used_cmd_amount].pcCommand = _command;
	shellCommands[used_cmd_amount].pcHelpString = _descriptor;
	SHELL_RegisterCommand(s_shellHandle, &shellCommands[used_cmd_amount]);
	used_cmd_amount++;
}

int absl_debug_printf_freertos(const char *fmt_s, ...)
{
	va_list ap;
	int result = 0;

	if(prompt_enabled && !prompt_written)
	{
		absl_timer_stop(&prompt_timer);
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
		absl_timer_start(&prompt_timer);
	}
	return result;
}

uint32_t absl_debug_shell_wait_commad_freertos(char** _argv)
{
	uint32_t event;

	absl_event_wait(&debug_shell_event, DEBUG_SHELL_COMMAND_RECIEVED, &event);
	memcpy(_argv, arguments, sizeof(arguments));

	return last_arg_count;
}

void absl_debug_print_prompt_freertos(void)
{
	SHELL_PrintPrompt(s_shellHandle);
	prompt_enabled = true;
	prompt_written = true;
}

void absl_debug_enable_prompt_freertos(void)
{
	prompt_enabled = true;
}

void absl_debug_disable_prompt_freertos(void)
{
	prompt_enabled = false;
}

#endif /* ABSL_OS_FREE_RTOS */
#endif /* ABSL_DEBUG */
