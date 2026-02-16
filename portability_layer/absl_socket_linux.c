/*
 * absl_socket.c
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_socket.h"
#ifdef ABSL_SOCKET

#if defined(ABSL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>


absl_socket_rv_t absl_socket_init_linux(absl_socket_t * _socket, absl_socket_config_t* _socket_config)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	if(NULL!= _socket_config)
	{
		_socket->socket_config = _socket_config;

		_socket->size_saddress = sizeof(_socket->si_other);
		_socket->remote_address = inet_addr(_socket_config->remote_ip_address);
		
		socket_rv = absl_socket_create_linux(_socket);
	}
	else
	{
		socket_rv = ABSL_SOCKET_RV_NO_CONF;
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_create_linux(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	if(ABSL_SOCKET_PROTOCOL_UDP == _socket->socket_config->socket_protocol)
	{
		//create a UDP socket
		_socket->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(0 <= _socket->socket)
		{
			socket_rv = ABSL_SOCKET_RV_OK;
		}
	}
	else if(ABSL_SOCKET_PROTOCOL_TCP  == _socket->socket_config->socket_protocol)
	{
		/* Todo - Implement TCP part */
	}
	else
	{
		socket_rv = ABSL_SOCKET_RV_NO_CONF;
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_bind_linux(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;

	memset((char *) &_socket->si_me, 0, sizeof(_socket->si_me));

	_socket->si_me.sin_family = AF_INET; 
	_socket->si_me.sin_port = htons(_socket->socket_config->src_port);
	_socket->si_me.sin_addr.s_addr = _socket->remote_address;

	//bind socket to port
	if(0 <= bind(_socket->socket,(struct sockaddr*)&_socket->si_me, sizeof(_socket->si_me)))
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}


absl_socket_rv_t absl_socket_connect_linux(absl_socket_t * _socket)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;
	
	/* todo - implement connect */

	return socket_rv;
}

absl_socket_rv_t absl_socket_receive_linux(absl_socket_t * _socket, char* _received_buff, uint32_t _rx_buff_size, uint32_t* _rx_length)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;
	
	*_rx_length = recvfrom(_socket->socket, _received_buff, _rx_buff_size, 0, (struct sockaddr *)&_socket->si_other, &_socket->size_saddress);
	if (0 < _rx_length)
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}

absl_socket_rv_t absl_socket_transfer_linux(absl_socket_t * _socket, char* _send_buff, uint32_t _length)
{
	absl_socket_rv_t socket_rv = ABSL_SOCKET_RV_ERROR;
	uint32_t send_length = 0;

	send_length =  sendto(_socket->socket, _send_buff, _length, 0, (struct sockaddr*) &_socket->si_other, _socket->size_saddress);
	if(0 <= send_length)
	{
		socket_rv = ABSL_SOCKET_RV_OK;
	}

	return socket_rv;
}


#endif /* ABSL_LINUX */
#endif /* ABSL_SOCKET */
