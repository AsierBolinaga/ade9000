/*
 * absl_socket.c
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_socket.h"
#ifdef ABSL_SOCKET
#include "absl_macros.h"

#if defined(ABSL_OS_FREE_RTOS)
#if !defined(ABSL_EVENT)
#error "absl_socket module for freertos uses event, ABSL_EVENT needs to be active in absl_config.h file."
#else
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
#include "lwip/api.h"

#include "absl_thread.h"
#include "absl_debug.h"

#define ABSL_SOCKET_MAX_AMOUNT 		3
#define ABSL_SOCKET_MAX_FCN_AMOUNT	5

#define SOCKET_EVENTS 	ABSL_SOCKET_MESSAGE_RECEIVED | ABSL_SOCKET_MESSAGE_SEND | \
                        ABSL_SOCKET_ERROR    | ABSL_SOCKET_TIMEOUT

#ifdef ABSL_EVENT
static err_t absl_socket_client_connected(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t absl_socket_accept_tcp_freertos(void *_arg, struct tcp_pcb *_pcb, err_t _err);
static err_t absl_socket_receive_tcp_freertos(void *_arg, struct tcp_pcb *_tpcb, struct pbuf *_pbuff, err_t _err);
static err_t absl_socket_sent_tcp_freertos(void *_arg, struct tcp_pcb *_pcb, u16_t _len);
static err_t absl_socket_poll_tcp_freertos(void *_arg, struct tcp_pcb *_tpcb);
#endif
static void absl_socket_error_freertos(void *_arg, err_t _err);

void absl_socket_udp_rcv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	absl_socket_t* socket = (absl_socket_t*)arg;

	ABSL_UNUSED_ARG(pcb);
	ABSL_UNUSED_ARG(addr);

	memcpy(socket->socket_config->rx_buffer, p->payload, p->len);
	socket->rx_length = p->len;
	socket->rx_port = port;

	absl_event_set_fromISR(&socket->event_group, ABSL_SOCKET_MESSAGE_RECEIVED);

	pbuf_free(p);
}

absl_socket_rv_t absl_socket_init_freertos(absl_socket_t * _socket, absl_socket_config_t* _socket_config)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	if(NULL!= _socket_config)
	{
		_socket->socket_config = _socket_config;

		if(ABSL_PHY_RV_OK == absl_phy_init(&_socket->enet_phy, _socket_config->socket_phy))
		{
			absl_event_create(&_socket->event_group);

			_socket->buffer_index = 0;
			_socket->tcp_sending_data = false;
			socket_rv = ABSL_SOCKET_RV_OK;
		}
	}
	else
	{
		socket_rv = ABSL_SOCKET_RV_NO_CONF;
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_create_freertos(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	absl_phy_t* phy = absl_phy_get_object(_socket->enet_phy);

	if(ABSL_SOCKET_PROTOCOL_UDP == _socket->socket_config->socket_protocol)
	{
		LOCK_TCPIP_CORE();

		_socket->udp_pcb = udp_new();

		if(ABSL_SOCKET_MODE_CLIENT == _socket->socket_config->socket_mode)
		{
			if(ERR_OK == udp_bind(_socket->udp_pcb, IP_ADDR_ANY, _socket->socket_config->src_port))
			{
				udp_recv(_socket->udp_pcb, absl_socket_udp_rcv_cb, (void*)_socket);
				socket_rv = ABSL_SOCKET_RV_OK;
			}
		}
		else
		{
			/* Implement server part */
		}
		UNLOCK_TCPIP_CORE();
	}
	else if(ABSL_SOCKET_PROTOCOL_TCP  == _socket->socket_config->socket_protocol)
	{
#ifdef ABSL_EVENT
		tcp_accept(_socket->tcp_pcb, absl_socket_accept_tcp_freertos);
#endif
		_socket->tcp_pcb = tcp_new();
		tcp_bind(_socket->tcp_pcb, &phy->netif_local_addr, 3000);
		_socket->tcp_pcb = tcp_listen(_socket->tcp_pcb);
		tcp_arg(_socket->tcp_pcb,  (void*)_socket);
		socket_rv = ABSL_SOCKET_RV_OK;
	}
	else if(ABSL_SOCKET_PROTOCOL_UDP_TCP  == _socket->socket_config->socket_protocol)
	{
		LOCK_TCPIP_CORE();

		_socket->udp_pcb = udp_new();
		if(ABSL_SOCKET_MODE_CLIENT == _socket->socket_config->socket_mode)
		{
			if(ERR_OK == udp_bind(_socket->udp_pcb, IP_ADDR_ANY, _socket->socket_config->src_port))
			{
				udp_recv(_socket->udp_pcb, absl_socket_udp_rcv_cb, (void*)_socket);
			}
		}
		else
		{
			/* Implement server part */
		}

		_socket->tcp_pcb = tcp_new();

		tcp_bind(_socket->tcp_pcb, IP_ADDR_ANY, _socket->socket_config->src_port);

		tcp_arg(_socket->tcp_pcb, _socket);

		tcp_err(_socket->tcp_pcb,  absl_socket_error_freertos);
		tcp_sent(_socket->tcp_pcb, absl_socket_sent_tcp_freertos);
		tcp_recv(_socket->tcp_pcb, absl_socket_receive_tcp_freertos);
		tcp_poll(_socket->tcp_pcb, absl_socket_poll_tcp_freertos, 10);

		UNLOCK_TCPIP_CORE();

		socket_rv = ABSL_SOCKET_RV_OK;

	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_bind(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	_socket->si_me.sin_family = AF_INET;
	_socket->si_me.sin_port = htons(_socket->socket_config->src_port);
	_socket->si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if(0 <= bind(_socket->socket_udp, (struct sockaddr *)&_socket->si_me, sizeof(struct sockaddr_in)))
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}

bool absl_socket_check_ip_freertos(absl_socket_t* _socket, char* _ip)
{
	bool valid_ip = false;

	absl_phy_t* phy =  absl_phy_get_object(_socket->enet_phy);

	ip4_addr_t 	remote_addr;
	ip4_addr_t 	locar_host;
	ip4addr_aton(_ip, &remote_addr);
	ip4addr_aton("127.0.0.1", &locar_host);

	if((locar_host.addr != remote_addr.addr) && (phy->netif_local_addr.addr != remote_addr.addr))
	{
		valid_ip = true;
	}

	return valid_ip;
}

absl_socket_rv_t absl_socket_connect_freertos(absl_socket_t * _socket, char* _ip, uint32_t _port)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	uint32_t	event;
	ip4_addr_t 	remote_addr;
	ip4addr_aton(_ip, &remote_addr);

	LOCK_TCPIP_CORE();

	if(ERR_OK == tcp_connect(_socket->tcp_pcb, &remote_addr, _port, absl_socket_client_connected))
	{
		UNLOCK_TCPIP_CORE();

		if(ABSL_EVENT_RV_OK == absl_event_timed_wait(&_socket->event_group, ABSL_SOCKET_CONNECTED, &event, 15000))
		{
			socket_rv = ABSL_SOCKET_RV_OK;
		}
		else
		{
			absl_socket_tcp_close_freertos(_socket);
		}
	}
	else
	{
		UNLOCK_TCPIP_CORE();
		absl_socket_tcp_close_freertos(_socket);
		socket_rv = absl_socket_connect_freertos(_socket, _ip, _port);
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_reconnect_freertos(absl_socket_t * _socket, char* _ip, uint32_t _port)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	uint32_t	event;
	ip4_addr_t 	remote_addr;
	ip4addr_aton(_ip, &remote_addr);

	LOCK_TCPIP_CORE();

	if(ERR_OK == tcp_connect(_socket->tcp_pcb, &remote_addr, _port, absl_socket_client_connected))
	{
		UNLOCK_TCPIP_CORE();

		if(ABSL_EVENT_RV_OK == absl_event_timed_wait(&_socket->event_group, ABSL_SOCKET_CONNECTED, &event, 0))
		{
			socket_rv = ABSL_SOCKET_RV_OK;
		}
	}
	else
	{
		UNLOCK_TCPIP_CORE();
	}

	return socket_rv;
}

void absl_socket_tcp_close_freertos(absl_socket_t * _socket)
{
	LOCK_TCPIP_CORE();

	tcp_abort(_socket->tcp_pcb);

	_socket->tcp_pcb = tcp_new();

	tcp_bind(_socket->tcp_pcb, IP_ADDR_ANY, _socket->socket_config->src_port);

	tcp_arg(_socket->tcp_pcb, _socket);

	tcp_err(_socket->tcp_pcb,  absl_socket_error_freertos);
	tcp_sent(_socket->tcp_pcb, absl_socket_sent_tcp_freertos);
	tcp_recv(_socket->tcp_pcb, absl_socket_receive_tcp_freertos);
	tcp_poll(_socket->tcp_pcb, absl_socket_poll_tcp_freertos, 10);

	UNLOCK_TCPIP_CORE();
}

absl_socket_rv_t absl_socket_receive_freertos(absl_socket_t * _socket, uint32_t* _rx_length)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	ABSL_UNUSED_ARG(_rx_length);

	uint32_t socket_events;
	absl_event_wait(&_socket->event_group, ABSL_SOCKET_MESSAGE_RECEIVED, &socket_events);

	if(ABSL_SOCKET_MESSAGE_RECEIVED == (socket_events & ABSL_SOCKET_MESSAGE_RECEIVED) &&
	   (_socket->socket_config->socket_protocol == ABSL_SOCKET_PROTOCOL_UDP))
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}
	else
	{
		/* Todo - implement the rest of protocols */
	}


	return socket_rv;
}

uint32_t absl_socked_get_received_data_freertos(absl_socket_t * _socket, uint8_t* _rx_data)
{
	uint32_t length = 0;

	if(_socket->socket_config->socket_protocol == ABSL_SOCKET_PROTOCOL_UDP)
	{
		/* Todo - implement UDP rx buff handling */
	}
	else if(_socket->socket_config->socket_protocol == ABSL_SOCKET_PROTOCOL_MQTT)
	{
		memcpy(_rx_data, _socket->socket_config->rx_buffer, _socket->rx_length);
		length = _socket->rx_length;
		_socket->rx_length= 0;
	}
	else
	{
		/* Todo - implement the rest of protocols */
	}

	return length;
}

absl_socket_rv_t absl_socket_udp_transfer_freertos(absl_socket_t * _socket, char* _send_buff, uint32_t _length,
										char* _ip, uint32_t _port)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	ip4_addr_t 	remote_addr;

	ip4addr_aton(_ip, &remote_addr);

	LOCK_TCPIP_CORE();
	_socket->buf = pbuf_alloc(PBUF_TRANSPORT, _length, PBUF_RAM);
	pbuf_take(_socket->buf, _send_buff, _length);
	err_t udp_err = udp_sendto(_socket->udp_pcb, _socket->buf, &remote_addr, _port);
	UNLOCK_TCPIP_CORE();
	pbuf_free(_socket->buf);

	if(ERR_OK == udp_err)
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_tcp_transfer_freertos(absl_socket_t * _socket, char* _send_buff, uint32_t _length)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	err_t 		tcp_error;

	_socket->data_length = _length;
	LOCK_TCPIP_CORE();

	tcp_error = tcp_write(_socket->tcp_pcb, _send_buff, _length, TCP_WRITE_FLAG_COPY);

	if (ERR_OK == tcp_error)
	{
		tcp_error = tcp_output(_socket->tcp_pcb);

		if(ERR_OK == tcp_error)
		{
			_socket->tcp_sending_data = true;
			UNLOCK_TCPIP_CORE();

			socket_rv = ABSL_SOCKET_RV_OK;
		}
		else
		{
			UNLOCK_TCPIP_CORE();
		}
	}
	else
	{
		UNLOCK_TCPIP_CORE();
	}

	return socket_rv;
}

uint32_t absl_socket_get_remote_port(absl_socket_t * _socket)
{
	return _socket->rx_port;
}

absl_socket_rv_t absl_socket_tcp_check_state_freertos(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	switch(_socket->tcp_pcb->state)
	{
	case ESTABLISHED:
		socket_rv = ABSL_SOCKET_RV_OK;
		break;
	case SYN_SENT:
	case SYN_RCVD:
		socket_rv = ABSL_SOCKET_RV_CONNECTING;
		break;
	case CLOSED:
		socket_rv = ABSL_SOCKET_RV_DISCONNETED;
		break;
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case CLOSE_WAIT:
	case CLOSING:
		socket_rv = ABSL_SOCKET_RV_CLOSING;
		break;
	default:
		break;
	}

	return socket_rv;
}

#ifdef ABSL_EVENT
absl_socket_rv_t absl_socket_check_freertos(absl_socket_t * _socket, uint32_t* _socket_event)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_NO_CONF;

	*_socket_event = 0;

	if(ABSL_EVENT_RV_OK == absl_event_timed_wait_freertos(&_socket->event_group, SOCKET_EVENTS, _socket_event, 0))
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}

static err_t absl_socket_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
	absl_socket_t* absl_socket = (absl_socket_t*)arg;

 	if (err == ERR_OK)
	{
 		while(ESTABLISHED != absl_socket->tcp_pcb->state);
		absl_event_set_fromISR(&absl_socket->event_group, ABSL_SOCKET_CONNECTED);
	}
	else
	{
		absl_event_set_fromISR(&absl_socket->event_group, ABSL_SOCKET_ERROR);
		tcp_arg(pcb, NULL);
		tcp_sent(pcb, NULL);
		tcp_abort(pcb);
	}

	return err;
}

static err_t absl_socket_accept_tcp_freertos (void *_arg, struct tcp_pcb *_pcb, err_t _err)
{
	absl_socket_t* socket = (absl_socket_t*)_arg;

	ABSL_UNUSED_ARG(_pcb);
	ABSL_UNUSED_ARG(_err);

	socket->tcp_pcb->state = ESTABLISHED;

	return ERR_OK;
}

static err_t absl_socket_receive_tcp_freertos (void *_arg, struct tcp_pcb *_tpcb, struct pbuf *_pbuf, err_t _err)
{
	absl_socket_t* socket;

	ABSL_UNUSED_ARG(_err);

	LWIP_ASSERT("arg != NULL", _arg != NULL);

	socket = (absl_socket_t*)_arg;

	/* if we receive an empty tcp frame from client => close connection */
	if (_pbuf == NULL)
	{
//		socket->tcp_pcb->state = CLOSING;
	}
	else
	{
		memcpy(socket->socket_config->rx_buffer, _pbuf->payload, _pbuf->len);
		socket->rx_length = _pbuf->len;

		/* indicate that the packet has been received */
		tcp_recved(_tpcb, _pbuf->len);

	    pbuf_free(_pbuf);

		absl_event_set_fromISR(&socket->event_group, ABSL_SOCKET_MESSAGE_RECEIVED);
	}

	return ERR_OK;
}

static err_t absl_socket_sent_tcp_freertos(void *_arg, struct tcp_pcb *_pcb, u16_t _len)
{
	absl_socket_t* socket = (absl_socket_t*)_arg;

	ABSL_UNUSED_ARG(_pcb);

    socket->data_length -= _len;

    if(socket->data_length <= 0)
    {
    	socket->tcp_sending_data = false;
		absl_event_set_fromISR(&socket->event_group, ABSL_SOCKET_MESSAGE_SEND);
    }

   	return ERR_OK;
}

static err_t absl_socket_poll_tcp_freertos(void *_arg, struct tcp_pcb *_tpcb)
{
	ABSL_UNUSED_ARG(_arg);
	ABSL_UNUSED_ARG(_tpcb);

	return ERR_OK;
}

static void absl_socket_error_freertos(void *_arg, err_t _err)
{
	absl_socket_t* socket = (absl_socket_t*)_arg;

	if ((socket->tcp_pcb) || (_err == ERR_RST) || (_err == ERR_CLSD))
	{

	}
	else
	{
		absl_socket_tcp_close_freertos(socket);
	}
}
#endif

#endif
#endif
#endif /* ABSL_SOCKET */
