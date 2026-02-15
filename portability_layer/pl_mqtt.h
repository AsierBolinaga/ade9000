/*
 * pl_socket.h
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_MQTT_H_
#define PL_MQTT_H_

#include "pl_config.h"
#ifdef PL_MQTT

#include "pl_types.h"

#if defined(PL_OS_FREE_RTOS)
#include "pl_phy.h"
#include "pl_queue.h"
#include "pl_event.h"

#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"

#include "lwip/apps/mqtt.h"
#include "fsl_silicon_id.h"
#elif defined(PL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>

#define PL_SOCKET_ANY_IP_ADDR 	"0.0.0.0"
#endif

#define PL_MQTT_MSG_MAX_AMOUNT 		10

typedef enum pl_mqtt_rv
{
    PL_MQTT_RV_OK = 0x0U,
	PL_MQTT_RV_ERROR,
	PL_MQTT_RV_NO_CONF,
	PL_MQTT_RV_TIMEOUT,
	PL_MQTT_RV_DISCONNETED
} pl_mqtt_rv_t;

typedef enum pl_mqtt_events
{
    PL_MQTT_MESSAGE_RECEIVED   		= 0x00000001,
    PL_MQTT_MESSAGE_SEND       		= 0x00000002,
	PL_MQTT_ERROR 	           		= 0x00000004,
	PL_MQTT_TIMEOUT        	   		= 0x00000008,
	PL_MQTT_CONNECTED      	   		= 0x00000010,
	PL_MQTT_DISCONNECTED       		= 0x00000020,
	PL_MQTT_SUBSCRIBED         		= 0x00000040,
	PL_MQTT_SBUFFER_FULL       		= 0x00000080,
	PL_MQTT_NOT_CONNECTED      		= 0x00000100,
    PL_MQTT_MESSAGE_FRAME_RECEIVED  = 0x00000200
} pl_mqtt_events;


typedef struct pl_mqtt_topics
{
	char	 	topics[100];
	uint8_t  	qos;
}pl_mqtt_topics_t;

typedef struct pl_mqtt_config
{
	pl_phy_config_t*			mqtt_phy;
	uint16_t					src_port;
	uint32_t					rx_buffer_length;
	uint8_t*					rx_buffer;
	char*						user;
	char*						password;
	uint32_t					keep_alive;
}pl_mqtt_config_t;


typedef struct pl_mqtt
{
#if defined(PL_OS_FREE_RTOS)
	uint8_t 				mqtt_phy;
	mqtt_client_t *			mqtt_client;
	char 					mqtt_client_id[(SILICONID_MAX_LENGTH * 2) + 5];
	struct mqtt_connect_client_info_t mqtt_client_info;
	volatile bool 			mqtt_connected;
	ip4_addr_t 				netif_remote_addr;
	pl_mqtt_config_t*		mqtt_config;
	volatile uint32_t		buffer_index;
	pl_event_t 				event_group;
	char					mqtt_topic[300];
	volatile int32_t		mqtt_topic_in_buff;
	volatile int32_t		topic_buff_index;
	uint32_t				buffer_length;
	volatile uint32_t		topic_lengths[PL_MQTT_MSG_MAX_AMOUNT];
	volatile int32_t		mqtt_msg_in_buff;
	volatile uint32_t		mqtt_msg_lengths[PL_MQTT_MSG_MAX_AMOUNT];
	pl_mqtt_topics_t	    last_will_mqtt_topic;
	char*	    			last_will_msg;
	pl_event_t* 			config_event_group;
	uint32_t				rx_config_event;
	uint32_t				frame_rx_config_event;
	uint32_t				connect_timeout_event;
	uint32_t				server_disconnected_event;
	uint32_t				rx_full_buff_event;
	bool					config_event;
#endif
}pl_mqtt_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_mqtt_init							pl_mqtt_init_freertos
#define pl_mqtt_config_rx_event					pl_mqtt_config_rx_event_freertos
#define pl_mqtt_create							pl_mqtt_create_freertos
#define pl_mqtt_connect							pl_mqtt_connect_freertos
#define pl_mqtt_reconnect						pl_mqtt_reconnect_freertos
#define pl_mqtt_disconnect						pl_mqtt_disconnect_freertos
#define pl_mqtt_publish							pl_mqtt_publish_freertos
#define pl_mqtt_wait_messages					pl_mqtt_wait_messages_freertos
#define pl_mqtt_get_message						pl_mqtt_get_message_freertos
#define pl_mqtt_get_frame						pl_mqtt_get_frame_freertos
#define pl_mqtt_get_last_topic					pl_mqtt_get_last_topic_freertos
#define pl_mqtt_get_last_topic_length			pl_mqtt_get_last_topic_length_freertos
#define pl_mqtt_subscribe_topics				pl_mqtt_subscribe_topics_freertos
#define pl_mqtt_reset							pl_mqtt_reset_freertos
#define pl_mqtt_get_ip							pl_mqtt_get_ip_freertos
#define pl_mqtt_check_connection				pl_mqtt_check_connection_freertos
#else
#error Platform not defined
#endif

pl_mqtt_rv_t pl_mqtt_init(pl_mqtt_t * _mqtt, pl_mqtt_config_t* _mqtt_config);

void pl_mqtt_config_rx_event(pl_mqtt_t * _mqtt, pl_event_t* _event_group, uint32_t _rx_event,
							 uint32_t _frame_rx_event, uint32_t _server_disconnected,
							 uint32_t _connect_timeout_event, uint32_t _full_buffer_event);

pl_mqtt_rv_t pl_mqtt_create(pl_mqtt_t * _mqtt);

pl_mqtt_rv_t pl_mqtt_connect(pl_mqtt_t * _mqtt, char* _client_id, pl_mqtt_topics_t _last_will, char* _last_will_msg);

pl_mqtt_rv_t pl_mqtt_reconnect(pl_mqtt_t * _mqtt, char* _last_will_msg);

void pl_mqtt_disconnect(pl_mqtt_t * _mqtt);

pl_mqtt_rv_t pl_mqtt_subscribe_topics(pl_mqtt_t * _mqtt, pl_mqtt_topics_t* mqtt_topis, uint32_t _topic_num);

pl_mqtt_rv_t pl_mqtt_publish(pl_mqtt_t * _mqtt, char* _topic, char* _tx_buff, uint16_t _payload_length, uint8_t _qos, bool _retain);

pl_mqtt_rv_t pl_mqtt_wait_messages(pl_mqtt_t * _mqtt, uint32_t *msgs, uint32_t _timeout_ms);

uint32_t pl_mqtt_get_message(pl_mqtt_t * _mqtt, uint32_t* _topic_length, char* _topic, uint32_t* _msg_length, char* _rx_msg);

uint32_t pl_mqtt_get_frame(pl_mqtt_t * _mqtt, char* _rx_msg);

char* pl_mqtt_get_last_topic(pl_mqtt_t* _mqtt);

uint32_t pl_mqtt_get_last_topic_length(pl_mqtt_t * _mqtt);

void pl_mqtt_reset(pl_mqtt_t * _mqtt);

char* pl_mqtt_get_ip(pl_mqtt_t * _mqtt);

pl_mqtt_rv_t pl_mqtt_check_connection(pl_mqtt_t * _mqtt);

#endif /* PL_MQTT */
#endif /* PL_MQTT_H_ */
