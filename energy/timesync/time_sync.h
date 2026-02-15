/*
 * time_sync.h
 *
 *  Created on: May 30, 2023
 *      Author: abolinaga
 */

#ifndef TIMESYNC_TIME_SYNC_H_
#define TIMESYNC_TIME_SYNC_H_

#include "pl_types.h"
#include "pl_event.h"

#define TIMESYNC_SYNC_MSG	0x00000001
#define TIMESYNC_DO_SYNC	0x00000002

#define TIMESYNC_EVENTS		TIMESYNC_SYNC_MSG |\
							TIMESYNC_DO_SYNC


typedef enum time_sync_events
{
	TS_EVENTS_SYNC_TIMEOUT = 0,
	TS_EVENTS_SYNC,
	TS_EVENTS_UNKNOWN_SYNC_TYPE,
	TS_EVENTS_MAXVALUE
}time_sync_events_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef struct time_sync_thread_config
{
	uint8_t			tyme_sync_ntp_index;
	pl_event_t*		time_sync_events;
	pl_event_t*		system_events;
	uint32_t		sync_process_finished;
	void* 			event_info_array;
	bool			time_sync_initialized;
}time_sync_thread_config_t;

bool time_sync_initialize(time_sync_thread_config_t* _time_sync_config);

void time_sync_task(void* arg);

#endif /* TIMESYNC_TIME_SYNC_H_ */
