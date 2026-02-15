/*
 * pl_input_output_event.h
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_IO_EVENT_H_
#define PL_IO_EVENT_H_

#include "pl_config.h"

#ifdef PL_IO_EVENT
#include "pl_time.h"
#include "pl_event.h"

typedef enum pl_io_event_type
{
	PL_INPUT_CAPTURE_EVENT = 0,
	PL_OUTPU_COMPARE_EVENT
}pl_io_event_type_t;

typedef enum pl_io_input_event_type
{
	PL_INPUT_RISING_EDGET 			  = 0x00000001,
	PL_INPUT_FALLING_EDGE 			  = 0x00000002,
	PL_INPUT_FALLING_AND_RISING_EDEGE = 0x00000004,
}pl_io_input_event_type_t;

typedef struct pl_io_event_config
{
	pl_io_event_type_t 			event_type;
	pl_io_input_event_type_t	input_event_type;
	pl_event_t*					event_group;
	uint32_t					signal_to_give;
}pl_io_event_config_t;

typedef struct pl_io_event
{
	pl_io_event_config_t*		event_config;
	pl_time_t					event_time;
	pl_event_t					io_event_groip;
	pl_io_input_event_type_t	io_event_to_give;
}pl_io_event_t;


#if defined(PL_OS_FREE_RTOS)
#define pl_io_event_init		pl_io_event_init_freertos
#define pl_io_event_enable		pl_io_event_enable_freertos
#else
#error Platform not defined
#endif

void pl_io_event_init(pl_io_event_t * _pl_io_event, pl_io_event_config_t* _io_event_config);

void pl_io_event_enable(pl_io_event_t * _pl_io_event);

void pl_io_event_get_event_time(pl_io_event_t * _pl_io_event, pl_time_t* _event_time);

uint64_t pl_io_event_get_event_time_ns(pl_io_event_t* _io_event);

uint64_t pl_io_event_get_event_time_us(pl_io_event_t* _io_event);

#endif /* PL_IO_EVENT */
#endif /* PL_IO_EVENT_H_ */
