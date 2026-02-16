/*
 * event_handler.h
 *
 *  Created on: Jun 16, 2023
 *      Author: abolinaga
 */

#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include "system_types.h"

#include "absl_event.h"
#include "absl_queue.h"

typedef struct event_data_to_send
{
	event_type_t			type;
	state_data_to_send_t	data;
}event_data_to_send_t;

void event_handler_init(sensor_config_t* _sensors_cfg, absl_event_t* _system_events_group, uint32_t _task_event_or_error_mask, absl_queue_t* _event_queue);

event_data_to_send_t event_handler_new_event_code(event_info_t* _event);

error_code_t* event_handler_get_error_codes(sensors_t _sensor, services_t _service,  uint32_t* _error_ammount);

void event_handler_notify_system_event(void* _events_info, uint32_t _event);

#endif /* EVENT_HANDLER_H_ */
