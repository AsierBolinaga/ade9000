/*
 * absl_socket.h
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_SOCKET_H_
#define ABSL_SOCKET_H_

#include "absl_config.h"
#ifdef ABSL_SOCKET

#include "absl_types.h"

#if defined(ABSL_OS_FREE_RTOS)
#include "absl_phy.h"
#include "absl_queue.h"
#include "absl_event.h"

#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#elif defined(ABSL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>

#define ABSL_SOCKET_ANY_IP_ADDR 	"0.0.0.0"
#endif



typedef enum absl_socket_rv
{
    ABSL_SOCKET_RV_OK = 0x0U,
	ABSL_SOCKET_RV_ERROR,
	ABSL_SOCKET_RV_NO_CONF,
	ABSL_SOCKET_RV_TIMEOUT,
	ABSL_SOCKET_RV_CONNECTING,
	ABSL_SOCKET_RV_CLOSING,
	ABSL_SOCKET_RV_DISCONNETED
} absl_socket_rv_t;

typedef enum absl_socket_protocol
{
    ABSL_SOCKET_PROTOCOL_UDP = 0x0U,
    ABSL_SOCKET_PROTOCOL_TCP,
    ABSL_SOCKET_PROTOCOL_UDP_TCP,
	ABSL_SOCKET_PROTOCOL_MQTT,
	ABSL_SOCKET_PROTOCOL_NONE
} absl_socket_protocol_t;

typedef enum absl_socket_mode
{
    ABSL_SOCKET_MODE_SERVER = 0x0U,
    ABSL_SOCKET_MODE_CLIENT
} absl_socket_mode_t;

typedef enum absl_socket_events
{
    ABSL_SOCKET_MESSAGE_RECEIVED     = 0x00000001,
    ABSL_SOCKET_MESSAGE_SEND         = 0x00000002,
	ABSL_SOCKET_ERROR 	           = 0x00000004,
	ABSL_SOCKET_TIMEOUT        	   = 0x00000008,
	ABSL_SOCKET_CONNECTED      	   = 0x00000010,
	ABSL_SOCKET_DISCONNECTED         = 0x00000020,
	ABSL_SOCKET_SUBSCRIBED      	   = 0x00000040
} absl_socket_events_t;

typedef struct absl_socket_config
{
#ifndef ABSL_LINUX
	absl_phy_config_t*			socket_phy;
#endif
	absl_socket_protocol_t		socket_protocol;	
	absl_socket_mode_t			socket_mode;
#ifdef ABSL_LINUX
	char*						remote_ip_address;
#endif
	uint16_t					src_port;
	uint8_t*					rx_buffer;
}absl_socket_config_t;

typedef struct absl_socket
{
#if defined(ABSL_OS_FREE_RTOS)
	uint8_t 				enet_phy;
	struct udp_pcb *		udp_pcb;
	struct netconn *		udp_conn;
	uint32_t				socket_udp;
	struct pbuf *			buf;
	struct netbuf *			udp_buf;
	struct sockaddr_in 		si_me;
	struct sockaddr_in 		si_other;
	struct tcp_pcb *		tcp_pcb;
	char 					received_data[50];
#elif defined(ABSL_LINUX)
	uint32_t				socket;
	in_addr_t				remote_address;
	struct sockaddr_in 		si_me;
	struct sockaddr_in 		si_other;
	uint32_t 				size_saddress;
#endif
	absl_socket_config_t*		socket_config;
	uint32_t				buffer_index;
	uint32_t				rx_length;
	uint32_t				rx_port;
	absl_event_t 				event_group;
	uint32_t				buffer_length;
	uint32_t				data_length;
	bool					tcp_sending_data;
}absl_socket_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_socket_init							absl_socket_init_freertos
#define absl_socket_create						absl_socket_create_freertos
#define absl_socket_set_event						absl_socket_set_event_freertos
#define absl_socket_bind							absl_socket_bind_freertos
#define absl_socket_check_ip						absl_socket_check_ip_freertos
#define absl_socket_connect						absl_socket_connect_freertos
#define absl_socket_reconnect						absl_socket_reconnect_freertos
#define absl_socket_tcp_close						absl_socket_tcp_close_freertos
#define absl_socket_udp_transfer					absl_socket_udp_transfer_freertos
#define absl_socket_tcp_transfer					absl_socket_tcp_transfer_freertos
#define absl_socked_get_received_data				absl_socked_get_received_data_freertos
#define absl_socket_get_remote_port				absl_socket_get_remote_port_freertos
#define absl_socket_get_mac						absl_socket_get_mac_freertos
#define absl_socket_tcp_check_state				absl_socket_tcp_check_state_freertos
#ifdef ABSL_EVENT
#define absl_socket_check							absl_socket_check_freertos
#endif
#elif defined(ABSL_LINUX)
#define absl_socket_init					absl_socket_init_linux
#define absl_socket_create				absl_socket_create_linux
#define absl_socket_bind					absl_socket_bind_linux	
#define absl_socket_connect				absl_socket_connect_linux
#define absl_socket_receive				absl_socket_receive_linux
#define absl_socket_transfer				absl_socket_transfer_linux
#else
#error Platform not defined
#endif

absl_socket_rv_t absl_socket_init(absl_socket_t * _socket, absl_socket_config_t* _socket_config);

void absl_socket_set_event(absl_socket_t* _socket, absl_event_t * _socket_event,
								  uint32_t _rx_flag, uint32_t	_tx_flag);

absl_socket_rv_t absl_socket_create(absl_socket_t * _socket);

absl_socket_rv_t absl_socket_bind(absl_socket_t * _socket);

bool absl_socket_check_ip(absl_socket_t * _socket, char* _ip);

absl_socket_rv_t absl_socket_connect(absl_socket_t * _socket, char* _ip, uint32_t _port);

absl_socket_rv_t absl_socket_reconnect(absl_socket_t * _socket, char* _ip, uint32_t _port);

void absl_socket_tcp_close(absl_socket_t * _socket);

absl_socket_rv_t absl_socket_receive(absl_socket_t * _socket, uint32_t* _rx_length);

uint32_t absl_socked_get_received_data(absl_socket_t * _socket, uint8_t* _rx_data);

absl_socket_rv_t absl_socket_udp_transfer(absl_socket_t * _socket, char* _send_buff, uint32_t _length, char* _ip, uint32_t _port);

absl_socket_rv_t absl_socket_tcp_transfer(absl_socket_t * _socket, char* _send_buff, uint32_t _length);

uint32_t absl_socket_get_remote_port(absl_socket_t * _socket);

char* absl_socket_get_mac(absl_socket_t * _socket);

#ifdef ABSL_EVENT
absl_socket_rv_t absl_socket_check(absl_socket_t * _socket, uint32_t* _socket_event);
#endif /* ABSL_EVENT */

absl_socket_rv_t absl_socket_tcp_check_state(absl_socket_t * _socket);

#endif /* ABSL_SOCKET */
#endif /* ABSL_SOCKET_H_ */
