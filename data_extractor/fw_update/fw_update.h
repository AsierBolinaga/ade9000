/*
 * fw_update_task.h
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */

#ifndef FW_UPDATE_TASK_H_
#define FW_UPDATE_TASK_H_
/******************************************************************************
 * Includes
 ******************************************************************************/
#include "system_types.h"

#include "absl_types.h"
#include "absl_event.h"
#include "absl_queue.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define FW_UPDATE_CHUNK_RECEIVED		0x00000001
#define FW_UPDATE_RECEPTION_FINISHED	0x00000002
#define FW_UPDATE_RESET_FW_UPDATE		0x00000004
#define FW_UPDATE_ERASE_NEW_FW_SECTOR	0x00000008
#define FW_UPDATE_WATCHDOG				0x00000010

#define FW_UPDATE_EVENTS	FW_UPDATE_CHUNK_RECEIVED		|\
							FW_UPDATE_RECEPTION_FINISHED	|\
							FW_UPDATE_RESET_FW_UPDATE		|\
							FW_UPDATE_ERASE_NEW_FW_SECTOR	|\
							FW_UPDATE_WATCHDOG

typedef enum fw_update_events
{
	FWU_EVENTS_WRITE_FAILED = 0,
	FWU_EVENTS_AREA_EXCEEDED,
	FWU_EVENTS_UPDATE_AREA_NOT_EMPTY,
	FWU_EVENTS_MAXVALUE
}fw_events_codes_t;


typedef enum fw_update_task_states
{
	FW_UPDATE_TASK_STATE_IDLE = 0,
	FW_UPDATE_TASK_STATE_ACTIVE
}fw_update_task_states_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef struct fw_update_thread_config
{
	uint8_t 						nvm_conf_index;
	fw_update_handling_conf_t		fw_update_handling_conf;
	absl_event_t* 					fw_update_event_group;
	fw_update_task_states_t*		sensor_to_task_state;
	uint32_t						erase_chunks;
	absl_event_t*						system_event_group;
	uint32_t						erase_done_event;
	uint32_t						erase_needed;
	uint32_t						fw_update_done_flag;
	uint32_t						fw_update_started_flag;
	absl_queue_t*						watchdog_queue;
	event_info_t* 					event_info_array;
	bool							fw_update_initialized;
}fw_update_thread_config_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool fw_update_task_initialize(fw_update_thread_config_t* _fw_update_task_config);

void fw_update_task(void *arg);

#endif /* FW_UPDATE_TASK_H_ */
