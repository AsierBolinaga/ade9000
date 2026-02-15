/*
 * pl_input_output_event.c
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */
#ifdef PL_IO_EVENT

#include "pl_io_event.h"

#ifdef PL_OS_FREE_RTOS
#include "ethernetif.h"
#include "pl_debug.h"

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL

static pl_io_event_t* input_event;


static void enet1588_callback(ENET_Type *base, enet_handle_t *handle, enet_event_t event, void *param)
{
	pl_io_event_t* input_event = (pl_io_event_t*)param;

	enet_1588_get_event_time(&input_event->event_time.seconds, &input_event->event_time.nseconds);

	pl_event_set_fromISR(input_event->event_config->event_group, input_event->event_config->signal_to_give);
}


void pl_io_event_init_freertos(pl_io_event_t * _pl_io_event, pl_io_event_config_t* _io_event_config)
{
	_pl_io_event->event_config = _io_event_config;
	input_event = _pl_io_event;
}

void pl_io_event_enable_freertos(pl_io_event_t * _pl_io_event)
{
	enet_1588_event_activate(enet1588_callback,  (void*)_pl_io_event);
}

#endif

void pl_io_event_get_event_time(pl_io_event_t* _io_event, pl_time_t * _io_event_time)
{
	*_io_event_time = _io_event->event_time;
}

uint64_t pl_io_event_get_event_time_ns(pl_io_event_t* _io_event)
{
	return (_io_event->event_time.seconds * NANOSECONDS_IN_SECOND) + _io_event->event_time.nseconds;
}

uint64_t pl_io_event_get_event_time_us(pl_io_event_t* _io_event)
{
	return (_io_event->event_time.seconds * MICROSECONDS_IN_SECOND) + (_io_event->event_time.nseconds/1000);
}

#endif /* PL_IO_EVENT */
