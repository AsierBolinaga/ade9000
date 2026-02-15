/*
 * pl_socket.h
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_SOCKET_H_
#define PL_SOCKET_H_

#include "pl_config.h"
#ifdef PL_SOCKET

#include "pl_types.h"

#if defined(PL_OS_FREE_RTOS)
#include "pl_phy.h"
#include "pl_queue.h"
#include "pl_event.h"

#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"
#elif defined(PL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>

#define PL_SOCKET_ANY_IP_ADDR 	"0.0.0.0"
#endif



typedef enum pl_socket_rv
{
    PL_SOCKET_RV_OK = 0x0U,
	PL_SOCKET_RV_ERROR,
	PL_SOCKET_RV_NO_CONF,
	PL_SOCKET_RV_TIMEOUT,
	PL_SOCKET_RV_CONNECTING,
	PL_SOCKET_RV_CLOSING,
	PL_SOCKET_RV_DISCONNETED
} pl_socket_rv_t;

typedef enum pl_socket_protocol
{
    PL_SOCKET_PROTOCOL_UDP = 0x0U,
    PL_SOCKET_PROTOCOL_TCP,
    PL_SOCKET_PROTOCOL_UDP_TCP,
	PL_SOCKET_PROTOCOL_MQTT,
	PL_SOCKET_PROTOCOL_NONE
} pl_socket_protocol_t;

typedef enum pl_socket_mode
{
    PL_SOCKET_MODE_SERVER = 0x0U,
    PL_SOCKET_MODE_CLIENT
} pl_socket_mode_t;

typedef enum pl_socket_events
{
    PL_SOCKET_MESSAGE_RECEIVED     = 0x00000001,
    PL_SOCKET_MESSAGE_SEND         = 0x00000002,
	PL_SOCKET_ERROR 	           = 0x00000004,
	PL_SOCKET_TIMEOUT        	   = 0x00000008,
	PL_SOCKET_CONNECTED      	   = 0x00000010,
	PL_SOCKET_DISCONNECTED         = 0x00000020,
	PL_SOCKET_SUBSCRIBED      	   = 0x00000040
} pl_socket_events_t;

typedef struct pl_socket_config
{
#ifndef PL_LINUX
	pl_phy_config_t*			socket_phy;
#endif
	pl_socket_protocol_t		socket_protocol;	
	pl_socket_mode_t			socket_mode;
#ifdef PL_LINUX
	char*						remote_ip_address;
#endif
	uint16_t					src_port;
	uint8_t*					rx_buffer;
}pl_socket_config_t;

typedef struct pl_socket
{
#if defined(PL_OS_FREE_RTOS)
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
#elif defined(PL_LINUX)
	uint32_t				socket;
	in_addr_t				remote_address;
	struct sockaddr_in 		si_me;
	struct sockaddr_in 		si_other;
	uint32_t 				size_saddress;
#endif
	pl_socket_config_t*		socket_config;
	uint32_t				buffer_index;
	uint32_t				rx_length;
	uint32_t				rx_port;
	pl_event_t 				event_group;
	uint32_t				buffer_length;
	uint32_t				data_length;
	bool					tcp_sending_data;
}pl_socket_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_socket_init							pl_socket_init_freertos
#define pl_socket_create						pl_socket_create_freertos
#define pl_socket_set_event						pl_socket_set_event_freertos
#define pl_socket_bind							pl_socket_bind_freertos
#define pl_socket_check_ip						pl_socket_check_ip_freertos
#define pl_socket_connect						pl_socket_connect_freertos
#define pl_socket_reconnect						pl_socket_reconnect_freertos
#define pl_socket_tcp_close						pl_socket_tcp_close_freertos
#define pl_socket_udp_transfer					pl_socket_udp_transfer_freertos
#define pl_socket_tcp_transfer					pl_socket_tcp_transfer_freertos
#define pl_socked_get_received_data				pl_socked_get_received_data_freertos
#define pl_socket_get_remote_port				pl_socket_get_remote_port_freertos
#define pl_socket_get_mac						pl_socket_get_mac_freertos
#define pl_socket_tcp_check_state				pl_socket_tcp_check_state_freertos
#ifdef PL_EVENT
#define pl_socket_check							pl_socket_check_freertos
#endif
#elif defined(PL_LINUX)
#define pl_socket_init					pl_socket_init_linux
#define pl_socket_create				pl_socket_create_linux
#define pl_socket_bind					pl_socket_bind_linux	
#define pl_socket_connect				pl_socket_connect_linux
#define pl_socket_receive				pl_socket_receive_linux
#define pl_socket_transfer				pl_socket_transfer_linux
#else
#error Platform not defined
#endif

pl_socket_rv_t pl_socket_init(pl_socket_t * _socket, pl_socket_config_t* _socket_config);

void pl_socket_set_event(pl_socket_t* _socket, pl_event_t * _socket_event,
								  uint32_t _rx_flag, uint32_t	_tx_flag);

pl_socket_rv_t pl_socket_create(pl_socket_t * _socket);

pl_socket_rv_t pl_socket_bind(pl_socket_t * _socket);

bool pl_socket_check_ip(pl_socket_t * _socket, char* _ip);

pl_socket_rv_t pl_socket_connect(pl_socket_t * _socket, char* _ip, uint32_t _port);

pl_socket_rv_t pl_socket_reconnect(pl_socket_t * _socket, char* _ip, uint32_t _port);

void pl_socket_tcp_close(pl_socket_t * _socket);

pl_socket_rv_t pl_socket_receive(pl_socket_t * _socket, uint32_t* _rx_length);

uint32_t pl_socked_get_received_data(pl_socket_t * _socket, uint8_t* _rx_data);

pl_socket_rv_t pl_socket_udp_transfer(pl_socket_t * _socket, char* _send_buff, uint32_t _length, char* _ip, uint32_t _port);

pl_socket_rv_t pl_socket_tcp_transfer(pl_socket_t * _socket, char* _send_buff, uint32_t _length);

uint32_t pl_socket_get_remote_port(pl_socket_t * _socket);

char* pl_socket_get_mac(pl_socket_t * _socket);

#ifdef PL_EVENT
pl_socket_rv_t pl_socket_check(pl_socket_t * _socket, uint32_t* _socket_event);
#endif /* PL_EVENT */

pl_socket_rv_t pl_socket_tcp_check_state(pl_socket_t * _socket);

#endif /* PL_SOCKET */
#endif /* PL_SOCKET_H_ */
