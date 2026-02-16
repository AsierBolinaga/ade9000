/*
 * absl_socket.h
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_MQTT_H_
#define ABSL_MQTT_H_

#include "absl_config.h"
#ifdef ABSL_MQTT

#include "absl_types.h"

#if defined(ABSL_OS_FREE_RTOS)
#include "absl_phy.h"
#include "absl_queue.h"
#include "absl_event.h"

#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"

#include "lwip/apps/mqtt.h"
#elif defined(ABSL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>

#define ABSL_SOCKET_ANY_IP_ADDR 	"0.0.0.0"
#endif

#define ABSL_MQTT_MSG_MAX_AMOUNT 		10

#define ABSL_MQTT_ID_MAX_LENGTH		40

#define ABSL_MQTT_TOPIC_MAX_LEN   	500
#define ABSL_MQTT_PAYLOAD_MAX_LEN 	2000

typedef enum absl_mqtt_rv
{
    ABSL_MQTT_RV_OK = 0x0U,
	ABSL_MQTT_RV_ERROR,
	ABSL_MQTT_RV_NO_CONF,
	ABSL_MQTT_RV_TIMEOUT,
	ABSL_MQTT_RV_DISCONNETED
} absl_mqtt_rv_t;

typedef enum absl_mqtt_events
{
    ABSL_MQTT_MESSAGE_RECEIVED   		= 0x00000001,
    ABSL_MQTT_MESSAGE_SEND       		= 0x00000002,
	ABSL_MQTT_ERROR 	           		= 0x00000004,
	ABSL_MQTT_TIMEOUT        	   		= 0x00000008,
	ABSL_MQTT_CONNECTED      	   		= 0x00000010,
	ABSL_MQTT_DISCONNECTED       		= 0x00000020,
	ABSL_MQTT_SUBSCRIBED         		= 0x00000040,
	ABSL_MQTT_SBUFFER_FULL       		= 0x00000080,
	ABSL_MQTT_NOT_CONNECTED      		= 0x00000100,
    ABSL_MQTT_MESSAGE_FRAME_RECEIVED  = 0x00000200
} absl_mqtt_events;


typedef struct absl_mqtt_topics
{
	char	 	topics[100];
	uint8_t  	qos;
}absl_mqtt_topics_t;

typedef struct absl_mqtt_config
{
	absl_phy_config_t*			mqtt_phy;
	uint16_t					src_port;
	char*						user;
	char*						password;
	uint32_t					keep_alive;
}absl_mqtt_config_t;

typedef struct
{
    char      		topic[ABSL_MQTT_TOPIC_MAX_LEN];
    uint16_t  		topic_len;
    bool			topic_obtained;
    uint8_t   		payload[ABSL_MQTT_PAYLOAD_MAX_LEN];
    uint16_t  		payload_len;
} mqtt_msg_t;

typedef struct absl_mqtt
{
#if defined(ABSL_OS_FREE_RTOS)
	uint8_t 				mqtt_phy;
	mqtt_client_t *			mqtt_client;
	char 					mqtt_client_id[ABSL_MQTT_ID_MAX_LENGTH];
	struct mqtt_connect_client_info_t mqtt_client_info;
	volatile bool 			mqtt_connected;
	ip4_addr_t 				netif_remote_addr;
	absl_mqtt_config_t*		mqtt_config;
	absl_event_t 				event_group;
	mqtt_msg_t				mqtt_buff[ABSL_MQTT_MSG_MAX_AMOUNT];
	volatile uint8_t 		msg_index_in;
	volatile uint8_t		msg_index_out;
	volatile uint8_t		msg_index_topic;
	volatile uint8_t 		msg_count;
	char      				last_topic[ABSL_MQTT_TOPIC_MAX_LEN];
	absl_mqtt_topics_t	    last_will_mqtt_topic;
	char*	    			last_will_msg;
	absl_event_t* 			config_event_group;
	uint32_t				rx_config_event;
	uint32_t				frame_rx_config_event;
	uint32_t				connect_timeout_event;
	uint32_t				server_disconnected_event;
	uint32_t				rx_full_buff_event;
	bool					config_event;
#endif
}absl_mqtt_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_mqtt_init							absl_mqtt_init_freertos
#define absl_mqtt_config_rx_event					absl_mqtt_config_rx_event_freertos
#define absl_mqtt_create							absl_mqtt_create_freertos
#define absl_mqtt_connect							absl_mqtt_connect_freertos
#define absl_mqtt_reconnect						absl_mqtt_reconnect_freertos
#define absl_mqtt_disconnect						absl_mqtt_disconnect_freertos
#define absl_mqtt_publish							absl_mqtt_publish_freertos
#define absl_mqtt_get_message						absl_mqtt_get_message_freertos
#define absl_mqtt_get_frame						absl_mqtt_get_frame_freertos
#define absl_mqtt_get_last_topic					absl_mqtt_get_last_topic_freertos
#define absl_mqtt_get_last_topic_length			absl_mqtt_get_last_topic_length_freertos
#define absl_mqtt_subscribe_topics				absl_mqtt_subscribe_topics_freertos
#define absl_mqtt_reset							absl_mqtt_reset_freertos
#define absl_mqtt_get_ip							absl_mqtt_get_ip_freertos
#define absl_mqtt_check_connection				absl_mqtt_check_connection_freertos
#else
#error Platform not defined
#endif

absl_mqtt_rv_t absl_mqtt_init(absl_mqtt_t * _mqtt, absl_mqtt_config_t* _mqtt_config);

void absl_mqtt_config_rx_event(absl_mqtt_t * _mqtt, absl_event_t* _event_group, uint32_t _rx_event,
							 uint32_t _frame_rx_event, uint32_t _server_disconnected,
							 uint32_t _connect_timeout_event, uint32_t _full_buffer_event);

absl_mqtt_rv_t absl_mqtt_create(absl_mqtt_t * _mqtt);

absl_mqtt_rv_t absl_mqtt_connect(absl_mqtt_t * _mqtt, char* _client_id, absl_mqtt_topics_t _last_will, char* _last_will_msg);

absl_mqtt_rv_t absl_mqtt_reconnect(absl_mqtt_t * _mqtt, char* _last_will_msg);

void absl_mqtt_disconnect(absl_mqtt_t * _mqtt);

absl_mqtt_rv_t absl_mqtt_subscribe_topics(absl_mqtt_t * _mqtt, absl_mqtt_topics_t* mqtt_topis, uint32_t _topic_num);

absl_mqtt_rv_t absl_mqtt_publish(absl_mqtt_t * _mqtt, char* _topic, char* _tx_buff, uint16_t _payload_length, uint8_t _qos, bool _retain);


uint32_t absl_mqtt_get_message(absl_mqtt_t * _mqtt, uint32_t* _topic_length, char* _topic, uint32_t* _msg_length, char* _rx_msg);

uint32_t absl_mqtt_get_frame(absl_mqtt_t * _mqtt, char* _rx_msg);

char* absl_mqtt_get_last_topic(absl_mqtt_t* _mqtt);

uint32_t absl_mqtt_get_last_topic_length(absl_mqtt_t * _mqtt);

void absl_mqtt_reset(absl_mqtt_t * _mqtt);

char* absl_mqtt_get_ip(absl_mqtt_t * _mqtt);

absl_mqtt_rv_t absl_mqtt_check_connection(absl_mqtt_t * _mqtt);

#endif /* ABSL_MQTT */
#endif /* ABSL_MQTT_H_ */
