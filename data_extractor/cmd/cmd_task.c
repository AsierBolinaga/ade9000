/*
 * cmd_task.c
 *
 *  Created on: Mar 23, 2023
 *      Author: abolinaga
 */

#include "cmd_task.h"

#include "interfaces.h"

#include "absl_debug.h"
#include "absl_hw_config.h"
#include "absl_socket.h"
#include "absl_time.h"
#include "absl_thread.h"
#include "absl_system.h"
#include "absl_temperature.h"
#include "absl_timer.h"
#include "absl_macros.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CMD_STATE_CHNGE_DATA_RECEPCTION_TIMOUT_MS	 10000

/*******************************************************************************
 * Variables
 ******************************************************************************/
static cmd_thread_config_t* cmd_config;

static cmd_task_states_t 	cmd_state;

static char sensor[20];
static char service[20];

static bool first_time_connected;
static bool	connected_to_server;

static state_data_to_send_t		data_to_send;

static uint32_t	consistency_counter;

/*******************************************************************************
 * Private function prototypes
 ******************************************************************************/
static void cmd_task_idle_state(uint32_t _events);
static void cmd_task_connect_state(uint32_t _events);
static void cmd_task_normal_state(uint32_t _events);
static void cmd_task_fw_update_state(uint32_t _events);
static void cmd_task_error_state(uint32_t _events);

static void cmd_task_reconnect(void);
static void cmd_task_reboot_system(void);
static void cmd_task_timestamp_msg(void);
static void cmd_task_info_msg(void);
static void cmd_task_check_disconnected(uint32_t _events);
static void cmd_task_check_data_to_send(uint32_t _events);
static void cmd_task_check_update_frame(uint32_t _events);
static void cmd_task_get_sensor_index(uint32_t* _sensor_index);
static void cmd_task_get_sensor_service_index(uint32_t* _sensor_index, uint32_t* _service_index);
static void cmd_task_get_alert_and_send(void);
static void cmd_task_get_new_state_and_send(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
bool cmd_task_initialize(cmd_thread_config_t* _cmd_task_config)
{
	bool return_value = false;

	if(NULL != _cmd_task_config)
	{
		cmd_config = _cmd_task_config;

		if((ABSL_EVENT_RV_OK == absl_event_create(cmd_config->cmd_events)) &&
		   (ABSL_TEMPERATURE_RV_OK == absl_temperature_init()) &&
		   (ABSL_QUEUE_RV_OK == absl_queue_create(cmd_config->watchdog_queue, sizeof(consistency_counter), 1)))
		{
			if(true == cmd_protocol_init(cmd_config->protocol_config, cmd_config->cmd_events,
										CMD_RECEIVED,
										CMD_FRAME_RECEIVED,
										CMD_SERVER_TIMEOUT,
										CMD_SERVER_DISCONNECTED,
										CMD_BUFF_IS_FULL))
			{
				cmd_config->cmd_initialized = true;
				first_time_connected = false;
				connected_to_server = false;
				consistency_counter = 0;

				return_value = true;
			}
		}
	}

	return return_value;
}

/*!
 * @brief comm task entry point
 *
 * @param arg unused
 */
void cmd_task(void *arg)
{
	uint32_t	recieved_events;
	uint8_t* 	system_state = (uint8_t*)arg;

	if(!cmd_config->cmd_initialized)
	{
		absl_debug_printf("ERROR! cmd thread has not been initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	while (1)
	{
		if(ABSL_EVENT_RV_OK == absl_event_wait(cmd_config->cmd_events, CMD_EVENTS, &recieved_events))
		{
			cmd_state = cmd_config->sensor_to_task_state[*system_state];
			switch(cmd_state)
			{
			case CMD_TASK_STATE_IDLE:
				cmd_task_idle_state(recieved_events);
			break;

			case CMD_TASK_STATE_CONNECT:
				cmd_task_connect_state(recieved_events);
			break;

			case CMD_TASK_STATE_NORMAL:
				cmd_task_normal_state(recieved_events);
			break;

			case CMD_TASK_STATE_FW_UPDATE:
				cmd_task_fw_update_state(recieved_events);
			break;

			case CMD_TASK_STATE_ERROR:
				cmd_task_error_state(recieved_events);
			break;

			default:
				absl_hardfault_handler(UNKNOWN_SWITCH_CASE_ERROR);
			break;
			}

			if(CMD_WATCHDOG == (recieved_events & CMD_WATCHDOG))
			{
				consistency_counter++;
				absl_queue_send(cmd_config->watchdog_queue, (void*)&consistency_counter, 1);
			}
		}
		else
		{
			absl_hardfault_handler(UNKNOWN_EVENT_ERROR);
		}
	}
}

static void cmd_task_idle_state(uint32_t _events)
{
	cmd_task_check_disconnected(_events);
}

static void cmd_task_connect_state(uint32_t _events)
{
	if(CMD_CONNECT == (_events & CMD_CONNECT))
	{
		cmd_protocol_reset();
		if(false == connected_to_server)
		{
			if(false == first_time_connected)
			{
				if(true == cmd_protocol_connect())
				{
					first_time_connected = true;
					connected_to_server = true;
					absl_event_set(cmd_config->system_events, cmd_config->connected_event);
				}
				else
				{
					absl_event_set(cmd_config->system_events, cmd_config->no_server_event);
				}
			}
			else
			{
				cmd_task_reconnect();
			}
		}
		else
		{
			absl_event_set(cmd_config->system_events, cmd_config->connected_event);
		}
	}

	cmd_task_check_disconnected(_events);
}

static void cmd_task_normal_state(uint32_t _events)
{
	uint32_t		  	messages_left;

	if(CMD_SEND_ONLINE == (_events & CMD_SEND_ONLINE))
	{
#ifndef FABRICATION_TEST
			if(true == cmd_protocol_send_discovery())
			{
#endif
				cmd_task_get_new_state_and_send();
#ifndef FABRICATION_TEST
			}
			else
			{
				absl_hardfault_handler(ERROR_SENDING_MQTT);
			}
#endif
	}

	cmd_task_check_data_to_send(_events);

	if(CMD_RECEIVED == (_events & CMD_RECEIVED))
	{
			protocol_rv_t recieved_cmd = cmd_protocol_messages_to_get(sensor, service, &messages_left);

			switch(recieved_cmd)
			{
			case PROTOCOL_INVALID_MESSAGE:
				absl_debug_printf("Invalid command received!\n");
				cmd_protocol_reset();
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_CMD);
				break;
			case PROTOCOL_INVALID_CONFIG_MESSAGE:
				if(true == cmd_get_sensor_config())
				{
					cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_CONF_PAYLOAD_KEEP_CONFIG);
				}
				else
				{
					cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_CONF_PAYLOAD_NO_CONFIG);
				}
				break;
			case PROTOCOL_CONFIG_MSG_RECIEVED:
			{
				absl_debug_printf("Config command received!\n");
				uint32_t sensor = SENSOR_MAXVALUE;
				cmd_task_get_sensor_index(&sensor);
				if(sensor < SENSOR_MAXVALUE)
				{
					absl_event_set(cmd_config->sensor_event_groups[sensor], cmd_config->config_events[sensor]);
				}
				else
				{
					cmd_protocol_reset();
					cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_CMD);
				}
				break;
			}
			case PROTOCOL_RESET_MSG_RECIEVED:
				absl_debug_printf("Reset command received!\n");
				for(uint32_t sensor_index = 0; sensor_index < cmd_config->protocol_config->sensor_amount; sensor_index++)
				{
					absl_event_set(cmd_config->sensor_event_groups[sensor_index], cmd_config->reset_events[sensor_index]);
				}
				break;
			case PROTOCOL_SENSOR_RESET_MSG_RECIEVED:
			{
				absl_debug_printf("Sensor reset command received!\n");
				uint32_t sensor = SENSOR_MAXVALUE;
				cmd_task_get_sensor_index(&sensor);
				if(sensor < SENSOR_MAXVALUE)
				{
					absl_event_set(cmd_config->sensor_event_groups[sensor], cmd_config->reset_events[sensor]);
				}
				else
				{
					cmd_protocol_reset();
					cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_CMD);
				}
				break;
			}
			case PROTOCOL_REBOOT_MSG_RECIEVED:
				cmd_task_reboot_system();
				break;
			case PROTOCOL_INVALID_START_MESSAGE:
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_START_PAYLOAD);
				break;
			case PROTOCOL_START_CMD_RECIEVED:
			{
				absl_debug_printf("Start command received!\n");
				uint32_t sensor;
				uint32_t service;
				cmd_task_get_sensor_service_index(&sensor, &service);
				absl_event_set(cmd_config->sensor_event_groups[sensor], cmd_config->start_events[sensor][service]);
				break;
			}
			case PROTOCOL_STOP_CMD_RECIEVED:
			{
				absl_debug_printf("Stop command received!\n");
				uint32_t sensor;
				uint32_t service;
				cmd_task_get_sensor_service_index(&sensor, &service);
				absl_event_set(cmd_config->sensor_event_groups[sensor], cmd_config->stop_events[sensor][service]);
				break;
			}
			case PROTOCOL_INVALID_SYNC_MESSAGE:
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_SYNC_PAYLOAD);
				break;
			case PROTOCOL_TIME_SYNC_MSG_RECIEVED:
				absl_debug_printf("Time sync command received!\n");
				absl_event_set(cmd_config->timesync_events, cmd_config->sync_event);
				break;
			case PROTOCOL_GET_TIMESTAMP_MSG_RECIEVED:
				cmd_task_timestamp_msg();
				break;
			case PROTOCOL_GET_INFO_MSG_RECIEVED:
				cmd_task_info_msg();
				break;
			case PROTOCOL_UPDATE_MSG_RECIEVED:
				absl_debug_printf("Update command received!\n");
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_UPDATE_INCORRECT_STATE);
				break;
			case PROTOCOL_INVALID_MANUFACTUR_MESSAGE:
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INVALID_MANUFACTUR_PAYLOAD);
				break;
			case PROTOCOL_MANUFACTUR_MSG_RECIEVED:
				absl_debug_printf("Manufactur info received!\n");
				manufacturing_t received_manufactur_data = cmd_protocol_get_manufactur();
				if(true == cmd_protocol_manufactur_write(&received_manufactur_data))
				{
#ifdef FABRICATION_TEST
					cmd_protocol_send_written_manufacturing(true);
#else
					cmd_task_reboot_system();
#endif
				}
				else
				{
#ifdef FABRICATION_TEST
					cmd_protocol_send_written_manufacturing(false);
#else
					cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_INCORRECT_MANUFACTURING);
#endif
				}
				break;
#ifdef FABRICATION_TEST
			case PROTOCOL_ID_MSG_RECIEVED:
				uint32_t sensor_id;
				if(true == cmd_task_get_id(&sensor_id))
				{
					cmd_protocol_send_id(sensor_id);
				}
				else
				{
					cmd_protocol_send_id(0);
				}
				break;
			case PROTOCOL_IRQ0_MSG_RECIEVED:
				bool irq0_detected = cmd_task_detect_irq0();
				cmd_protocol_irq0_detected(irq0_detected);
				break;
			case PROTOCOL_RST_MSG_RECIEVED:
				bool reset_done = cmd_task_reset_sensor();
				cmd_protocol_reset_done(reset_done);
				break;
#endif
			case PROTOCOL_ERROR:
			default:
				cmd_protocol_reset();
				cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_ERROR_GETTING_CMD);
				break;
			}

		if(0 < messages_left)
		{
			/* There are more commands left, reactivate the reception event, to take care of them in the next thread execution */
			absl_event_set(cmd_config->cmd_events, CMD_RECEIVED);
			absl_thread_sleep(100); /* A small sleep time to give some time to the system to react to the last command */
		}
	}

	if(CMD_BUFF_IS_FULL == (_events & CMD_BUFF_IS_FULL))
	{
		cmd_task_notify_system_event(cmd_config->event_info_array, CMD_EVENTS_FULL_RX_BUFFER);
	}

	cmd_task_check_update_frame(_events);
	cmd_task_check_disconnected(_events);
}

static void cmd_task_fw_update_state(uint32_t _events)
{
	cmd_task_check_data_to_send(_events);

	if(CMD_RECEIVED == (_events & CMD_RECEIVED))
	{
		/* In update state, just update frames should be received, so frame type should be checked.
		 * CMD_RECEIVED means that the last frame was received the complete update command is received. */
		protocol_rv_t recieved_cmd = cmd_protocol_frame_type(sensor, service);

		if(PROTOCOL_UPDATE_MSG_RECIEVED == recieved_cmd)
		{
			absl_event_set(cmd_config->fw_update_events, cmd_config->fw_file_reception_finished);
		}
		else
		{
			/* Do nothing */
		}
	}

	cmd_task_check_update_frame(_events);
	cmd_task_check_disconnected(_events);
}


static void cmd_task_error_state(uint32_t _events)
{
	uint32_t messages_left = 0;
	
	/* depending on the error, commands could not be received, 
	   but in case it is possible, give the possibility to update,
	   reboot and to notice if it has been disconnected */
	cmd_task_check_data_to_send(_events);

	if(CMD_RECEIVED == (_events & CMD_RECEIVED))
	{
		do
		{
			protocol_rv_t recieved_cmd = cmd_protocol_messages_to_get(sensor, service, &messages_left);
			if(PROTOCOL_REBOOT_MSG_RECIEVED == recieved_cmd)
			{
				cmd_task_reboot_system();
			}
			else
			{
				/* Do nothing */
			}
		}while (0 < messages_left);
	}

	cmd_task_check_update_frame(_events);
	cmd_task_check_disconnected(_events);
}

static void cmd_task_get_sensor_index(uint32_t* _sensor_index)
{
	for(uint32_t sensor_index = 0; sensor_index < cmd_config->protocol_config->sensor_amount; sensor_index++)
	{
		if(!strcmp(sensor, cmd_config->protocol_config->sensors_config[sensor_index].sensor_name))
		{
			*_sensor_index = sensor_index;
		}
	}
}

static void cmd_task_get_sensor_service_index(uint32_t* _sensor_index, uint32_t* _service_index)
{
	for(uint32_t sensor_index = 0; sensor_index < cmd_config->protocol_config->sensor_amount; sensor_index++)
	{
		if(!strcmp(sensor, cmd_config->protocol_config->sensors_config[sensor_index].sensor_name))
		{
			*_sensor_index = sensor_index;
			for(uint32_t service_index = 0; service_index < cmd_config->protocol_config->sensors_config[sensor_index].service_amount; service_index++)
			{
				if(!strcmp(service, cmd_config->protocol_config->sensors_config[sensor_index].sensor_services_config[service_index]->service_name))
				{
					*_service_index = service_index;
				}
			}
		}
	}
}

static void cmd_task_reconnect(void)
{
	if(true == cmd_protocol_reconnect())
	{
		connected_to_server = true;
		absl_event_set(cmd_config->system_events, cmd_config->connected_event);
	}
	else
	{
		absl_event_set(cmd_config->system_events, cmd_config->no_server_event);
	}
}

static void cmd_task_reboot_system(void)
{
	absl_debug_printf("Reboot command received!\n");
	absl_system_reboot();
}

static void cmd_task_timestamp_msg(void)
{
	absl_time_t 	actual_time;
	uint64_t  	actual_time_us;
	
	absl_debug_printf("Get timestamp command received!\n");
	actual_time = absl_time_gettime();
	actual_time_us = absl_time_to_us(actual_time);
	cmd_protocol_send_timestamp(actual_time_us);
}

static void cmd_task_info_msg(void)
{
	absl_time_t 					actual_time;
	device_status_information_t	device_status_info;

	absl_debug_printf("info command received!\n");
	actual_time = absl_time_gettime();
	device_status_info.timestamp = absl_time_to_us(actual_time);
	device_status_info.ip_address = cmd_protocol_get_ip();
	device_status_info.temperature = absl_temperature_get();
	device_status_info.cpu_load = absl_system_supervisor_get_CPU_load();
	device_status_info.heap_usage = absl_system_supervisor_get_heap_usage();
	device_status_info.sync_interval = cmd_protocol_get_sync_interval();
	device_status_info.hw_version = cmd_config->hw_version;

	cmd_protocol_send_info(&device_status_info);
}

static void cmd_task_check_disconnected(uint32_t _events)
{
	if(CMD_SERVER_TIMEOUT == (_events & CMD_SERVER_TIMEOUT) ||
	  (CMD_SERVER_DISCONNECTED == (_events & CMD_SERVER_DISCONNECTED)))
	{
		connected_to_server = false;
		absl_event_set(cmd_config->system_events, cmd_config->disconnected_event);
		cmd_protocol_reset();
		cmd_protocol_disconnect();

		for(uint32_t sensor_index = 0; sensor_index < cmd_config->protocol_config->sensor_amount; sensor_index++)
		{
			absl_event_set(cmd_config->sensor_event_groups[sensor_index], cmd_config->reset_events[sensor_index]);
			for(uint32_t service_index = 0; service_index < cmd_config->protocol_config->sensors_config[sensor_index].service_amount; service_index++)
			{
				absl_event_set(cmd_config->sensor_event_groups[sensor_index], cmd_config->stop_events[sensor_index][service_index]);
			}
		}
	}

	if(CMD_SERVER_DISCONNECT == (_events & CMD_SERVER_DISCONNECT))
	{
		if(true == connected_to_server)
		{
			connected_to_server = false;
			cmd_protocol_reset();
			cmd_protocol_disconnect();

			for(uint32_t sensor_index = 0; sensor_index < cmd_config->protocol_config->sensor_amount; sensor_index++)
			{
				absl_event_set(cmd_config->sensor_event_groups[sensor_index], cmd_config->reset_events[sensor_index]);
				for(uint32_t service_index = 0; service_index < cmd_config->protocol_config->sensors_config[sensor_index].service_amount; service_index++)
				{
					absl_event_set(cmd_config->sensor_event_groups[sensor_index], cmd_config->stop_events[sensor_index][service_index]);
				}
			}
		}
	}
}

static void cmd_task_check_data_to_send(uint32_t _events)
{
	if(CMD_SEND_ALERT == (_events & CMD_SEND_ALERT))
	{
		cmd_task_get_alert_and_send();
	}
	if(CMD_SEND_STATUS_CHANGE == (_events & CMD_SEND_STATUS_CHANGE))
	{
		cmd_task_get_new_state_and_send();
	}
}

static void cmd_task_check_update_frame(uint32_t _events)
{
	if(CMD_FRAME_RECEIVED == (_events & CMD_FRAME_RECEIVED))
	{
		protocol_rv_t recieved_cmd = cmd_protocol_frame_type(sensor, service);

		switch(recieved_cmd)
		{
		case PROTOCOL_UPDATE_MSG_RECIEVED:
			absl_event_set(cmd_config->system_events, cmd_config->fw_update_event);
			absl_event_set(cmd_config->fw_update_events, cmd_config->fw_file_chunk_received);
		break;
		default:
		/* Do nothing */
		break;
		}
	}
}

static void cmd_task_get_alert_and_send(void)
{
	while(ABSL_QUEUE_RV_OK == absl_queue_receive(cmd_config->alets_queue, (void*)&data_to_send.alert_data, ABSL_QUEUE_NO_DELAY))
	{
		cmd_protocol_send_event(&data_to_send.alert_data);
	}
}

static void cmd_task_get_new_state_and_send(void)
{
	while(ABSL_QUEUE_RV_OK == absl_queue_receive(cmd_config->states_queue, (void*)&data_to_send.state_change_data, ABSL_QUEUE_NO_DELAY))
	{
		if(false == cmd_protocol_send_status(&data_to_send.state_change_data))
		{
			absl_hardfault_handler(ERROR_SENDING_MQTT);
		}
	}
}
