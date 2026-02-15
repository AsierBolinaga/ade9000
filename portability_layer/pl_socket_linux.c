/*
 * pl_socket.c
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_socket.h"
#ifdef PL_SOCKET

#if defined(PL_LINUX)
#include <arpa/inet.h>
#include <sys/socket.h>


pl_socket_rv_t pl_socket_init_linux(pl_socket_t * _socket, pl_socket_config_t* _socket_config)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;

	if(NULL!= _socket_config)
	{
		_socket->socket_config = _socket_config;

		_socket->size_saddress = sizeof(_socket->si_other);
		_socket->remote_address = inet_addr(_socket_config->remote_ip_address);
		
		socket_rv = pl_socket_create_linux(_socket);
	}
	else
	{
		socket_rv = PL_SOCKET_RV_NO_CONF;
	}

	return socket_rv;
}

pl_socket_rv_t pl_socket_create_linux(pl_socket_t * _socket)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;

	if(PL_SOCKET_PROTOCOL_UDP == _socket->socket_config->socket_protocol)
	{
		//create a UDP socket
		_socket->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(0 <= _socket->socket)
		{
			socket_rv = PL_SOCKET_RV_OK;
		}
	}
	else if(PL_SOCKET_PROTOCOL_TCP  == _socket->socket_config->socket_protocol)
	{
		/* Todo - Implement TCP part */
	}
	else
	{
		socket_rv = PL_SOCKET_RV_NO_CONF;
	}

	return socket_rv;
}

pl_socket_rv_t pl_socket_bind_linux(pl_socket_t * _socket)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;

	memset((char *) &_socket->si_me, 0, sizeof(_socket->si_me));

	_socket->si_me.sin_family = AF_INET; 
	_socket->si_me.sin_port = htons(_socket->socket_config->src_port);
	_socket->si_me.sin_addr.s_addr = _socket->remote_address;

	//bind socket to port
	if(0 <= bind(_socket->socket,(struct sockaddr*)&_socket->si_me, sizeof(_socket->si_me)))
	{
		socket_rv = PL_SOCKET_RV_OK;
	}

	return socket_rv;
}


pl_socket_rv_t pl_socket_connect_linux(pl_socket_t * _socket)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;
	
	/* todo - implement connect */

	return socket_rv;
}

pl_socket_rv_t pl_socket_receive_linux(pl_socket_t * _socket, char* _received_buff, uint32_t _rx_buff_size, uint32_t* _rx_length)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;
	
	*_rx_length = recvfrom(_socket->socket, _received_buff, _rx_buff_size, 0, (struct sockaddr *)&_socket->si_other, &_socket->size_saddress);
	if (0 < _rx_length)
	{
		socket_rv = PL_SOCKET_RV_OK;
	}

	return socket_rv;
}

pl_socket_rv_t pl_socket_transfer_linux(pl_socket_t * _socket, char* _send_buff, uint32_t _length)
{
	pl_socket_rv_t socket_rv = PL_SOCKET_RV_ERROR;
	uint32_t send_length = 0;

	send_length =  sendto(_socket->socket, _send_buff, _length, 0, (struct sockaddr*) &_socket->si_other, _socket->size_saddress);
	if(0 <= send_length)
	{
		socket_rv = PL_SOCKET_RV_OK;
	}

	return socket_rv;
}


#endif /* PL_LINUX */
#endif /* PL_SOCKET */
