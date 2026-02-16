/*
 * streaming_task.c
 *
 *  Created on: Mar 23, 2023
 *      Author: abolinaga
 */

#include "streaming_task.h"

#include "interfaces.h"

#include "absl_system.h"
#include "absl_debug.h"
#include "absl_timer.h"
#include "absl_macros.h"
#include "absl_hw_config.h"
#ifdef DEBUG_PIN
#include "absl_gpio.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STREAMING_NOT_SEND_AMOUNT_TO_ERROR  		20

#define STREAMING_TRYING_TO_READ_WITH_NO_PROTOCOL 	10

/*******************************************************************************
 * Variables
 ******************************************************************************/
static absl_timer_t	timer_reconnect_timeout;
static bool			reconnect_timeout;

static uint8_t		error_count;

#ifdef DEBUG_PIN
static absl_gpio_t            debug_gpio;
static absl_gpio_config_t*    debug_gpio_config;
#endif

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
static void streaming_send_udp(streaming_thread_data_t* _stream_thread_data, streaming_thread_config_t* _stream_thread_config, void* _data, stream_data_config_t _stream_config);
static void streaming_send_tcp(streaming_thread_data_t* _stream_thread_data, streaming_thread_config_t* _stream_thread_config, void* _data, stream_data_config_t _stream_config);

/*******************************************************************************
 * Code
 ******************************************************************************/
void streaming_task_reconnect_timeout(void* arg)
{
	ABSL_UNUSED_ARG(arg);

	reconnect_timeout = true;
}

bool streaming_task_initialize(streaming_thread_data_t* _streaming_task_data, streaming_thread_config_t* _streaming_task_config)
{
	bool return_value = false;

	absl_time_t reconnect_timeout_time;

	if(NULL != _streaming_task_config)
	{
		if((ABSL_EVENT_RV_OK == absl_event_create(_streaming_task_config->stream_events)) &&
		   (ABSL_QUEUE_RV_OK == absl_queue_create(_streaming_task_config->stream_data_queue, _streaming_task_config->queue_data_size,
				   	   	   	   	   	   	      _streaming_task_config->queue_data_amount)))
		{
			_streaming_task_data->socket_stream_config = absl_config_get_socket_conf(_streaming_task_config->stream_socket_index);
			if(ABSL_SOCKET_RV_OK == absl_socket_init(&_streaming_task_data->absl_socket_stream, _streaming_task_data->socket_stream_config))
			{
				_streaming_task_data->not_send_count = 0;

				_streaming_task_config->stream_initialized = true;

				reconnect_timeout_time.seconds = 0;
				reconnect_timeout_time.nseconds = 500000000;

				absl_timer_create(&timer_reconnect_timeout, &streaming_task_reconnect_timeout, NULL,
						        reconnect_timeout_time, false, false);

				error_count = 0;

#ifdef DEBUG_PIN
				debug_gpio_config = absl_config_get_gpio_conf(_streaming_task_config->debug_gpio_index);

				absl_gpio_init(&debug_gpio, debug_gpio_config, ABSL_GPIO_NO_INT);
				absl_gpio_off(&debug_gpio);
#endif

				return_value = true;
			}
			return return_value;
		}
	}

	return return_value;
}

/*!
 * @brief comm task entry point
 *
 * @param arg unused
 */
void streaming_task(void *arg)
{
	streaming_thread_t* streaming_thread = (streaming_thread_t*)arg;

	streaming_thread_config_t* 	stream_thread_config = streaming_thread->stream_thread_config;
	streaming_thread_data_t*	stream_thread_data = streaming_thread->stream_thread_data;

	uint32_t event_flags;

	if(!stream_thread_config->stream_initialized)
	{
		absl_debug_printf("ERROR! streaming thread has not been initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	absl_socket_create(&stream_thread_data->absl_socket_stream);

	while (1)
	{
		if(ABSL_EVENT_RV_OK == absl_event_wait(stream_thread_config->stream_events, STREAM_EVENTS, &event_flags))
		{
			if(STREAM_DATA == (event_flags & STREAM_DATA))
			{
				while(ABSL_QUEUE_RV_OK == absl_queue_receive(stream_thread_config->stream_data_queue, stream_thread_config->stream_data, ABSL_QUEUE_NO_DELAY))
				{
					if(PROTCOL_UDP == stream_thread_data->stream_config.protocol)
					{
						error_count = 0;
						streaming_send_udp(stream_thread_data, stream_thread_config, stream_thread_config->stream_data, stream_thread_data->stream_config);
					}
					else if(PROTCOL_TCP == stream_thread_data->stream_config.protocol)
					{
						error_count = 0;
						streaming_send_tcp(stream_thread_data, stream_thread_config, stream_thread_config->stream_data, stream_thread_data->stream_config);
					}
					else
					{
						if(error_count >= STREAMING_TRYING_TO_READ_WITH_NO_PROTOCOL)
						{
							streaming_task_notify_system_event(stream_thread_config->event_info_array, ST_EVENTS_PROTOCOL_NOT_DEFINED);
						}
						else
						{
							error_count++;
						}
					}
				}
			}
			if(STREAM_CONNECT == (event_flags & STREAM_CONNECT))
			{
				stream_thread_data->stream_config = stream_thread_config->stream_config_cb();

				if(true == absl_socket_check_ip(&stream_thread_data->absl_socket_stream, stream_thread_data->stream_config.ip))
				{
					if(PROTCOL_TCP == stream_thread_data->stream_config.protocol)
					{
						if(ABSL_SOCKET_RV_ERROR == absl_socket_connect(&stream_thread_data->absl_socket_stream,
																	stream_thread_data->stream_config.ip,
																	stream_thread_data->stream_config.port))
						{
							/* TODO - add retries */
							absl_event_set(stream_thread_config->service_event_group, stream_thread_config->could_not_connect_event);
							streaming_task_notify_system_event(stream_thread_config->event_info_array, ST_EVENTS_COULD_NOT_CONNECT_TCP);
						}
						else
						{
							absl_event_set(stream_thread_config->service_event_group, stream_thread_config->connected_event);
						}
					}
					else if(PROTCOL_UDP == stream_thread_data->stream_config.protocol)
					{
						absl_event_set(stream_thread_config->service_event_group, stream_thread_config->connected_event);
					}
					else
					{
						streaming_task_notify_system_event(stream_thread_config->event_info_array, ST_EVENTS_UNKNOWN_PROTOCOL);
					}
				}
				else
				{
					absl_event_set(stream_thread_config->service_event_group, stream_thread_config->could_not_connect_event);
					streaming_task_notify_system_event(stream_thread_config->event_info_array, ST_EVENTS_INCORRECT_IP);
				}
			}
			if(STREAM_DISCONNECT == (event_flags & STREAM_DISCONNECT))
			{
				if(PROTCOL_TCP == stream_thread_data->stream_config.protocol)
				{
					absl_socket_tcp_close(&stream_thread_data->absl_socket_stream);
				}
				else
				{
					/* Do nothing */
				}
			}
		}
	}
}

static void streaming_send_udp(streaming_thread_data_t* _stream_thread_data, streaming_thread_config_t* _stream_thread_config, void* _data, stream_data_config_t _stream_config)
{
	if(FORMAT_BINARY == _stream_config.format)
	{
		if(ABSL_SOCKET_RV_ERROR == absl_socket_udp_transfer(&_stream_thread_data->absl_socket_stream, (char*)_data,
													_stream_config.data_size, _stream_config.ip, _stream_config.port))
		{
			_stream_thread_data->not_send_count++;
			if(_stream_thread_data->not_send_count < STREAMING_NOT_SEND_AMOUNT_TO_ERROR)
			{
				streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_UDP_NOT_SEND);
			}
			else
			{
				streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_UDP_NOT_WORKING);
			}
		}
		else
		{
			_stream_thread_data->not_send_count = 0;
		}
	}
	else
	{
		streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_UNKNOWN_FORMAT);
	}
}

static void streaming_send_tcp(streaming_thread_data_t* _stream_thread_data, streaming_thread_config_t* _stream_thread_config, void* _data, stream_data_config_t _stream_config)
{
	uint32_t 		  data_length;

	absl_socket_rv_t tcp_socket = absl_socket_tcp_check_state(&_stream_thread_data->absl_socket_stream);

	switch(tcp_socket)
	{
		case ABSL_SOCKET_RV_OK:
		{
			_stream_thread_data->data_to_send.data_size = _stream_config.data_size;
			memcpy(_stream_thread_data->data_to_send.paylod, _data, _stream_config.data_size);
			data_length = _stream_config.data_size + 8;

			if(FORMAT_BINARY == _stream_config.format)
			{
				if(ABSL_SOCKET_RV_ERROR == absl_socket_tcp_transfer(&_stream_thread_data->absl_socket_stream, (char*)&_stream_thread_data->data_to_send,
																data_length))
				{
					_stream_thread_data->not_send_count++;
					if(_stream_thread_data->not_send_count < STREAMING_NOT_SEND_AMOUNT_TO_ERROR)
					{
						streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_TCP_NOT_SEND);
					}
					else
					{
						streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_TCP_NOT_WORKING);
					}
				}
				else
				{
					_stream_thread_data->not_send_count = 0;
				}
			}
			else
			{
				streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_UNKNOWN_FORMAT);
			}
			break;
		}
		case ABSL_SOCKET_RV_DISCONNETED:
			if(ABSL_SOCKET_RV_ERROR != absl_socket_reconnect(&_stream_thread_data->absl_socket_stream,
														  _stream_config.ip, _stream_config.port))
			{
				reconnect_timeout = false;
				absl_timer_start(&timer_reconnect_timeout);
			}
			else
			{
				streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_COULD_NOT_RECONNECT_TCP);
			}
			break;
		case ABSL_SOCKET_RV_CONNECTING:
			if(true == reconnect_timeout)
			{
				absl_socket_tcp_close(&_stream_thread_data->absl_socket_stream);
				streaming_task_notify_system_event(_stream_thread_config->event_info_array, ST_EVENTS_COULD_NOT_CONNECT_TCP);
			}
			break;
		case ABSL_SOCKET_RV_CLOSING:
			absl_socket_tcp_close(&_stream_thread_data->absl_socket_stream);
			break;
		default:
			break;
	}

}
