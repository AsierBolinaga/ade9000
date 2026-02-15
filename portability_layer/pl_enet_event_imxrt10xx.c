/*
 * pl_input_output_event.c
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_enet_event.h"

#ifdef PL_ENET_EVENT
#ifdef PL_OS_FREE_RTOS
#include "ethernetif.h"
#include "pl_debug.h"
#include "pl_time.h"

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL

static void enet1588_callback(void* _arg)
{
	pl_enet_event_t* input_event = (pl_enet_event_t*)_arg;

	enet_1588_get_event_time(&input_event->event_time.seconds, &input_event->event_time.nseconds,
			                  input_event->event_config->channel);

	input_event->actual_time_us = pl_enet_event_get_event_time_us_imxrt10xx(input_event);

	/* in the sdk seconds are handled by software. Sometimes they are increased too late, and
	   previous time can be higher than the actual time, leading to errors, this has to be handled*/
	if(input_event->previous_time_us > input_event->actual_time_us)
	{
		input_event->event_time.seconds++;
		input_event->actual_time_us = pl_enet_event_get_event_time_us_imxrt10xx(input_event);
	}

	input_event->previous_time_us = input_event->actual_time_us;
	pl_event_set_fromISR(input_event->event_group, input_event->signal_to_give);
}


void pl_enet_event_init_imxrt10xx(pl_enet_event_t * _pl_enet_event, pl_enet_event_config_t* _io_event_config,
							      pl_event_t* _event_group, uint32_t _event_mask)
{
	pl_phy_init(&_pl_enet_event->enet_phy, _io_event_config->event_phy);

	_pl_enet_event->event_config = _io_event_config;
	_pl_enet_event->event_group = _event_group;
	_pl_enet_event->signal_to_give = _event_mask;

	_pl_enet_event->previous_time_us = 0;
}

void pl_enet_event_enable_imxrt10xx(pl_enet_event_t * _pl_enet_event)
{
	enet_1588_event_activate(enet1588_callback, _pl_enet_event->event_config->channel_mode,
			_pl_enet_event->event_config->channel, (void*)_pl_enet_event);
}

void pl_enet_event_disable_imxrt10xx(pl_enet_event_t * _pl_enet_event)
{
	enet_1588_event_deactivate( _pl_enet_event->event_config->channel);
}

void pl_enet_event_get_event_time_imxrt10xx(pl_enet_event_t* _io_event, pl_time_t * _io_event_time)
{
	*_io_event_time = _io_event->event_time;
}

uint64_t pl_enet_event_get_event_time_ns_imxrt10xx(pl_enet_event_t* _io_event)
{
	return (_io_event->event_time.seconds * NANOSECONDS_IN_SECOND) + _io_event->event_time.nseconds;
}

uint64_t pl_enet_event_get_event_time_us_imxrt10xx(pl_enet_event_t* _io_event)
{
	return (_io_event->event_time.seconds * MICROSECONDS_IN_SECOND) + (_io_event->event_time.nseconds/1000);
}

uint64_t* pl_enet_event_get_pointer_to_event_time_us_imxrt10xx(pl_enet_event_t* _io_event)
{
	return &_io_event->actual_time_us;
}

#endif
#endif
