/*
 * absl_input_output_event.c
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_enet_event.h"

#ifdef ABSL_ENET_EVENT
#ifdef ABSL_OS_FREE_RTOS
#include "ethernetif.h"
#include "absl_debug.h"
#include "absl_time.h"

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL

static void enet1588_callback(void* _arg)
{
	absl_enet_event_t* input_event = (absl_enet_event_t*)_arg;

	enet_1588_get_event_time(&input_event->event_time.seconds, &input_event->event_time.nseconds,
			                  input_event->event_config->channel);

	input_event->actual_time_us = absl_enet_event_get_event_time_us_imxrt10xx(input_event);

	/* in the sdk seconds are handled by software. Sometimes they are increased too late, and
	   previous time can be higher than the actual time, leading to errors, this has to be handled*/
	if(input_event->previous_time_us > input_event->actual_time_us)
	{
		input_event->event_time.seconds++;
		input_event->actual_time_us = absl_enet_event_get_event_time_us_imxrt10xx(input_event);
	}

	input_event->previous_time_us = input_event->actual_time_us;
	absl_event_set_fromISR(input_event->event_group, input_event->signal_to_give);
}


void absl_enet_event_init_imxrt10xx(absl_enet_event_t * _absl_enet_event, absl_enet_event_config_t* _io_event_config,
							      absl_event_t* _event_group, uint32_t _event_mask)
{
	absl_phy_init(&_absl_enet_event->enet_phy, _io_event_config->event_phy);

	_absl_enet_event->event_config = _io_event_config;
	_absl_enet_event->event_group = _event_group;
	_absl_enet_event->signal_to_give = _event_mask;

	_absl_enet_event->previous_time_us = 0;
}

void absl_enet_event_enable_imxrt10xx(absl_enet_event_t * _absl_enet_event)
{
	enet_1588_event_activate(enet1588_callback, _absl_enet_event->event_config->channel_mode,
			_absl_enet_event->event_config->channel, (void*)_absl_enet_event);
}

void absl_enet_event_disable_imxrt10xx(absl_enet_event_t * _absl_enet_event)
{
	enet_1588_event_deactivate( _absl_enet_event->event_config->channel);
}

void absl_enet_event_get_event_time_imxrt10xx(absl_enet_event_t* _io_event, absl_time_t * _io_event_time)
{
	*_io_event_time = _io_event->event_time;
}

uint64_t absl_enet_event_get_event_time_ns_imxrt10xx(absl_enet_event_t* _io_event)
{
	return (_io_event->event_time.seconds * NANOSECONDS_IN_SECOND) + _io_event->event_time.nseconds;
}

uint64_t absl_enet_event_get_event_time_us_imxrt10xx(absl_enet_event_t* _io_event)
{
	return (_io_event->event_time.seconds * MICROSECONDS_IN_SECOND) + (_io_event->event_time.nseconds/1000);
}

uint64_t* absl_enet_event_get_pointer_to_event_time_us_imxrt10xx(absl_enet_event_t* _io_event)
{
	return &_io_event->actual_time_us;
}

#endif
#endif
