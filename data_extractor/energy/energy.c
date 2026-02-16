/*
 * energy.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#include "energy.h"

#include "interfaces.h"

#include "absl_debug.h"
#include "absl_time.h"
#include "absl_timer.h"
#include "absl_thread.h"
#include "absl_system.h"
#include "absl_macros.h"

/*******************************************************************************
 * type definitions
 ******************************************************************************/
#define STREAM_DATA_BUFFFER_SIZE			10
#define ENERGY_READ_EVENT_MAX_TIMEOUTS		5

#define INC_INDEX(a) (((a + 1) == STREAM_DATA_BUFFFER_SIZE) ? 0 : (a + 1))

#define ENERGY_ERROR_CODE_STARTUP_ERROR			0x02

static bool obtain_fast_vars;
static bool obtain_slow_vars;

typedef enum energy_task_normal_states
{
	ENERGY_TASK_NORMAL_STATE_CONFIGURE = 0,
	ENERGY_TASK_NORMAL_STATE_RUNNING
}energy_task_normal_states_t;


/******************************************************************************
 * Variables
 ******************************************************************************/
static energy_thread_config_t* energy_config;

static energy_task_states_t			energy_state;
static energy_task_normal_states_t	energy_normal_state;

static absl_timer_t	read_vars_event_timer;
static absl_time_t	read_vars_timeout;
static uint32_t		event_timeout_error_count;

static uint32_t	consistency_counter;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
static void energy_read_vars_event_timeout_cb(void* arg);

static void energy_task_idle_state(uint32_t _events);
static void energy_task_normal_state(uint32_t _events);
static void energy_task_configure_state(uint32_t _events);
static void energy_task_running_state(uint32_t _events);

static void 	energy_initialize_variables(void);
static bool 	energy_task_startup(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
bool energy_task_initialize(energy_thread_config_t* _energy_task_config)
{
	bool return_value = false;

	if(NULL != _energy_task_config)
	{
		energy_config = _energy_task_config;

		if(ABSL_EVENT_RV_OK == absl_event_create(energy_config->energy_events) &&
		  (ABSL_QUEUE_RV_OK == absl_queue_create(_energy_task_config->watchdog_queue, sizeof(consistency_counter), 1)))
		{
			if(energy_init(energy_config->energy_sensor_conf, energy_config->energy_events,
					       ENERGY_READ_FAST_VARIABLES, ENERGY_READ_SLOW_VARIABLES))
			{
				read_vars_timeout.seconds = 0;
				read_vars_timeout.nseconds = energy_config->time_between_vars_read_events_ms * 1000000;
				absl_timer_create(&read_vars_event_timer, &energy_read_vars_event_timeout_cb, NULL,
								read_vars_timeout, false, false);

				energy_normal_state = ENERGY_TASK_NORMAL_STATE_CONFIGURE;

				energy_initialize_variables();
				
				consistency_counter = 0;

				energy_config->energy_initialized = true;
				return_value = true;
			}
		}
	}

	return return_value;
}

/*!
 * @brief Entry point of the energy task
 *
 * @param *arg  Pointer to energy task parameters
 */
void energy_task(void* arg)
{
	uint32_t event_flags;

	uint8_t* system_state = (uint8_t*)arg;

	if(!energy_config->energy_initialized)
	{
		absl_debug_printf("Energy task was not initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	while(1)
	{
		if(ABSL_EVENT_RV_OK == absl_event_wait(energy_config->energy_events, ENERGY_EVENTS, &event_flags))
		{
			energy_state = energy_config->sensor_to_task_state[*system_state];
			switch(energy_state)
			{
			case ENERGY_TASK_IDLE:
				energy_task_idle_state(event_flags);
				break;
			case ENERGY_TASK_STATE_NORMAL:
				energy_task_normal_state(event_flags);
				break;
			case ENERGY_TASK_STATE_ERROR:
				break;
			default:
				absl_hardfault_handler(UNKNOWN_SWITCH_CASE_ERROR);
				break;
			}

			if(ENERGY_WATCHDOG == (event_flags & ENERGY_WATCHDOG))
			{
				consistency_counter++;
				absl_queue_send(energy_config->watchdog_queue, (void*)&consistency_counter, 1);
			}
		}
		else
		{
			absl_hardfault_handler(UNKNOWN_EVENT_ERROR);
		}
	}
}

static void energy_read_vars_event_timeout_cb(void* arg)
{
	ABSL_UNUSED_ARG(arg);

	absl_event_set_fromISR(energy_config->energy_events, ENERGY_READ_VARS_TIMEOUT);
}

static void energy_task_idle_state(uint32_t _events)
{
	if(ENERGY_READ_FAST_VARIABLES == (_events & ENERGY_READ_FAST_VARIABLES))
	{
		if(obtain_fast_vars)
		{
			energy_enable_data_obtaining_fast();
			energy_clear_waveform();
		}
	}
	if(ENERGY_READ_SLOW_VARIABLES == (_events & ENERGY_READ_SLOW_VARIABLES))
	{
		if(obtain_slow_vars)
		{
			energy_enable_data_obtaining_slow();
			energy_clear_slow_vars();
		}
	}
	if(ENERGY_STOP_FAST_VARS == (_events & ENERGY_STOP_FAST_VARS))
	{
		obtain_fast_vars = false;
		energy_clear_waveform();

		absl_event_set(energy_config->fast_vars_stream_events, energy_config->stream_disconnect);
	}
	if(ENERGY_STOP_SLOW_VARS == (_events & ENERGY_STOP_SLOW_VARS))
	{
		obtain_slow_vars = false;
		energy_clear_slow_vars();

		absl_event_set(energy_config->slow_vars_stream_events, energy_config->stream_disconnect);
	}
	if(ENERGY_CONFIG_RESET ==(_events & ENERGY_CONFIG_RESET))
	{
		energy_normal_state = ENERGY_TASK_NORMAL_STATE_CONFIGURE;
		absl_event_set(energy_config->system_events, energy_config->reconfig_event);
	}
}

static void energy_task_normal_state(uint32_t _events)
{
	switch(energy_normal_state)
	{
	case ENERGY_TASK_NORMAL_STATE_CONFIGURE:
		energy_task_configure_state(_events);
		break;

	case ENERGY_TASK_NORMAL_STATE_RUNNING:
		energy_task_running_state(_events);
		break;
	default:
		absl_hardfault_handler(UNKNOWN_SWITCH_CASE_ERROR);
		break;
	}
}

static void energy_task_configure_state(uint32_t _events)
{
	if(ENERGY_CONFIGURATION == (_events & ENERGY_CONFIGURATION))
	{
		if(true == energy_task_startup())
		{
			uint32_t event_to_send = energy_config->configured_event;

			if(obtain_fast_vars)
			{
				event_to_send |= energy_config->wf_running;
			}
			else
			{
				event_to_send |= energy_config->wf_stopped;
			}

			if(obtain_slow_vars)
			{
				event_to_send |= energy_config->reg_running;
			}
			else
			{
				event_to_send |= energy_config->reg_stopped;
			}

			energy_normal_state = ENERGY_TASK_NORMAL_STATE_RUNNING;
			absl_event_set(energy_config->system_events, event_to_send);
		}
	}
	if(ENERGY_READ_FAST_VARIABLES == (_events & ENERGY_READ_FAST_VARIABLES))
	{
		if(obtain_fast_vars)
		{
			energy_enable_data_obtaining_fast();
			energy_clear_waveform();
		}
	}
	if(ENERGY_READ_SLOW_VARIABLES == (_events & ENERGY_READ_SLOW_VARIABLES))
	{
		if(obtain_slow_vars)
		{
			energy_enable_data_obtaining_slow();
			energy_clear_slow_vars();
		}
	}
	if(ENERGY_START_FAST_VARS == (_events & ENERGY_START_FAST_VARS))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_START_RAW_NOT_ACCEPTED);
	}
	if(ENERGY_START_SLOW_VARS == (_events & ENERGY_START_SLOW_VARS))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_START_REGISTER_NOT_ACCEPTED);
	}
	if(ENERGY_STOP_FAST_VARS == (_events & ENERGY_STOP_FAST_VARS))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_STOP_RAW_NOT_ACCEPTED);
	}
	if(ENERGY_STOP_SLOW_VARS == (_events & ENERGY_STOP_SLOW_VARS))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_STOP_REGISTER_NOT_ACCEPTED);
	}
	if(ENERGY_CONFIG_RESET ==(_events & ENERGY_CONFIG_RESET))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_RESET_CFG_NOT_ACCEPTED);
	}
	else
	{
		
	}
}

static void energy_task_running_state(uint32_t _events)
{
	if(ENERGY_READ_FAST_VARIABLES == (_events & ENERGY_READ_FAST_VARIABLES))
	{
		if(obtain_fast_vars)
		{
			energy_enable_data_obtaining_fast();
			absl_event_set(energy_config->fast_vars_events, energy_config->vars_read);
		}
	}
	if(ENERGY_READ_SLOW_VARIABLES == (_events & ENERGY_READ_SLOW_VARIABLES))
	{
		if(obtain_slow_vars)
		{
			energy_enable_data_obtaining_slow();
			absl_event_set(energy_config->slow_vars_events, energy_config->vars_read);
		}
	}
	if(ENERGY_START_FAST_VARS == (_events & ENERGY_START_FAST_VARS))
	{
		if(!obtain_fast_vars)
		{
			energy_clear_waveform();

			absl_event_set(energy_config->fast_vars_stream_events, energy_config->stream_connect);
		}
		else
		{
			energy_notify_system_event(energy_config->event_info_array, E_EVENTS_FAST_SERVICE_ALREADY_RUNNING);
		}
	}
	if(ENERGY_FAST_CONNECTED == (_events & ENERGY_FAST_CONNECTED))
	{
		absl_event_set(energy_config->fast_vars_events, energy_config->vars_config);
	}
	if(ENERGY_FAST_CONFIGURED == (_events & ENERGY_FAST_CONFIGURED))
	{
		energy_enable_data_obtaining_fast();
		obtain_fast_vars = true;
		absl_event_set(energy_config->system_events, energy_config->wf_running);
	}
	if(ENERGY_FAST_NOT_CONNECTED == (_events & ENERGY_FAST_NOT_CONNECTED))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_FAST_SERVICE_COULD_NOT_START);
	}
	if(ENERGY_STOP_FAST_VARS == (_events & ENERGY_STOP_FAST_VARS))
	{
		if(true == obtain_fast_vars)
		{
			obtain_fast_vars = false;
			absl_event_set(energy_config->system_events, energy_config->wf_stopped);
			energy_clear_waveform();

			absl_event_set(energy_config->fast_vars_stream_events, energy_config->stream_disconnect);
		}
		else
		{
			energy_notify_system_event(energy_config->event_info_array, E_EVENTS_FAST_SERVICE_ALREADY_STOPPED);
		}
	}
	if(ENERGY_START_SLOW_VARS == (_events & ENERGY_START_SLOW_VARS))
	{
		if(!obtain_slow_vars)
		{
			energy_clear_slow_vars();

			absl_event_set(energy_config->slow_vars_stream_events, energy_config->stream_connect);
		}
		else
		{
			energy_notify_system_event(energy_config->event_info_array, E_EVENTS_SLOW_SERVICE_ALREADY_RUNNING);
		}
	}
	if(ENERGY_SLOW_CONNECTED == (_events & ENERGY_SLOW_CONNECTED))
	{
		absl_event_set(energy_config->slow_vars_events, energy_config->vars_config);
	}
	if(ENERGY_SLOW_CONFIGURED == (_events & ENERGY_SLOW_CONFIGURED))
	{
		energy_enable_data_obtaining_slow();
		obtain_slow_vars = true;
		absl_event_set(energy_config->system_events, energy_config->reg_running);
	}
	if(ENERGY_SLOW_NOT_CONNECTED == (_events & ENERGY_SLOW_NOT_CONNECTED))
	{
		energy_notify_system_event(energy_config->event_info_array, E_EVENTS_SLOW_SERVICE_COULD_NOT_START);
	}
	if(ENERGY_STOP_SLOW_VARS == (_events & ENERGY_STOP_SLOW_VARS))
	{
		if(true == obtain_slow_vars)
		{
			obtain_slow_vars = false;
			absl_event_set(energy_config->system_events, energy_config->reg_stopped);
			energy_clear_slow_vars();

			absl_event_set(energy_config->slow_vars_stream_events, energy_config->stream_disconnect);
		}
		else
		{
			energy_notify_system_event(energy_config->event_info_array, E_EVENTS_SLOW_SERVICE_ALREADY_STOPPED);
		}
	}
	if(ENERGY_CONFIG_RESET ==(_events & ENERGY_CONFIG_RESET))
	{
		energy_normal_state = ENERGY_TASK_NORMAL_STATE_CONFIGURE;
		absl_event_set(energy_config->system_events, energy_config->reconfig_event);
	}
	if(ENERGY_CONFIGURATION == (_events & ENERGY_CONFIGURATION))
	{
		energy_clear_waveform();
		energy_clear_slow_vars();

		if(true == energy_configure())
		{
			energy_notify_system_event(energy_config->event_info_array, E_EVENTS_RECONFIGURED);
		}
	}
}

static void energy_initialize_variables(void)
{
	obtain_fast_vars = false;
	obtain_slow_vars = false;
	event_timeout_error_count = 0;
}

static bool energy_task_startup(void)
{
	bool startup_ok = false;

	if(!energy_startup())
	{
		absl_debug_printf("ERROR! Energy startup process failed.\n");
	}
	else
	{
		startup_ok = true;
	}

	return startup_ok;
}

