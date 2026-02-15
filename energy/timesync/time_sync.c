/*
 * time_sync.c
 *
 *  Created on: May 30, 2023
 *      Author: abolinaga
 */
#include "time_sync.h"

#include "interfaces.h"

#include "pl_system.h"
#include "pl_ntp.h"
#include "pl_thread.h"
#include "pl_debug.h"
#include "pl_hw_config.h"
#include "pl_timer.h"
#include "pl_time.h"
#include "pl_macros.h"

static time_sync_thread_config_t* time_sync_config;

static pl_ntp_t 			sync_ntp;
static pl_ntp_config_t* 	sync_ntp_config;

static pl_timer_t	timer_sync;

static void time_sync_do_synchronization(void);

void time_sync_timeout(void* arg)
{
	PL_UNUSED_ARG(arg);
	pl_event_set_fromISR_freertos(time_sync_config->time_sync_events, TIMESYNC_DO_SYNC);
}

bool time_sync_initialize(time_sync_thread_config_t* _time_sync_config)
{
	bool return_value = false;
	pl_time_t init_period;

	if(NULL != _time_sync_config)
	{
		time_sync_config = _time_sync_config;

		sync_ntp_config = pl_config_get_ntp_conf(time_sync_config->tyme_sync_ntp_index);

		if(PL_NTP_RV_OK == pl_ntp_init(&sync_ntp, sync_ntp_config))
		{
			if(PL_EVENT_RV_OK == pl_event_create(time_sync_config->time_sync_events))
			{
				init_period.nseconds = 0;
				init_period.seconds = 3600;

				pl_timer_create(&timer_sync, &time_sync_timeout, NULL, init_period, true, false);

				time_sync_config->time_sync_initialized = true;
				return_value = true;
			}
		}
	}

	return return_value;
}

void time_sync_task(void* arg)
{
	uint32_t event_flags = 0;


	sync_type_t	sync_type;
	pl_time_t	sync_period;

	PL_UNUSED_ARG(arg);

	if(!time_sync_config->time_sync_initialized)
	{
		pl_debug_printf("time sync task was not initialized!\n");
		pl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	while(1)
	{
		pl_event_wait(time_sync_config->time_sync_events, TIMESYNC_EVENTS, &event_flags);

		if(TIMESYNC_DO_SYNC == (event_flags & TIMESYNC_DO_SYNC))
		{
			time_sync_do_synchronization();
		}
		if(TIMESYNC_SYNC_MSG == (event_flags & TIMESYNC_SYNC_MSG))
		{
			sync_type = time_sync_get_sync_type();

			if(SYNC_POLL == sync_type)
			{
				sync_period = time_sync_get_sync_period();
				pl_timer_change(&timer_sync, sync_period, true);

				time_sync_do_synchronization();
			}
			else if(SYNC_NOW == sync_type)
			{
				time_sync_do_synchronization();
			}
			else
			{
				/* TODO - warning */
				time_sync_notify_system_event(time_sync_config->event_info_array, TS_EVENTS_UNKNOWN_SYNC_TYPE);
			}
		}
	}
}

static void time_sync_do_synchronization(void)
{
	if(PL_NTP_RV_OK == pl_ntp_sync_time(&sync_ntp))
	{
		time_sync_notify_system_event(time_sync_config->event_info_array, TS_EVENTS_SYNC);
	}
	else
	{
		time_sync_notify_system_event(time_sync_config->event_info_array, TS_EVENTS_SYNC_TIMEOUT);
	}

	pl_event_set(time_sync_config->system_events, time_sync_config->sync_process_finished);
}
