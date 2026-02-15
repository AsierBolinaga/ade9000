/*
 * pl_socket.c
 *
 *  Created on: 1 feb. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_mqtt.h"
#ifdef PL_MQTT

#if defined(PL_OS_FREE_RTOS)
#if !defined(PL_EVENT)
#error "pl_mqtt module for freertos uses event, PL_EVENT needs to be active in pl_config.h file."
#else
#include "lwip/tcpip.h"

#include "pl_thread.h"
#include "pl_debug.h"
#include "pl_macros.h"

static void pl_mqtt_connect_broker(void *ctx);
static void pl_mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

pl_mqtt_rv_t pl_mqtt_init_freertos(pl_mqtt_t * _mqtt, pl_mqtt_config_t* _mqtt_config)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;

	if(NULL!= _mqtt_config)
	{
		_mqtt->mqtt_config = _mqtt_config;

		if(PL_PHY_RV_OK == pl_phy_init(&_mqtt->mqtt_phy, _mqtt_config->mqtt_phy))
		{
			pl_event_create(&_mqtt->event_group);

			_mqtt->buffer_index = 0;
			for(uint32_t index = 0; index < PL_MQTT_MSG_MAX_AMOUNT; index++)
			{
				_mqtt->mqtt_msg_lengths[index] = 0;
				_mqtt->topic_lengths[index] = 0;
			}

			_mqtt->mqtt_connected = false;
			_mqtt->config_event = false;
			_mqtt->mqtt_msg_in_buff= 0;
			_mqtt->mqtt_topic_in_buff = 0;
			_mqtt->topic_buff_index = 0;

			memset(_mqtt->mqtt_topic, 0, 300);
			memset(_mqtt->mqtt_config->rx_buffer, 0, _mqtt->mqtt_config->rx_buffer_length);
			mqtt_rv = PL_MQTT_RV_OK;
		}
	}
	else
	{
		mqtt_rv = PL_MQTT_RV_NO_CONF;
	}

	return mqtt_rv;
}

void pl_mqtt_config_rx_event_freertos(pl_mqtt_t * _mqtt, pl_event_t* _event_group, uint32_t _rx_event,
											  uint32_t _frame_rx_event, uint32_t _server_disconnected,
											  uint32_t _connect_timeout_event, uint32_t _full_buffer_event)
{
	_mqtt->config_event_group 		 = _event_group;
	_mqtt->rx_config_event 			 = _rx_event;
	_mqtt->frame_rx_config_event	 = _frame_rx_event;
	_mqtt->connect_timeout_event 	 = _connect_timeout_event;
	_mqtt->server_disconnected_event = _server_disconnected;
	_mqtt->rx_full_buff_event 		 = _full_buffer_event;
	_mqtt->config_event 			 = true;
}

pl_mqtt_rv_t pl_mqtt_create_freertos(pl_mqtt_t * _mqtt)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;

	LOCK_TCPIP_CORE();
	_mqtt->mqtt_client = mqtt_client_new();
	UNLOCK_TCPIP_CORE();

	if (_mqtt->mqtt_client != 0)
	{
		mqtt_rv = PL_MQTT_RV_OK;
	}

	return mqtt_rv;
}

pl_mqtt_rv_t pl_mqtt_connect_freertos(pl_mqtt_t * _mqtt, char* _client_id, pl_mqtt_topics_t _last_will, char* _last_will_msg)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;

	err_t 		err;
	uint32_t 	events;

	if(false == _mqtt->mqtt_connected)
	{
		if(0 != *_client_id)
		{
			memcpy(_mqtt->mqtt_client_id, _client_id, strlen(_client_id));

			_mqtt->last_will_mqtt_topic = _last_will;
			_mqtt->last_will_msg = _last_will_msg;

			/* Start connecting to MQTT broker from tcpip_thread */
			err = tcpip_callback(pl_mqtt_connect_broker, _mqtt);
			if(ERR_OK == err)
			{
				pl_event_wait_freertos(&_mqtt->event_group, PL_MQTT_CONNECTED | PL_MQTT_TIMEOUT | PL_MQTT_DISCONNECTED, &events);

				if(PL_MQTT_CONNECTED == (events & PL_MQTT_CONNECTED))
				{
					mqtt_rv = PL_MQTT_RV_OK;
				}
				else if(PL_MQTT_TIMEOUT == (events & PL_MQTT_TIMEOUT))
				{
					mqtt_rv = PL_MQTT_RV_TIMEOUT;
				}
				else if(PL_MQTT_DISCONNECTED == (events & PL_MQTT_DISCONNECTED))
				{
					mqtt_rv = PL_MQTT_RV_DISCONNETED;
				}
			}
		}
	}
	else
	{
		mqtt_rv = PL_MQTT_RV_OK;
	}

	return mqtt_rv;
}

pl_mqtt_rv_t pl_mqtt_reconnect(pl_mqtt_t * _mqtt, char* _last_will_msg)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;

	err_t 		err;
	uint32_t 	events;

	if(false == _mqtt->mqtt_connected)
	{
		_mqtt->last_will_msg = _last_will_msg;

		pl_event_clear_events(&_mqtt->event_group, PL_MQTT_CONNECTED | PL_MQTT_TIMEOUT | PL_MQTT_DISCONNECTED);

		err = tcpip_callback(pl_mqtt_connect_broker, _mqtt);
		if(ERR_OK == err)
		{
			pl_event_wait(&_mqtt->event_group, PL_MQTT_CONNECTED | PL_MQTT_TIMEOUT | PL_MQTT_DISCONNECTED, &events);
			if(PL_MQTT_CONNECTED == (events & PL_MQTT_CONNECTED))
			{
				mqtt_rv = PL_MQTT_RV_OK;
			}
			else if(PL_MQTT_TIMEOUT == (events & PL_MQTT_TIMEOUT))
			{
				mqtt_rv = PL_MQTT_RV_TIMEOUT;
			}
		}
	}
	else
	{
		mqtt_rv = PL_MQTT_RV_OK;
	}

	return mqtt_rv;
}

void pl_mqtt_disconnect_freertos(pl_mqtt_t * _mqtt)
{
	LOCK_TCPIP_CORE();
	mqtt_disconnect(_mqtt->mqtt_client);
	_mqtt->mqtt_connected = false;
	UNLOCK_TCPIP_CORE();
}

static void pl_mqtt_message_published_cb(void *arg, err_t err)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)arg;

    if (err == ERR_OK)
    {
        pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_MESSAGE_SEND);
    }
    else if (err == ERR_TIMEOUT)
    {
    	 pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_TIMEOUT);
    }
    else
    {
    	pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_ERROR);
    }
}

pl_mqtt_rv_t pl_mqtt_publish_freertos(pl_mqtt_t * _mqtt, char* _topic, char* _tx_buff, uint16_t _payload_length, uint8_t _qos, bool _retain)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;
	uint32_t events;

	if(true == _mqtt->mqtt_connected)
	{
		LOCK_TCPIP_CORE();
		u8_t retain;
		pl_debug_printf("Going to publish to the topic \"%s\"...\r\n", _topic);

		if(_retain)
		{
			retain = 1;
		}
		else
		{
			retain = 0;
		}

		mqtt_publish(_mqtt->mqtt_client, _topic, _tx_buff, _payload_length, _qos, retain,
					pl_mqtt_message_published_cb, (void *)_mqtt);
		UNLOCK_TCPIP_CORE();

		if(PL_EVENT_RV_NO_EVENT !=  pl_event_timed_wait(&_mqtt->event_group, PL_MQTT_MESSAGE_SEND | PL_MQTT_TIMEOUT | PL_MQTT_ERROR, &events, 500))
		{
			if(PL_MQTT_MESSAGE_SEND == (events & PL_MQTT_MESSAGE_SEND))
			{
				mqtt_rv = PL_MQTT_RV_OK;
			}
			else if(PL_MQTT_TIMEOUT == (events & PL_MQTT_TIMEOUT))
			{
				mqtt_rv = PL_MQTT_RV_TIMEOUT;
			}
		}
		else
		{
			mqtt_rv = PL_MQTT_RV_TIMEOUT;
		}
	}
	else
	{
		mqtt_rv = PL_MQTT_RV_DISCONNETED;
	}

	return mqtt_rv;
}

pl_mqtt_rv_t pl_mqtt_wait_messages_freertos(pl_mqtt_t * _mqtt, uint32_t *msgs, uint32_t _timeout_ms)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;
	uint32_t events;

	if(!_timeout_ms)
	{
		pl_event_wait(&_mqtt->event_group, PL_MQTT_MESSAGE_RECEIVED | PL_MQTT_DISCONNECTED | PL_MQTT_TIMEOUT, &events);
	}
	else
	{
		pl_event_timed_wait(&_mqtt->event_group, PL_MQTT_MESSAGE_RECEIVED | PL_MQTT_DISCONNECTED | PL_MQTT_TIMEOUT, &events, _timeout_ms);
	}

	if(0 != events)
	{
		if(PL_MQTT_MESSAGE_RECEIVED == (events & PL_MQTT_MESSAGE_RECEIVED))
		{
			*msgs = _mqtt->mqtt_msg_in_buff;
			mqtt_rv = PL_MQTT_RV_OK;
		}
		else if((PL_MQTT_DISCONNECTED == (events & PL_MQTT_DISCONNECTED)) ||
				(PL_MQTT_TIMEOUT == (events & PL_MQTT_TIMEOUT)) )
		{
			mqtt_rv = PL_MQTT_RV_DISCONNETED;
		}
		else
		{
			if(_timeout_ms != 0)
			{
				mqtt_rv = PL_MQTT_RV_OK;
			}
		}
	}
	else
	{
		mqtt_rv = PL_MQTT_RV_TIMEOUT;
	}

	return mqtt_rv;
}

char* pl_mqtt_get_last_topic_freertos(pl_mqtt_t * _mqtt)
{
	return _mqtt->mqtt_topic;
}

uint32_t pl_mqtt_get_message_freertos(pl_mqtt_t * _mqtt, uint32_t* _topic_length, char* _topic, uint32_t* _msg_length, char* _rx_msg)
{
	char aux_topic_buff[300];
	char aux_rx_buff[_mqtt->mqtt_config->rx_buffer_length];

	memset(aux_topic_buff, 0, 300);
	memset(aux_rx_buff, 0, _mqtt->mqtt_config->rx_buffer_length);

	*_topic_length = _mqtt->topic_lengths[0];
	memcpy(_topic, _mqtt->mqtt_topic, _mqtt->topic_lengths[0]);

	*_msg_length = _mqtt->mqtt_msg_lengths[0];
	memcpy(_rx_msg, _mqtt->mqtt_config->rx_buffer, _mqtt->mqtt_msg_lengths[0]);

	memcpy(aux_topic_buff, &_mqtt->mqtt_topic[_mqtt->topic_lengths[0]], (_mqtt->topic_buff_index - _mqtt->topic_lengths[0]));
	memcpy(aux_rx_buff, &_mqtt->mqtt_config->rx_buffer[_mqtt->mqtt_msg_lengths[0]], (_mqtt->buffer_index - _mqtt->mqtt_msg_lengths[0]));

	memcpy(_mqtt->mqtt_topic, aux_topic_buff, 300);
	memcpy(_mqtt->mqtt_config->rx_buffer, aux_rx_buff, _mqtt->mqtt_config->rx_buffer_length);

	_mqtt->topic_buff_index -= _mqtt->topic_lengths[0];
	_mqtt->buffer_index -=  _mqtt->mqtt_msg_lengths[0];

	for(uint32_t msg_index = 1; msg_index < PL_MQTT_MSG_MAX_AMOUNT; msg_index++)
	{
		 _mqtt->topic_lengths[msg_index-1] =  _mqtt->topic_lengths[msg_index];
		 _mqtt->mqtt_msg_lengths[msg_index-1] =  _mqtt->mqtt_msg_lengths[msg_index];
	}
	_mqtt->topic_lengths[PL_MQTT_MSG_MAX_AMOUNT-1] = 0;
	_mqtt->mqtt_msg_lengths[PL_MQTT_MSG_MAX_AMOUNT-1] = 0;


	if(0 < _mqtt->mqtt_msg_in_buff)
	{
		_mqtt->mqtt_msg_in_buff--;
		_mqtt->mqtt_topic_in_buff--;
	}

	return _mqtt->mqtt_msg_in_buff;
}

uint32_t pl_mqtt_get_frame_freertos(pl_mqtt_t * _mqtt, char* _rx_msg)
{
	uint32_t frame_length;

	frame_length = _mqtt->mqtt_msg_lengths[0];
	memcpy(_rx_msg, &_mqtt->mqtt_config->rx_buffer[_mqtt->buffer_index - frame_length], frame_length);

	_mqtt->buffer_index -= frame_length;
	_mqtt->mqtt_msg_lengths[0] = 0;

	if(0 < _mqtt->mqtt_msg_in_buff)
	{
		_mqtt->topic_buff_index -= _mqtt->topic_lengths[0];
		_mqtt->buffer_index -=  _mqtt->mqtt_msg_lengths[0];

		for(uint32_t msg_index = 1; msg_index < PL_MQTT_MSG_MAX_AMOUNT; msg_index++)
		{
			 _mqtt->topic_lengths[msg_index-1] =  _mqtt->topic_lengths[msg_index];
			 _mqtt->mqtt_msg_lengths[msg_index-1] =  _mqtt->mqtt_msg_lengths[msg_index];
		}
		_mqtt->topic_lengths[PL_MQTT_MSG_MAX_AMOUNT-1] = 0;
		_mqtt->mqtt_msg_lengths[PL_MQTT_MSG_MAX_AMOUNT-1] = 0;

		_mqtt->mqtt_msg_in_buff--;
		_mqtt->mqtt_topic_in_buff--;
	}

	return frame_length;
}


uint32_t pl_mqtt_get_last_topic_length_freertos(pl_mqtt_t * _mqtt)
{
	return _mqtt->topic_lengths[_mqtt->mqtt_topic_in_buff - 1];
}

static void pl_mqtt_connect_broker(void *ctx)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)ctx;

	pl_phy_t* phy = pl_phy_get_object(mqtt->mqtt_phy);

	pl_debug_printf("Connecting to MQTT broker at %s...\r\n", ipaddr_ntoa(&phy->netif_remote_addr));

	mqtt->mqtt_client_info.client_id = (const char *)&mqtt->mqtt_client_id[0];
	mqtt->mqtt_client_info.client_user = mqtt->mqtt_config->user;
	mqtt->mqtt_client_info.client_pass = mqtt->mqtt_config->password;
	mqtt->mqtt_client_info.keep_alive = mqtt->mqtt_config->keep_alive;
	mqtt->mqtt_client_info.will_topic = mqtt->last_will_mqtt_topic.topics;
	mqtt->mqtt_client_info.will_msg = mqtt->last_will_msg;
	mqtt->mqtt_client_info.will_qos = mqtt->last_will_mqtt_topic.qos;
	mqtt->mqtt_client_info.will_retain = 1;
#if LWIP_ALTCP && LWIP_ALTCP_TLS
	mqtt->mqtt_client_info.tls_config = NULL;
#endif

	mqtt->mqtt_connected = false;

	mqtt_client_connect(mqtt->mqtt_client, &phy->netif_remote_addr, 1883, pl_mqtt_connection_cb,
						mqtt, &mqtt->mqtt_client_info);
}

static void pl_mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)arg;

	PL_UNUSED_ARG(client);

	mqtt->mqtt_connected = (status == MQTT_CONNECT_ACCEPTED);

	switch (status)
	{
		case MQTT_CONNECT_ACCEPTED:
			mqtt->mqtt_msg_in_buff = 0;
			mqtt->mqtt_topic_in_buff = 0;
			mqtt->buffer_index  = 0;
			mqtt->topic_buff_index = 0;
			mqtt->mqtt_connected = true;
			PRINTF("MQTT client \"%s\" connected.\r\n", mqtt->mqtt_client_info.client_id);
			pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_CONNECTED);
			break;

		case MQTT_CONNECT_DISCONNECTED:
			mqtt->mqtt_msg_in_buff = 0;
			mqtt->mqtt_topic_in_buff = 0;
			mqtt->buffer_index  = 0;
			mqtt->topic_buff_index = 0;
			mqtt->mqtt_connected = false;
			PRINTF("MQTT client \"%s\" not connected.\r\n", mqtt->mqtt_client_info.client_id);
			pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_DISCONNECTED);
			if(true == mqtt->config_event)
			{
				pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->server_disconnected_event);
			}
			break;

		case MQTT_CONNECT_TIMEOUT:
			mqtt->mqtt_msg_in_buff = 0;
			mqtt->mqtt_topic_in_buff = 0;
			mqtt->buffer_index  = 0;
			mqtt->topic_buff_index = 0;
			mqtt->mqtt_connected = false;
			PRINTF("MQTT client \"%s\" connection timeout.\r\n", mqtt->mqtt_client_info.client_id);
			pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_TIMEOUT);
			if(true == mqtt->config_event)
			{
				pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->connect_timeout_event);
			}
			break;

		case MQTT_CONNECT_REFUSED_PROTOCOL_VERSION:
		case MQTT_CONNECT_REFUSED_IDENTIFIER:
		case MQTT_CONNECT_REFUSED_SERVER:
		case MQTT_CONNECT_REFUSED_USERNAME_PASS:
		case MQTT_CONNECT_REFUSED_NOT_AUTHORIZED_:
			mqtt->mqtt_msg_in_buff = 0;
			mqtt->mqtt_topic_in_buff = 0;
			mqtt->buffer_index  = 0;
			mqtt->topic_buff_index = 0;
			mqtt->mqtt_connected = false;
			PRINTF("MQTT client \"%s\" connection refused: %d.\r\n", mqtt->mqtt_client_info.client_id, (int)status);
			pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_NOT_CONNECTED);
			break;

		default:
			mqtt->mqtt_msg_in_buff = 0;
			mqtt->mqtt_topic_in_buff = 0;
			mqtt->buffer_index  = 0;
			mqtt->topic_buff_index = 0;
			mqtt->mqtt_connected = false;
			PRINTF("MQTT client \"%s\" connection status: %d.\r\n", mqtt->mqtt_client_info.client_id, (int)status);
			pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_ERROR);
			if(true == mqtt->config_event)
			{
				pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->server_disconnected_event);
			}
			break;
	}
}

static void pl_mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)arg;

	if((PL_MQTT_MSG_MAX_AMOUNT <= mqtt->mqtt_topic_in_buff) || (PL_MQTT_MSG_MAX_AMOUNT <= mqtt->topic_buff_index))
	{
		 pl_mqtt_reset_freertos(mqtt);
	}
	else
	{
		mqtt->topic_lengths[mqtt->mqtt_topic_in_buff] = strlen(topic);
		memcpy(&mqtt->mqtt_topic[mqtt->topic_buff_index], topic, mqtt->topic_lengths[mqtt->mqtt_topic_in_buff]);
		mqtt->topic_buff_index += mqtt->topic_lengths[mqtt->mqtt_topic_in_buff];
		mqtt->mqtt_topic_in_buff++;

		pl_debug_printf("Received %u bytes from the topic \"%s\"\n", tot_len, topic);
	}
}

static void pl_mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)arg;

	if((mqtt->buffer_index + len) >= mqtt->mqtt_config->rx_buffer_length)
	{
		pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_SBUFFER_FULL);
		if(true == mqtt->config_event)
		{
			pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->rx_full_buff_event);
		}
	}
	else
	{
		mqtt->mqtt_msg_lengths[mqtt->mqtt_msg_in_buff] += len;
		memcpy(&mqtt->mqtt_config->rx_buffer[mqtt->buffer_index], data, len);
		mqtt->buffer_index += len;

		pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_MESSAGE_FRAME_RECEIVED);
		if(true == mqtt->config_event)
		{
			pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->frame_rx_config_event);
		}
	}

	if (flags & MQTT_DATA_FLAG_LAST)
	{
		mqtt->mqtt_msg_in_buff++;
		pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_MESSAGE_RECEIVED);
		if(true == mqtt->config_event)
		{
			pl_event_set_fromISR_freertos(mqtt->config_event_group, mqtt->rx_config_event);
		}
	}
	pl_thread_sleep(5); //wait a millisecond to give time to the app to get the data
}

static void pl_mqtt_topic_subscribed_cb(void *arg, err_t err)
{
	pl_mqtt_t* mqtt = (pl_mqtt_t*)arg;

    if (err == ERR_OK)
    {
        pl_debug_printf("Topic subscribed.\r\n");
    	pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_SUBSCRIBED);
    }
    else
    {
        pl_debug_printf("Failed to subscribe topic. Error %d\r\n", err);
    	pl_event_set_fromISR_freertos(&mqtt->event_group, PL_MQTT_ERROR);
    }
}

pl_mqtt_rv_t pl_mqtt_subscribe_topics_freertos(pl_mqtt_t * _mqtt, pl_mqtt_topics_t* mqtt_topis, uint32_t _topic_num)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_ERROR;
	bool subscribtion_failed = false;

	uint32_t events;
    err_t err;
    uint32_t i;

    LOCK_TCPIP_CORE();
    mqtt_set_inpub_callback(_mqtt->mqtt_client, pl_mqtt_incoming_publish_cb, pl_mqtt_incoming_data_cb, _mqtt);
    UNLOCK_TCPIP_CORE();
    for (i = 0; i < _topic_num; i++)
    {
        LOCK_TCPIP_CORE();
        err = mqtt_subscribe(_mqtt->mqtt_client, mqtt_topis[i].topics, mqtt_topis[i].qos,
        					 pl_mqtt_topic_subscribed_cb, _mqtt);
        UNLOCK_TCPIP_CORE();

        if (err == ERR_OK)
        {
            pl_debug_printf("Subscribing to the topic \"%s\" with QoS %d...\r\n", mqtt_topis[i].topics, mqtt_topis[i].qos);
        }
        else
        {
            pl_debug_printf("Failed to subscribe to the topic \"%s\" with QoS %d: %d.\r\n", mqtt_topis[i].topics, mqtt_topis[i].qos, err);
        	subscribtion_failed = true;
        }

        if(false == subscribtion_failed)
        {
			pl_event_wait(&_mqtt->event_group, PL_MQTT_SUBSCRIBED | PL_MQTT_ERROR, &events);

			if(PL_MQTT_ERROR == (events & PL_MQTT_ERROR))
			{
				subscribtion_failed = true;
			}
        }
    }

    if(false == subscribtion_failed)
    {
    	mqtt_rv = PL_MQTT_RV_OK;
    }

    return mqtt_rv;
}

void pl_mqtt_reset_freertos(pl_mqtt_t * _mqtt)
{
	_mqtt->buffer_index = 0;
	for(uint32_t index = 0; index < PL_MQTT_MSG_MAX_AMOUNT; index++)
	{
		_mqtt->mqtt_msg_lengths[index] = 0;
		_mqtt->topic_lengths[index] = 0;
	}

	_mqtt->mqtt_msg_in_buff= 0;
	_mqtt->mqtt_topic_in_buff = 0;
	_mqtt->topic_buff_index = 0;

	memset(_mqtt->mqtt_topic, 0, 300);
	memset(_mqtt->mqtt_config->rx_buffer, 0, _mqtt->mqtt_config->rx_buffer_length);
}

char* pl_mqtt_get_ip_freertos(pl_mqtt_t * _mqtt)
{
	pl_phy_t* phy = pl_phy_get_object(_mqtt->mqtt_phy);

	return ipaddr_ntoa(&phy->netif_local_addr);
}

pl_mqtt_rv_t pl_mqtt_check_connection_freertos(pl_mqtt_t * _mqtt)
{
	pl_mqtt_rv_t mqtt_rv = PL_MQTT_RV_DISCONNETED;

	LOCK_TCPIP_CORE();
	if(mqtt_client_is_connected(_mqtt->mqtt_client))
	{
		mqtt_rv = PL_MQTT_RV_OK;
	}
	UNLOCK_TCPIP_CORE();

	return mqtt_rv;
}

#endif
#endif
#endif /* PL_SOCKET */
