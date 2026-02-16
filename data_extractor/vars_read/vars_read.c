/*
 * energy.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#include "vars_read.h"

#include "interfaces.h"

#include "absl_debug.h"
#include "absl_time.h"
#include "absl_system.h"

/*******************************************************************************
 * type definitions
 ******************************************************************************/

/******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
bool vars_read_task_initialize(vars_read_thread_data_t* vars_read_thread_data, vars_read_thread_config_t* _vars_read_task_config)
{
	bool return_value = false;

	if(NULL != _vars_read_task_config)
	{
		if(ABSL_EVENT_RV_OK == absl_event_create(_vars_read_task_config->vars_events) &&
		  (ABSL_QUEUE_RV_OK == absl_queue_create(_vars_read_task_config->watchdog_queue,
				                             sizeof(vars_read_thread_data->consistency_counter), 1)))
		{
			vars_read_thread_data->consistency_counter = 0;

			_vars_read_task_config->vars_read_initialized = true;
			return_value = true;
		}
	}

	return return_value;
}

/*!
 * @brief Entry point of the energy task
 *
 * @param *arg  Pointer to energy task parameters
 */
void vars_read_task(void* arg)
{
	vars_read_thread_t* vars_read_thread = (vars_read_thread_t*)arg;

	vars_read_thread_data_t* 	vars_read_thread_data = vars_read_thread->vars_read_thread_data;
	vars_read_thread_config_t* 	vars_read_thread_config = vars_read_thread->vars_read_thread_config;
	
	uint32_t event_flags;
	bool data_to_queue = false;

	if(!vars_read_thread_config->vars_read_initialized)
	{
		absl_debug_printf("Slow variables task was not initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	while(1)
	{
		if(ABSL_EVENT_RV_OK == absl_event_wait(vars_read_thread_config->vars_events, VARS_READ_EVENTS, &event_flags))
		{
			if(VARS_WATCHDOG == (event_flags & VARS_WATCHDOG))
			{
				vars_read_thread_data->consistency_counter++;
				absl_queue_send(vars_read_thread_config->watchdog_queue, (void*)&vars_read_thread_data->consistency_counter, 1);
			}
			if(VARS_CONFIG == (event_flags & VARS_CONFIG))
			{
				if(true == vars_read_thread_config->vars_config_cb())
				{
					absl_event_set(vars_read_thread_config->energy_event, vars_read_thread_config->vars_configured_flag);
				}
				else
				{
					vars_read_notify_system_event(vars_read_thread_config->event_info_array, VR_INVALID_VARIABLES_CONFIG);
				}
			}
			if(VARS_READ == (event_flags & VARS_READ))
			{
				if(true == data_to_queue)
				{
					if(ABSL_QUEUE_RV_FULL == absl_queue_send(vars_read_thread_config->read_data_send_queue, vars_read_thread_config->read_vars,
											ABSL_QUEUE_NO_DELAY))
					{
						vars_read_notify_system_event(vars_read_thread_config->event_info_array, VR_QUEUE_FULL);
					}
					data_to_queue = false;
				}

				if(vars_read_thread_config->vars_read_cb(vars_read_thread_config->vars_config, vars_read_thread_config->read_vars))
				{
					absl_event_set(vars_read_thread_config->stream_event, vars_read_thread_config->data_to_send_flag);
					if(ABSL_QUEUE_RV_FULL == absl_queue_send(vars_read_thread_config->read_data_send_queue, vars_read_thread_config->read_vars,
							*vars_read_thread_config->queue_send_wait_ms))
					{
						absl_debug_printf("Vars queue full!\n");
						data_to_queue = true;
					}
				}
			}
		}
	}
}
