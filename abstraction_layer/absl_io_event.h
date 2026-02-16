/*
 * absl_input_output_event.h
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_IO_EVENT_H_
#define ABSL_IO_EVENT_H_

#include "absl_config.h"

#ifdef ABSL_IO_EVENT
#include "absl_time.h"
#include "absl_event.h"

typedef enum absl_io_event_type
{
	ABSL_INPUT_CAPTURE_EVENT = 0,
	ABSL_OUTPU_COMPARE_EVENT
}absl_io_event_type_t;

typedef enum absl_io_input_event_type
{
	ABSL_INPUT_RISING_EDGET 			  = 0x00000001,
	ABSL_INPUT_FALLING_EDGE 			  = 0x00000002,
	ABSL_INPUT_FALLING_AND_RISING_EDEGE = 0x00000004,
}absl_io_input_event_type_t;

typedef struct absl_io_event_config
{
	absl_io_event_type_t 			event_type;
	absl_io_input_event_type_t	input_event_type;
	absl_event_t*					event_group;
	uint32_t					signal_to_give;
}absl_io_event_config_t;

typedef struct absl_io_event
{
	absl_io_event_config_t*		event_config;
	absl_time_t					event_time;
	absl_event_t					io_event_groip;
	absl_io_input_event_type_t	io_event_to_give;
}absl_io_event_t;


#if defined(ABSL_OS_FREE_RTOS)
#define absl_io_event_init		absl_io_event_init_freertos
#define absl_io_event_enable		absl_io_event_enable_freertos
#else
#error Platform not defined
#endif

void absl_io_event_init(absl_io_event_t * _absl_io_event, absl_io_event_config_t* _io_event_config);

void absl_io_event_enable(absl_io_event_t * _absl_io_event);

void absl_io_event_get_event_time(absl_io_event_t * _absl_io_event, absl_time_t* _event_time);

uint64_t absl_io_event_get_event_time_ns(absl_io_event_t* _io_event);

uint64_t absl_io_event_get_event_time_us(absl_io_event_t* _io_event);

#endif /* ABSL_IO_EVENT */
#endif /* ABSL_IO_EVENT_H_ */
