/*
 * event_handler.c
 *
 *  Created on: Jun 16, 2023
 *      Author: abolinaga
 */

#include "event_handler.h"

#include "pl_time.h"
#include "pl_debug.h"

#define ERROR_CODE_ARRAY_NUMBER 	SENSOR_MAXVALUE + SERVICE_MAXVALUE + 1
#define ERROR_CODES_SYSTEM			SENSOR_MAXVALUE + SERVICE_MAXVALUE


typedef enum event_handler_error_codes
{
	EH_ERROR_CODE_NO_ERROR_CODE = 0,
	EH_ERROR_CODE_INVALID_EVENT,
	EH_ERROR_CODE_UNKNOWN_SENS_SERV,
	EH_ERROR_CODE_CORRUPT_ERROR_LIST,
	EH_ERROR_CODE_MAX_VALUE
}event_handler_error_codes_t;

static error_code_t	error_codes[ERROR_CODE_ARRAY_NUMBER][MAX_ERROR_CODES];
static uint8_t		error_code_number[ERROR_CODE_ARRAY_NUMBER];

static uint32_t	last_alert_code;
static uint64_t	last_alert_time;

static sensor_config_t* 	sensors_cfg;
static pl_event_t* 	 		system_events_group;
static uint32_t 			task_event_or_error_mask;
static pl_queue_t*			event_queue;

static void event_handler_new_error(uint32_t _error_index, error_code_t* _error_code);


static error_code_t eh_error_description[EH_ERROR_CODE_MAX_VALUE] =
{
	{CODE_NO_ERROR_CODE_SPECIFIED,	"Error code not specified trying to notify an alert."},
	{CODE_INVALID_EVENT_TYPE,		"Alert level not specified."},
	{CODE_INVALID_SENSOR_SERVICE,	"Invalid domain specified trying to give an alert"},
	{CODE_INVALID_SENSOR_SERVICE,	"Errors list has been corrupted, no error info available."}
};

void event_handler_init(sensor_config_t* _sensors_cfg, pl_event_t* _system_events_group, uint32_t _task_event_or_error_mask, pl_queue_t* _event_queue)
{
	sensors_cfg = _sensors_cfg;
	system_events_group = _system_events_group;
	task_event_or_error_mask = _task_event_or_error_mask;
	event_queue = _event_queue;

	for(uint32_t error_code_num_index = 0; error_code_num_index < ERROR_CODE_ARRAY_NUMBER; error_code_num_index++)
	{
		error_code_number[error_code_num_index] = 0;
	}

	last_alert_code = 0;
}

event_data_to_send_t event_handler_new_event_code(event_info_t* _event)
{
	event_data_to_send_t event_data;

	if((0 != _event->error_code->error_code_num) && (NULL != _event->error_code->error_code_description))
	{
		event_data.type = _event->event_type;

		if(EVENT_TYPE_ERROR != _event->event_type)
		{
			pl_time_t timestamp = pl_time_gettime();
			event_data.data.alert_data.time = pl_time_to_us(timestamp);
			if((last_alert_code == _event->error_code->error_code_num) &&
			   (1000000 > (event_data.data.alert_data.time - last_alert_time)))
			{
				event_data.type = EVENT_TYPE_NONE;
			}
			else
			{
				memset(event_data.data.alert_data.source, 0, 30);

				if(EVENT_TYPE_WARNING == _event->event_type)
				{
					memcpy(&event_data.data.alert_data.event_level[0], ALERT_LEVEL_WARNING, sizeof(ALERT_LEVEL_WARNING));
				}
				else if(EVENT_TYPE_INFO == _event->event_type)
				{
					memcpy(&event_data.data.alert_data.event_level[0], ALERT_LEVEL_INFO, sizeof(ALERT_LEVEL_INFO));
				}

				if(SENSOR_MAXVALUE == _event->sensor)
				{
					event_data.data.alert_data.domain = DOMAIN_SYSTEM;
					event_data.data.alert_data.error_code = _event->error_code;
				}
				else if ((SENSOR_MAXVALUE  > _event->sensor) && ((SERVICE_MAXVALUE  ==_event->service)))
				{
					event_data.data.alert_data.domain = DOMAIN_SENSOR;
					memcpy(event_data.data.alert_data.source, sensors_cfg[_event->sensor].sensor_name, strlen(sensors_cfg[_event->sensor].sensor_name));
					event_data.data.alert_data.error_code = _event->error_code;
				}
				else if ((SENSOR_MAXVALUE  > _event->sensor) && ((SERVICE_MAXVALUE  > _event->service)))
				{
					event_data.data.alert_data.domain = DOMAIN_SERVICE;
					memcpy(event_data.data.alert_data.source, sensors_cfg[_event->sensor].sensor_name, strlen(sensors_cfg[_event->sensor].sensor_name));
					strcat(event_data.data.alert_data.source, "/");
					strcat(event_data.data.alert_data.source, sensors_cfg[_event->sensor].sensor_services_config[_event->service]->service_name);
					event_data.data.alert_data.error_code = _event->error_code;
				}
				else
				{
					event_data.data.alert_data.domain = DOMAIN_SENSOR;
					event_data.data.alert_data.error_code = &eh_error_description[EH_ERROR_CODE_UNKNOWN_SENS_SERV];
				}

				last_alert_code = _event->error_code->error_code_num;
				last_alert_time = event_data.data.alert_data.time;
			}
		}
		else if (EVENT_TYPE_ERROR == _event->event_type)
		{
			if(SENSOR_MAXVALUE == _event->sensor)
			{
				event_handler_new_error(ERROR_CODES_SYSTEM, _event->error_code);

				event_data.data.state_change_data.state_type = DEVICE_STATE;
				event_data.data.state_change_data.state_info.device_info.status = DEVICE_STATUS_FAILURE;
				event_data.data.state_change_data.error_codes = error_codes[ERROR_CODES_SYSTEM];
				event_data.data.state_change_data.error_code_amount = error_code_number[ERROR_CODES_SYSTEM];
			}
			else if((SERVICE_MAXVALUE > _event->service) && (SENSOR_MAXVALUE  > _event->sensor))
			{
				event_handler_new_error(SENSOR_MAXVALUE + _event->service, _event->error_code);

				event_data.data.state_change_data.state_type = SERVICE_STATE;
				event_data.data.state_change_data.state_info.service_info.status = SERVICE_STATUS_FAILURE;
				event_data.data.state_change_data.state_info.service_info.sensor = _event->sensor;
				event_data.data.state_change_data.state_info.service_info.service = _event->service;
				event_data.data.state_change_data.error_codes = error_codes[SENSOR_MAXVALUE + _event->service];
				event_data.data.state_change_data.error_code_amount = error_code_number[SENSOR_MAXVALUE + _event->service];
			}
			else if ((SERVICE_MAXVALUE == _event->service) && (SENSOR_MAXVALUE  > _event->sensor))
			{
				event_handler_new_error(_event->sensor, _event->error_code);

				event_data.data.state_change_data.state_type = SENSOR_STATE;
				event_data.data.state_change_data.state_info.sensor_info.status = SENSOR_STATUS_FAILURE;
				event_data.data.state_change_data.state_info.sensor_info.sensor = _event->sensor;
				event_data.data.state_change_data.error_codes = error_codes[_event->sensor];
				event_data.data.state_change_data.error_code_amount = error_code_number[_event->sensor];
			}
			else
			{
				event_handler_new_error(ERROR_CODES_SYSTEM, &eh_error_description[EH_ERROR_CODE_UNKNOWN_SENS_SERV]);

				event_data.data.state_change_data.state_type = DEVICE_STATE;
				event_data.data.state_change_data.state_info.device_info.status = DEVICE_STATUS_FAILURE;
				event_data.data.state_change_data.error_codes = error_codes[ERROR_CODES_SYSTEM];
				event_data.data.state_change_data.error_code_amount = error_code_number[ERROR_CODES_SYSTEM];
			}
		}
		else
		{
			event_handler_new_error(ERROR_CODES_SYSTEM, &eh_error_description[EH_ERROR_CODE_INVALID_EVENT]);

			event_data.data.state_change_data.state_type = DEVICE_STATE;
			event_data.data.state_change_data.state_info.device_info.status = DEVICE_STATUS_FAILURE;
			event_data.data.state_change_data.error_codes = error_codes[ERROR_CODES_SYSTEM];
			event_data.data.state_change_data.error_code_amount = error_code_number[ERROR_CODES_SYSTEM];
		}
	}
	else
	{
		event_handler_new_error(ERROR_CODES_SYSTEM, &eh_error_description[EH_ERROR_CODE_NO_ERROR_CODE]);

		event_data.data.state_change_data.state_type = DEVICE_STATE;
		event_data.data.state_change_data.state_info.device_info.status = DEVICE_STATUS_FAILURE;
		event_data.data.state_change_data.error_codes = error_codes[ERROR_CODES_SYSTEM];
		event_data.data.state_change_data.error_code_amount = error_code_number[ERROR_CODES_SYSTEM];
	}

	return event_data;
}

error_code_t* event_handler_get_error_codes(sensors_t _sensor, services_t _service,  uint32_t* _error_amount)
{
	error_code_t* error_code;

	if(SENSOR_MAXVALUE == _sensor)
	{
		error_code = error_codes[ERROR_CODES_SYSTEM];
		*_error_amount = error_code_number[ERROR_CODES_SYSTEM];
	}
	else if((SERVICE_MAXVALUE > _service) && (SENSOR_MAXVALUE  > _sensor))
	{
		error_code = error_codes[SENSOR_MAXVALUE + _service];
		*_error_amount = error_code_number[SENSOR_MAXVALUE + _service];
	}
	else if ((SERVICE_MAXVALUE == _service) && (SENSOR_MAXVALUE  > _sensor))
	{
		error_code = error_codes[_sensor];
		*_error_amount = error_code_number[_sensor];
	}
	else
	{
		// THIS SHOULD NEVER HAPPEND. It should have a domain. will give a system error saying that error info was corrupted
		error_code =  &eh_error_description[EH_ERROR_CODE_CORRUPT_ERROR_LIST];
		*_error_amount = 1;
	}

	return error_code;
}


void event_handler_notify_system_event(void* _events_info, uint32_t _event)
{
	event_info_t* events_info = (event_info_t*)_events_info;

	if(EVENT_TYPE_NONE != events_info[_event].event_type)
	{
		pl_event_set(system_events_group, task_event_or_error_mask);
		pl_queue_send(event_queue, (void*)&events_info[_event], PL_QUEUE_NO_DELAY);
	}
}

static void event_handler_new_error(uint32_t _error_index, error_code_t* _error_code)
{
	error_code_t new_error_list[MAX_ERROR_CODES];

	memcpy(&new_error_list[1], error_codes[_error_index], (sizeof(error_code_t)*MAX_ERROR_CODES - sizeof(error_code_t)));
	new_error_list[0] = *_error_code;
	memcpy(error_codes[_error_index], new_error_list, sizeof(error_code_t)*MAX_ERROR_CODES);
	if(error_code_number[_error_index] < MAX_ERROR_CODES)
	{
		error_code_number[_error_index]++;
	}
}
