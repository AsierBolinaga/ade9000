/*
 * energy.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef VARS_READ_H_
#define VARS_READ_H_
/******************************************************************************
 * Includes
 ******************************************************************************/
#include "sensors_types.h"

#include "pl_event.h"
#include "pl_queue.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/
#define VARS_READ 			0x00000001
#define VARS_CONFIG			0x00000002

#define VARS_READ_EVENTS	VARS_READ		| \
							VARS_CONFIG

typedef enum var_read_events
{
	VR_QUEUE_FULL = 0,
	VR_INVALID_VARIABLES_CONFIG,
	VR_EVENTS_MAXVALUE
}var_read_events;

/******************************************************************************
 * Type definitions
 ******************************************************************************/

typedef struct vars_read_thread_config
{
	pl_event_t*			 		vars_events;
	pl_event_t*					stream_event;
	uint32_t					data_to_send_flag;
	pl_event_t*					energy_event;
	uint32_t					vars_configured_flag;
	pl_queue_t*			 		read_data_send_queue;
	float*			 			queue_send_wait_ms;
	bool 						(*vars_read_cb)(void* _vars_config, void* _vars_value);
	bool 						(*vars_config_cb)(void);
	void*						read_vars;
	void* 						vars_config;
	void* 						event_info_array;
	bool				 		vars_read_initialized;
}vars_read_thread_config_t;

typedef struct vars_read_thread
{
 	vars_read_thread_config_t* 	vars_read_thread_config;
}vars_read_thread_t;


/******************************************************************************
 * Prototypes
 ******************************************************************************/
bool vars_read_task_initialize(vars_read_thread_config_t* _vars_read_task_config);

void vars_read_task(void* arg);

#endif /* VARS_READ_H_ */
