/*
 * cmd_task.h
 *
 *  Created on: Mar 23, 2023
 *      Author: abolinaga
 */

#ifndef CMD_TASK_H_
#define CMD_TASK_H_
/******************************************************************************
 * Includes
 ******************************************************************************/
#include "protocol_types.h"

#include "absl_queue.h"
#include "absl_event.h"
#include "absl_mutex.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define CMD_RECEIVED				0x00000001
#define CMD_CONNECT					0x00000002
#define CMD_SERVER_TIMEOUT			0x00000004
#define CMD_FRAME_RECEIVED			0x00000008
#define CMD_SEND_ONLINE				0x00000010
#define CMD_SERVER_DISCONNECTED		0x00000020
#define CMD_SEND_STATUS_CHANGE		0x00000040
#define CMD_SEND_ALERT				0x00000080
#define CMD_SERVER_DISCONNECT		0x00000100
#define CMD_BUFF_IS_FULL			0x00000200

#define CMD_EVENTS		CMD_RECEIVED 			|\
						CMD_CONNECT	 			|\
						CMD_SERVER_TIMEOUT		|\
						CMD_FRAME_RECEIVED		|\
						CMD_SERVER_DISCONNECTED	|\
						CMD_SEND_ONLINE			|\
						CMD_SEND_STATUS_CHANGE	|\
						CMD_SEND_ALERT			|\
						CMD_SERVER_DISCONNECT   |\
						CMD_BUFF_IS_FULL

typedef enum cmd_task_states
{
	CMD_TASK_STATE_IDLE = 0,
	CMD_TASK_STATE_CONNECT,
	CMD_TASK_STATE_NORMAL,
	CMD_TASK_STATE_FW_UPDATE,
	CMD_TASK_STATE_ERROR
}cmd_task_states_t;

typedef enum cmd_events
{
	CMD_EVENTS_ERROR_SENDING_DISCOVERY = 0,
	CMD_EVENTS_ERROR_SENDING_STATUS,
	CMD_EVENTS_INVALID_CMD,
	CMD_EVENTS_UPDATE_INCORRECT_STATE,
	CMD_EVENTS_ERROR_GETTING_CMD,
	CMD_EVENTS_FULL_RX_BUFFER,
	CMD_EVENTS_INCORRECT_MANUFACTURING,
	CMD_EVENTS_STATE_CHANGE_DATA_TIMEOUT,
	CMD_EVENTS_INVALID_CONFIG_PAYLOAD,
	CMD_EVENTS_INVALID_START_PAYLOAD,
	CMD_EVENTS_INVALID_SYNC_PAYLOAD,
	CMD_EVENTS_INVALID_MANUFACTUR_PAYLOAD,
	CMD_EVENTS_MAXVALUE
}cmd_events_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef struct cmd_thread_config
{
	protocol_config_t*	    protocol_config;
	void *					hw_version;
	absl_queue_t*				states_queue;
	absl_queue_t*				alets_queue;
	absl_event_t*				cmd_events;
	absl_event_t*				system_events;
	uint32_t				connected_event;
	uint32_t				no_server_event;
	uint32_t				fw_update_event;
	uint32_t				disconnected_event;
	absl_event_t**			sensor_event_groups;
	uint32_t*				config_events;
	uint32_t*				reset_events;
	uint32_t**				start_events;
	uint32_t**				stop_events;
	absl_event_t*				timesync_events;
	uint32_t				sync_event;
	absl_event_t*				fw_update_events;
	uint32_t				fw_file_chunk_received;
	uint32_t				fw_file_reception_finished;
	cmd_task_states_t*		sensor_to_task_state;
	void* 					event_info_array;
	bool					cmd_initialized;
}cmd_thread_config_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool cmd_task_initialize(cmd_thread_config_t* _cmd_task_config);

void cmd_task(void *arg);


#endif /* CMD_TASK_H_ */
