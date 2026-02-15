/*
 * comm_task.h
 *
 *  Created on: Mar 23, 2023
 *      Author: abolinaga
 */

#ifndef STREAMING_TASK_H_
#define STREAMING_TASK_H_
/******************************************************************************
 * Includes
 ******************************************************************************/
#include "streaming_types.h"

#include "pl_socket.h"
#include "pl_queue.h"
#include "pl_event.h"

/******************************************************************************
 * Defines
 ******************************************************************************/
#define STREAM_DATA			0x00000001
#define STREAM_CONNECT		0x00000002
#define STREAM_DISCONNECT	0x00000004

#define STREAM_EVENTS	STREAM_DATA			| \
						STREAM_CONNECT 		| \
						STREAM_DISCONNECT 		

typedef enum streaming_events
{
	ST_EVENTS_UNKNOWN_PROTOCOL = 0,
	ST_EVENTS_UNKNOWN_FORMAT,
	ST_EVENTS_UDP_NOT_SEND,
	ST_EVENTS_UDP_NOT_WORKING,
	ST_EVENTS_TCP_NOT_SEND,
	ST_EVENTS_TCP_NOT_WORKING,
	ST_EVENTS_PROTOCOL_NOT_DEFINED,
	ST_EVENTS_COULD_NOT_CONNECT_TCP,
	ST_EVENTS_COULD_NOT_RECONNECT_TCP,
	ST_EVENTS_INCORRECT_IP,
	ST_EVENTS_MAXVALUE
}streaming_events_t;

typedef struct tcp_stream_data
{
	uint64_t	data_size;
	char		paylod[10000];
}tcp_stream_data_t;

/******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef struct streaming_thread_data
{
 	pl_socket_t				pl_socket_stream;
 	pl_socket_config_t* 	socket_stream_config;
 	uint32_t				not_send_count;
 	stream_data_config_t  	stream_config;
 	tcp_stream_data_t		data_to_send;
}streaming_thread_data_t;

typedef struct streaming_thread_config
{
	uint32_t	 				stream_socket_index;
	pl_event_t*					stream_events;
	pl_queue_t*					stream_data_queue;
	pl_event_t*					service_event_group;
	uint32_t					connected_event;
	uint32_t					could_not_connect_event;
	uint32_t					queue_data_size;
	uint32_t					queue_data_amount;
	stream_data_config_t 		(*stream_config_cb)(void);
	void* 						stream_data;
#ifdef DEBUG_PIN
	uint8_t						debug_gpio_index;
#endif
	void* 						event_info_array;
	bool						stream_initialized;
}streaming_thread_config_t;

typedef struct streaming_thread
{
	streaming_thread_data_t*	stream_thread_data;
	streaming_thread_config_t* 	stream_thread_config;
}streaming_thread_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
bool streaming_task_initialize(streaming_thread_data_t* _streaming_task_data, streaming_thread_config_t* _streaming_task_config);

void streaming_task(void *arg);


#endif /* STREAMING_TASK_H_ */
