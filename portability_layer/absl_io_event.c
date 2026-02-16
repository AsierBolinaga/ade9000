/*
 * absl_input_output_event.c
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */
#ifdef ABSL_IO_EVENT

#include "absl_io_event.h"

#ifdef ABSL_OS_FREE_RTOS
#include "ethernetif.h"
#include "absl_debug.h"

#define NANOSECONDS_IN_SECOND	1000000000ULL
#define MICROSECONDS_IN_SECOND	1000000ULL

static absl_io_event_t* input_event;


static void enet1588_callback(ENET_Type *base, enet_handle_t *handle, enet_event_t event, void *param)
{
	absl_io_event_t* input_event = (absl_io_event_t*)param;

	enet_1588_get_event_time(&input_event->event_time.seconds, &input_event->event_time.nseconds);

	absl_event_set_fromISR(input_event->event_config->event_group, input_event->event_config->signal_to_give);
}


void absl_io_event_init_freertos(absl_io_event_t * _absl_io_event, absl_io_event_config_t* _io_event_config)
{
	_absl_io_event->event_config = _io_event_config;
	input_event = _absl_io_event;
}

void absl_io_event_enable_freertos(absl_io_event_t * _absl_io_event)
{
	enet_1588_event_activate(enet1588_callback,  (void*)_absl_io_event);
}

#endif

void absl_io_event_get_event_time(absl_io_event_t* _io_event, absl_time_t * _io_event_time)
{
	*_io_event_time = _io_event->event_time;
}

uint64_t absl_io_event_get_event_time_ns(absl_io_event_t* _io_event)
{
	return (_io_event->event_time.seconds * NANOSECONDS_IN_SECOND) + _io_event->event_time.nseconds;
}

uint64_t absl_io_event_get_event_time_us(absl_io_event_t* _io_event)
{
	return (_io_event->event_time.seconds * MICROSECONDS_IN_SECOND) + (_io_event->event_time.nseconds/1000);
}

#endif /* ABSL_IO_EVENT */
