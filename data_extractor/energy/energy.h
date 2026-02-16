/*
 * energy.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ENERGY_H_
#define ENERGY_H_
/******************************************************************************
 * Includes
 ******************************************************************************/
#include "sensors_types.h"

#include "absl_event.h"
#include "absl_queue.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/
#define ENERGY_READ_FAST_VARIABLES 		0x00000001
#define ENERGY_READ_SLOW_VARIABLES 		0x00000002
#define ENERGY_START_FAST_VARS			0x00000004
#define ENERGY_STOP_FAST_VARS			0x00000008
#define ENERGY_START_SLOW_VARS			0x00000010
#define ENERGY_STOP_SLOW_VARS			0x00000020
#define ENERGY_READ_VARS_TIMEOUT		0x00000040
#define ENERGY_CONFIGURATION			0x00000200
#define ENERGY_CONFIG_RESET				0x00000400
#define ENERGY_FAST_CONNECTED			0x00000800
#define ENERGY_FAST_CONFIGURED			0x00001000
#define ENERGY_FAST_NOT_CONNECTED		0x00002000
#define ENERGY_SLOW_CONNECTED			0x00004000
#define ENERGY_SLOW_CONFIGURED			0x00008000
#define ENERGY_SLOW_NOT_CONNECTED		0x00010000
#define ENERGY_WATCHDOG					0x00020000

#define ENERGY_EVENTS	ENERGY_READ_FAST_VARIABLES	| \
						ENERGY_READ_SLOW_VARIABLES	| \
						ENERGY_START_FAST_VARS 		| \
						ENERGY_STOP_FAST_VARS 		| \
						ENERGY_START_SLOW_VARS 		| \
						ENERGY_STOP_SLOW_VARS		| \
						ENERGY_READ_VARS_TIMEOUT	| \
						ENERGY_CONFIGURATION		| \
						ENERGY_CONFIG_RESET			| \
						ENERGY_FAST_CONNECTED		| \
						ENERGY_FAST_CONFIGURED		| \
						ENERGY_FAST_NOT_CONNECTED	| \
						ENERGY_SLOW_CONNECTED		| \
						ENERGY_SLOW_CONFIGURED		| \
						ENERGY_SLOW_NOT_CONNECTED	| \
						ENERGY_WATCHDOG

typedef enum energy_events
{
	E_EVENTS_VAR_READ_EVENT_TIMEOUT = 0,
	E_EVENTS_RESET_CFG_NOT_ACCEPTED,
	E_EVENTS_START_RAW_NOT_ACCEPTED,
	E_EVENTS_STOP_RAW_NOT_ACCEPTED,
	E_EVENTS_START_REGISTER_NOT_ACCEPTED,
	E_EVENTS_STOP_REGISTER_NOT_ACCEPTED,
	E_EVENTS_RECONFIGURED,
	E_EVENTS_IRQ_NOT_WORKING,
	E_EVENTS_FAST_SERVICE_COULD_NOT_START,
	E_EVENTS_SLOW_SERVICE_COULD_NOT_START,
	E_EVENTS_FAST_SERVICE_ALREADY_RUNNING,
	E_EVENTS_SLOW_SERVICE_ALREADY_RUNNING,
	E_EVENTS_FAST_SERVICE_ALREADY_STOPPED,
	E_EVENTS_SLOW_SERVICE_ALREADY_STOPPED,
	E_EVENTS_MAXVALUE
}energy_events_t;

typedef enum energy_task_states
{
	ENERGY_TASK_IDLE = 0,
	ENERGY_TASK_STATE_NORMAL,
	ENERGY_TASK_STATE_ERROR
}energy_task_states_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef struct energy_thread_config
{
	energy_sensor_init_conf_t* 	energy_sensor_conf;
	absl_event_t*					system_events;
	uint32_t					configured_event;
	uint32_t					reconfig_event;
	uint32_t					wf_running;
	uint32_t					wf_stopped;
	uint32_t					reg_running;
	uint32_t					reg_stopped;
	absl_event_t*			 		energy_events;
	absl_event_t*			 		fast_vars_events;
	absl_event_t*			 		slow_vars_events;
	uint32_t					vars_config;
	uint32_t					vars_read;
	absl_event_t*			 		fast_vars_stream_events;
	absl_event_t*			 		slow_vars_stream_events;
	uint32_t					stream_connect;
	uint32_t					stream_disconnect;
	uint32_t			 		sensor_label;
	uint32_t			 		slow_vars_service_label;
	uint32_t			 		wvf_service_label;
	uint32_t			 		time_between_vars_read_events_ms;
	absl_queue_t*					watchdog_queue;
	energy_task_states_t*		sensor_to_task_state;
	void* 						event_info_array;
	bool				 		energy_initialized;
}energy_thread_config_t;

/******************************************************************************
 * Prototypes
 ******************************************************************************/
bool energy_task_initialize(energy_thread_config_t* _energy_task_config);

void energy_task(void* arg);

#endif /* ENERGY_H_ */
