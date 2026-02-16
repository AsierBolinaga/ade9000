/*
 * mqtt_protocol.c
 *
 *  Created on: Apr 19, 2023
 *      Author: abolinaga
 */
#include "mqtt_protocol.h"

#include "json.h"
#include "interfaces.h"

#include "absl_hw_config.h"
#include "absl_mqtt.h"
#include "absl_thread.h"
#include "absl_debug.h"
#include "absl_macros.h"
#include "absl_timer.h"
#include "absl_time.h"
#include "absl_event.h"

#define MAX_SENSOR_AMOUNT 		2
#define MAX_SERVICE_AMOUNT 		MAX_SENSOR_AMOUNT * 2

typedef enum mqtt_subscribe_topics
{
	MQTT_CONFIG_TOPIC = 0,
	MQTT_START_RAW_TOPIC,
	MQTT_STOP_RAW_TOPIC,
	MQTT_RESET_TOPIC,
	MQTT_RESET_SENSOR_TOPIC,
	MQTT_REBOOT_TOPIC,
	MQTT_UPDATE_TOPIC,
	MQTT_TIME_SYNC_TOPIC,
	MQTT_GET_TIME_TOPIC,
	MQTT_GET_INFO_TOPIC,
	MQTT_MANUFACTUR,
#ifdef FABRICATION_TEST
	MQTT_REQUEST_ID,
	MQTT_REQUEST_IRQ0,
	MQTT_REQUEST_RST,
#endif
	MQTT_SUB_TOPICS_MAXVALUE
}mqtt_subscribe_topics_t;

typedef enum mqtt_publish_topics
{
	MQTT_STATE_TOPIC = 0,
	MQTT_DISCOVERY_TOPIC,
	MQTT_ALERT_TOPIC,
	MQTT_TIME_TOPIC,
	MQTT_INFO_TOPIC,
#ifdef FABRICATION_TEST
	MQTT_ID,
	MQTT_IRQ0,
	MQTT_RST,
	MQTT_MANU_WRITTEN,
#endif
	MQTT_PUB_TOPICS_MAXVALUE
}mqtt_publish_topics_t;

typedef protocol_rv_t (*topic_cb_t)(char* _sensor, char* _service, char* _rx_buffer);

typedef struct mqtt_topics_info
{
	bool		has_id;
	char* 		preID_topic;
	char*		postID_topic;
	uint32_t	QoS;
	topic_cb_t	topic_cb;
}mqtt_topics_info_t;

static char 	topic[300];
static char 	mqtt_payload[1000];

static manufacturing_t 			last_manufacturing;
static time_sync_config_t 		last_sync_config;

static bool mqtt_protocol_sensor_status_changed(state_change_data_t* _state_change_data);
static bool mqtt_protocol_service_status_changed(state_change_data_t* _state_change_data);

static protocol_rv_t mqtt_protocol_config_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_start_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_stop_messager_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_reset_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_reset_sensor_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_reboot_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_update_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_sync_time_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_get_timestamp_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_get_info_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_manufactur_message_received(char* _sensor, char* _service, char* _rx_buffer);
#ifdef FABRICATION_TEST
static protocol_rv_t mqtt_protocol_request_id_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_request_irq0_message_received(char* _sensor, char* _service, char* _rx_buffer);
static protocol_rv_t mqtt_protocol_request_reset_message_received(char* _sensor, char* _service, char* _rx_buffer);
#endif

static void mqtt_protocol_get_sensor_and_service_from_topic(uint8_t _topic_index, char* _topic, char* _sensor, char* _service);

static mqtt_topics_info_t mqtt_subscribe_topics_info[MQTT_SUB_TOPICS_MAXVALUE] =
{
	{true,  "devices/", 			"/config/#",   		2, 	mqtt_protocol_config_message_received},
	{true,  "devices/", 			"/cmd/start/#",		1,	mqtt_protocol_start_message_received},
	{true,  "devices/", 			"/cmd/stop/#",		1, 	mqtt_protocol_stop_messager_received},
	{true,  "devices/",     		"/cmd/reset",		1,	mqtt_protocol_reset_message_received},
	{true,  "devices/",     		"/cmd/reset/#",		1,	mqtt_protocol_reset_sensor_message_received},
	{true,  "devices/",				"/cmd/reboot",		2,	mqtt_protocol_reboot_message_received},
	{true,  "devices/",	 			"/update",			2,	mqtt_protocol_update_message_received},
	{false, "devices/sync",	 		NULL,				2,	mqtt_protocol_sync_time_message_received},
	{false, "devices/timestamp",	NULL,				2,	mqtt_protocol_get_timestamp_message_received},
	{false,	"devices/info",			NULL,				2,	mqtt_protocol_get_info_message_received},
	{true,  "devices/", 			"/manufactur",		2, 	mqtt_protocol_manufactur_message_received},
#ifdef FABRICATION_TEST
	{false,  "devices/requestid",		NULL,			2, 	mqtt_protocol_request_id_message_received},
	{false,  "devices/requestirq0",		NULL,			2, 	mqtt_protocol_request_irq0_message_received},
	{false,  "devices/requestreset",	NULL,			2, 	mqtt_protocol_request_reset_message_received},
#endif
};

static mqtt_topics_info_t mqtt_publish_topics_info[MQTT_PUB_TOPICS_MAXVALUE] =
{
	{true,  "devices/", 		"/state",		2, 	NULL},
	{true, 	"devices/", 		"/discovery",	2, 	NULL},
	{true, 	"devices/", 		"/alerts",		2, 	NULL},
	{true,	"devices/", 		"/timestamp",	2, 	NULL},
	{true, 	"devices/", 		"/info",		2, 	NULL},
#ifdef FABRICATION_TEST
	{false, "devices/id",			NULL,		2, 	NULL},
	{false, "devices/irq0",			NULL,		2, 	NULL},
	{false, "devices/reset",		NULL,		2, 	NULL},
	{false, "devices/manufactur",	NULL,		2, 	NULL},
#endif
};

static absl_mqtt_topics_t last_will;
static char* json_last_will_buff = NULL;
static absl_mqtt_topics_t topics_to_subscribe[MQTT_SUB_TOPICS_MAXVALUE];

static absl_mqtt_t			cmd_mqtt;
static absl_mqtt_config_t* 	cmd_mqtt_config;

static sensor_config_t* 	sensors_config;
static uint8_t		 		sensor_amount;
static char*				deviceID;
static char*				model;
static char*				fw_version;
static void* 				events_array;

bool mqtt_protocol_init(protocol_config_t* _protocol_config, absl_event_t* _event_group,
						uint32_t _rx_event, uint32_t _frame_rx_event, uint32_t _server_timeout_event, 
						uint32_t _server_disconnected_event, uint32_t _full_buff_event)
{
	bool return_value = false;

	cmd_mqtt_config = absl_config_get_mqtt_conf(_protocol_config->mqtt_conf_index);
	if(ABSL_MQTT_RV_OK == absl_mqtt_init(&cmd_mqtt, cmd_mqtt_config))
	{
		absl_mqtt_config_rx_event(&cmd_mqtt, _event_group, _rx_event, _frame_rx_event, _server_disconnected_event, _server_timeout_event, _full_buff_event);
		if(ABSL_MQTT_RV_OK == absl_mqtt_create(&cmd_mqtt))
		{
			sensors_config = _protocol_config->sensors_config;
			sensor_amount = _protocol_config->sensor_amount;
			deviceID = _protocol_config->device_ID;
			model = _protocol_config->model;
			fw_version = _protocol_config->fw_version;
			events_array = _protocol_config->mqtt_events_array;

			json_init(_protocol_config->json_events_array);

			return_value = true;
		}
	}

	return return_value;
}

bool mqtt_protocol_connect(void)
{
	bool return_value = false;

	uint32_t topic_index;

	strcpy(last_will.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_STATE_TOPIC].has_id)
	{
		strcat(last_will.topics, deviceID);
		strcat(last_will.topics,  mqtt_publish_topics_info[MQTT_STATE_TOPIC].postID_topic);
	}
	last_will.qos = mqtt_publish_topics_info[MQTT_STATE_TOPIC].QoS;
	json_last_will_buff = json_get_status_data("offline", NULL, 0);

	if(ABSL_MQTT_RV_OK == absl_mqtt_connect(&cmd_mqtt, deviceID, last_will, json_last_will_buff))
	{
		for(topic_index = 0; topic_index < MQTT_SUB_TOPICS_MAXVALUE; topic_index++)
		{
			strcpy(topics_to_subscribe[topic_index].topics,
					mqtt_subscribe_topics_info[topic_index].preID_topic);
			if(mqtt_subscribe_topics_info[topic_index].has_id)
			{
				strcat(topics_to_subscribe[topic_index].topics, deviceID);
				strcat(topics_to_subscribe[topic_index].topics,
						mqtt_subscribe_topics_info[topic_index].postID_topic);
			}
			topics_to_subscribe[topic_index].qos = mqtt_subscribe_topics_info[topic_index].QoS;
		}

		if(ABSL_MQTT_RV_OK == absl_mqtt_subscribe_topics(&cmd_mqtt, topics_to_subscribe,  MQTT_SUB_TOPICS_MAXVALUE))
		{
			return_value = true;
		}
	}
	json_clear(json_last_will_buff);

	return return_value;
}

bool mqtt_protocol_reconnect(void)
{
	bool return_value = false;

	json_last_will_buff = json_get_status_data("offline", NULL, 0);

	if(ABSL_MQTT_RV_OK == absl_mqtt_reconnect(&cmd_mqtt, json_last_will_buff))
	{
		if(ABSL_MQTT_RV_OK == absl_mqtt_subscribe_topics(&cmd_mqtt, topics_to_subscribe, MQTT_SUB_TOPICS_MAXVALUE))
		{
			return_value = true;
		}
	}
	
	json_clear(json_last_will_buff);

	return return_value;
}

bool mqtt_protocol_send_discovery(void)
{
	bool return_value = false;

	char* json_discovery_data_buff;
	absl_mqtt_topics_t discovery_topic;

	strcpy(discovery_topic.topics, mqtt_publish_topics_info[MQTT_DISCOVERY_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_STATE_TOPIC].has_id)
	{
		strcat(discovery_topic.topics, deviceID);
		strcat(discovery_topic.topics, mqtt_publish_topics_info[MQTT_DISCOVERY_TOPIC].postID_topic);
	}
	discovery_topic.qos = mqtt_publish_topics_info[MQTT_DISCOVERY_TOPIC].QoS;

	json_discovery_data_buff = json_get_discovery_data(sensor_amount, sensors_config, model, fw_version);

	if(ABSL_MQTT_RV_OK == absl_mqtt_publish(&cmd_mqtt,
										discovery_topic.topics,
										json_discovery_data_buff,
										strlen(json_discovery_data_buff),
										discovery_topic.qos,
										true))
	{
		return_value = true;
	}

	json_clear(json_discovery_data_buff);
	return return_value;
}

bool mqtt_protocol_status_changed(state_change_data_t* _state_change_data)
{

	bool return_value = false;

	switch(_state_change_data->state_type)
	{
		case DEVICE_STATE:
			return_value = mqtt_protocol_device_status_changed(_state_change_data);
		break;
		case SENSOR_STATE:
			return_value = mqtt_protocol_sensor_status_changed(_state_change_data);
		break;
		case SERVICE_STATE:
			return_value = mqtt_protocol_service_status_changed(_state_change_data);
		break;
		default:
		break;
	}

	return return_value;
}

bool mqtt_protocol_device_status_changed(state_change_data_t* _state_change_data)
{
	bool return_value = false;

	char* json_status_data_buff = NULL;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t status_topic;

	strcpy(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_STATE_TOPIC].has_id)
	{
		strcat(status_topic.topics, deviceID);
		strcat(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].postID_topic);
	}
	status_topic.qos = mqtt_publish_topics_info[MQTT_STATE_TOPIC].QoS;

	switch(_state_change_data->state_info.device_info.status)
	{
	case DEVICE_STATUS_OFFLINE:
		json_status_data_buff = json_get_status_data("offline", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case DEVICE_STATUS_ONLINE:
		json_status_data_buff = json_get_status_data("online", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case DEVICE_STATUS_FAILURE:
		json_status_data_buff = json_get_status_data("failure", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	default:
		return return_value;
		break;
	}

	if(NULL != json_status_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								status_topic.topics,
								json_status_data_buff,
								strlen(json_status_data_buff),
								status_topic.qos,
								true);

		if((ABSL_MQTT_RV_OK == mqtt_rv) || (ABSL_MQTT_RV_DISCONNETED == mqtt_rv))
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from broker! Device status not send\n");
			return_value = true;
		}

		json_clear(json_status_data_buff);
	}

	return return_value;
}

bool mqtt_protocol_signalize_event(alert_data_t* _alert_data)
{
	bool return_value = false;
	char* json_status_data_buff;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t status_topic;

	status_topic.qos = mqtt_publish_topics_info[MQTT_ALERT_TOPIC].QoS;

	strcpy(status_topic.topics, mqtt_publish_topics_info[MQTT_ALERT_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_ALERT_TOPIC].has_id)
	{
		strcat(status_topic.topics, deviceID);
		strcat(status_topic.topics, mqtt_publish_topics_info[MQTT_ALERT_TOPIC].postID_topic);
	}

	if(DOMAIN_SENSOR == _alert_data->domain)
	{
		json_status_data_buff = json_get_sensor_service_alert_data(_alert_data->time, _alert_data->event_level, "sensor", _alert_data->source, _alert_data->error_code);
	}
	else if(DOMAIN_SERVICE == _alert_data->domain)
	{
		json_status_data_buff = json_get_sensor_service_alert_data(_alert_data->time, _alert_data->event_level, "service", _alert_data->source, _alert_data->error_code);
	}
	else
	{
		json_status_data_buff = json_get_system_alert_data(_alert_data->time, _alert_data->event_level, _alert_data->error_code);
	}

	if(NULL != json_status_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								status_topic.topics,
								json_status_data_buff,
								strlen(json_status_data_buff),
								status_topic.qos,
								false);

		if(ABSL_MQTT_RV_OK == mqtt_rv)
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from broker! Event not send\n");
			return_value = true; 
		}
	}
	else
	{
		mqtt_notify_system_event(events_array, MP_ALERT_JSON_GET_ERROR);
	}

	json_clear(json_status_data_buff);

	return return_value;
}

bool mqtt_protocol_send_timestamp(uint64_t _time)
{
	bool return_value = false;
	char* json_time_data_buff;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t time_topic;

	time_topic.qos = mqtt_publish_topics_info[MQTT_TIME_TOPIC].QoS;

	strcpy(time_topic.topics, mqtt_publish_topics_info[MQTT_TIME_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_ALERT_TOPIC].has_id)
	{
		strcat(time_topic.topics, deviceID);
		strcat(time_topic.topics, mqtt_publish_topics_info[MQTT_TIME_TOPIC].postID_topic);
	}

	json_time_data_buff = json_get_time_data(_time);

	if(NULL != json_time_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								time_topic.topics,
								json_time_data_buff,
								strlen(json_time_data_buff),
								time_topic.qos,
								false);

		if(ABSL_MQTT_RV_OK == mqtt_rv)
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from broker! Event not send\n");
			return_value = true; 
		}
	}
	else
	{
		mqtt_notify_system_event(events_array, MP_TS_JSON_GET_ERROR);
	}

	json_clear(json_time_data_buff);

	return return_value;
}

bool mqtt_protocol_send_info(device_status_information_t* _device_status_info)
{
	bool return_value = false;
	char* json_info_data_buff;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t info_topic;

	info_topic.qos = mqtt_publish_topics_info[MQTT_INFO_TOPIC].QoS;

	strcpy(info_topic.topics, mqtt_publish_topics_info[MQTT_INFO_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_INFO_TOPIC].has_id)
	{
		strcat(info_topic.topics, deviceID);
		strcat(info_topic.topics, mqtt_publish_topics_info[MQTT_INFO_TOPIC].postID_topic);
	}

	json_info_data_buff = json_get_info_data(_device_status_info, fw_version);

	if(NULL != json_info_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								info_topic.topics,
								json_info_data_buff,
								strlen(json_info_data_buff),
								info_topic.qos,
								false);

		if(ABSL_MQTT_RV_OK == mqtt_rv)
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from broker! Event not send\n");
			return_value = true;
		}
	}
	else
	{
		mqtt_notify_system_event(events_array, MP_INFO_JSON_GET_ERROR);
	}

	json_clear(json_info_data_buff);

	return return_value;
}

bool mqtt_protocol_disconnect(void)
{
	absl_mqtt_disconnect(&cmd_mqtt);
	absl_mqtt_reset(&cmd_mqtt);
	return true;
}

manufacturing_t	mqtt_protocol_get_manufactur(void)
{
	return last_manufacturing;
}

char* mqtt_protocol_get_ip(void)
{
	return absl_mqtt_get_ip(&cmd_mqtt);
}


char* mqtt_protocol_get_sync_when(void)
{
	return last_sync_config.when;
}

char* mqtt_protocol_get_sync_interval(void)
{
	return last_sync_config.interval;
}

protocol_rv_t mqtt_protocol_messages_to_get(char* _sensor, char* _service, uint32_t *_messages_left)
{
	protocol_rv_t return_received_msg_type = PROTOCOL_INVALID_MESSAGE;

	uint32_t length;
	uint32_t topic_index;
	uint32_t topic_length;
	uint32_t length_to_compare;

	memset(topic, 0, 300);
	memset(mqtt_payload, 0, 1000);

	*_messages_left = absl_mqtt_get_message(&cmd_mqtt, &topic_length, topic, &length, mqtt_payload);

	for(topic_index = 0; topic_index < MQTT_SUB_TOPICS_MAXVALUE; topic_index++)
	{
		if(strstr(topics_to_subscribe[topic_index].topics, "#") != NULL)
		{
			length_to_compare = strlen(topics_to_subscribe[topic_index].topics) - 1;
		}
		else
		{
			length_to_compare = strlen(topics_to_subscribe[topic_index].topics);
		}

		if(!strncmp(topic, topics_to_subscribe[topic_index].topics, length_to_compare))
		{
			if(strstr(topics_to_subscribe[topic_index].topics, "#") != NULL)
			{
				mqtt_protocol_get_sensor_and_service_from_topic(topic_index, topic, _sensor, _service);
			}
			return_received_msg_type = mqtt_subscribe_topics_info[topic_index].topic_cb(_sensor, _service, mqtt_payload);
		}
	}

	return return_received_msg_type;
}

uint32_t mqtt_protocol_get_frame(uint8_t* _frame)
{
	return absl_mqtt_get_frame(&cmd_mqtt, (char*)_frame);
}

protocol_rv_t mqtt_protocol_frame_type(char* _sensor, char* _service)
{
	protocol_rv_t return_received_msg_type = PROTOCOL_INVALID_MESSAGE;

	uint32_t topic_index;
	uint32_t length_to_compare;

	memset(topic, 0, 100);
	memcpy(topic,  absl_mqtt_get_last_topic(&cmd_mqtt), absl_mqtt_get_last_topic_length(&cmd_mqtt));

	for(topic_index = 0; topic_index < MQTT_SUB_TOPICS_MAXVALUE; topic_index++)
	{
		if(strstr(topics_to_subscribe[topic_index].topics, "#") != NULL)
		{
			length_to_compare = strlen(topics_to_subscribe[topic_index].topics) - 1;
		}
		else
		{
			length_to_compare = strlen(topics_to_subscribe[topic_index].topics);
		}

		if(!strncmp(topic, topics_to_subscribe[topic_index].topics, length_to_compare))
		{
			if(strstr(topics_to_subscribe[topic_index].topics, "#") != NULL)
			{
				mqtt_protocol_get_sensor_and_service_from_topic(topic_index, topic, _sensor, _service);
			}
			return_received_msg_type = mqtt_subscribe_topics_info[topic_index].topic_cb(_sensor, _service, NULL);
		}
	}

	return return_received_msg_type;
}

#ifdef FABRICATION_TEST
void mqtt_protocol_send_id(uint32_t _id)
{
	absl_mqtt_topics_t id_topic;
	char*	json_id_data_buff;

	id_topic.qos = mqtt_publish_topics_info[MQTT_ID].QoS;

	json_id_data_buff = json_get_id_data(_id);

	strcpy(id_topic.topics, mqtt_publish_topics_info[MQTT_ID].preID_topic);
	if(mqtt_publish_topics_info[MQTT_ID].has_id)
	{
		strcat(id_topic.topics, deviceID);
		strcat(id_topic.topics, mqtt_publish_topics_info[MQTT_ID].postID_topic);
	}

	 absl_mqtt_publish(&cmd_mqtt,
					 id_topic.topics,
					 json_id_data_buff,
					 strlen(json_id_data_buff),
					 id_topic.qos,
					 false);

	json_clear(json_id_data_buff);
}

void mqtt_protocol_irq0_detected(bool _irq0_detected)
{
	absl_mqtt_topics_t irq0_topic;
	char*	json_iq0_data_buff;

	json_iq0_data_buff = json_get_irq0_data(_irq0_detected);

	strcpy(irq0_topic.topics, mqtt_publish_topics_info[MQTT_IRQ0].preID_topic);
	if(mqtt_publish_topics_info[MQTT_IRQ0].has_id)
	{
		strcat(irq0_topic.topics, deviceID);
		strcat(irq0_topic.topics, mqtt_publish_topics_info[MQTT_IRQ0].postID_topic);
	}

	 absl_mqtt_publish(&cmd_mqtt,
			 	 	 irq0_topic.topics,
					 json_iq0_data_buff,
					 strlen(json_iq0_data_buff),
					 irq0_topic.qos,
					 false);

	json_clear(json_iq0_data_buff);
}

void mqtt_protocol_reset_done(bool _reset_done)
{
	absl_mqtt_topics_t reset_topic;
	char*	json_reset_data_buff;

	json_reset_data_buff = json_get_reset_data(_reset_done);

	strcpy(reset_topic.topics, mqtt_publish_topics_info[MQTT_RST].preID_topic);
	if(mqtt_publish_topics_info[MQTT_IRQ0].has_id)
	{
		strcat(reset_topic.topics, deviceID);
		strcat(reset_topic.topics, mqtt_publish_topics_info[MQTT_RST].postID_topic);
	}

	 absl_mqtt_publish(&cmd_mqtt,
			 	 	 reset_topic.topics,
			 	 	 json_reset_data_buff,
					 strlen(json_reset_data_buff),
					 reset_topic.qos,
					 false);

	json_clear(json_reset_data_buff);
}

void mqtt_protocol_send_written_manufacturing(bool _written)
{
	absl_mqtt_topics_t manu_topic;
	char*	json_manu_data_buff;

	json_manu_data_buff = json_get_manufactur_written_data(_written);

	strcpy(manu_topic.topics, mqtt_publish_topics_info[MQTT_MANU_WRITTEN].preID_topic);
	if(mqtt_publish_topics_info[MQTT_IRQ0].has_id)
	{
		strcat(manu_topic.topics, deviceID);
		strcat(manu_topic.topics, mqtt_publish_topics_info[MQTT_MANU_WRITTEN].postID_topic);
	}

	 absl_mqtt_publish(&cmd_mqtt,
					 manu_topic.topics,
					 json_manu_data_buff,
					 strlen(json_manu_data_buff),
					 manu_topic.qos,
					 false);

	json_clear(json_manu_data_buff);
}
#endif

static bool mqtt_protocol_sensor_status_changed(state_change_data_t* _state_change_data)
{
	bool return_value = false;

	char* json_status_data_buff = NULL;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t status_topic;

	strcpy(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_STATE_TOPIC].has_id)
	{
		strcat(status_topic.topics, deviceID);
		strcat(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].postID_topic);
	}
	strcat(status_topic.topics, "/");
	strcat(status_topic.topics, sensors_config[_state_change_data->state_info.sensor_info.sensor].sensor_name);

	status_topic.qos = mqtt_publish_topics_info[MQTT_STATE_TOPIC].QoS;

	switch(_state_change_data->state_info.sensor_info.status)
	{
	case SENSOR_STATUS_IDLE:
		json_status_data_buff = json_get_status_data("idle", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case SENSOR_STATUS_CONFIGURED:
		json_status_data_buff = json_get_status_data("configured", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case SENSOR_STATUS_FAILURE:
		json_status_data_buff = json_get_status_data("failure", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	default:
		mqtt_notify_system_event(events_array, MP_UNKNOWN_SENSOR_STATUS);
		return return_value;
		break;
	}

	if(NULL != json_status_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								status_topic.topics,
								json_status_data_buff,
								strlen(json_status_data_buff),
								status_topic.qos,
								true);

		if(ABSL_MQTT_RV_OK == mqtt_rv)
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from bkroker! Device status not send\n");
			return_value = true;
		}

		json_clear(json_status_data_buff);
	}
	else
	{
		mqtt_notify_system_event(events_array, MP_SENSOR_STATUS_JSON_GET_ERROR);
	}

	return return_value;
}

static bool mqtt_protocol_service_status_changed(state_change_data_t* _state_change_data)
{
	bool return_value = false;
	char* json_status_data_buff = NULL;

	absl_mqtt_rv_t     mqtt_rv;
	absl_mqtt_topics_t status_topic;

	strcpy(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].preID_topic);
	if(mqtt_publish_topics_info[MQTT_STATE_TOPIC].has_id)
	{
		strcat(status_topic.topics, deviceID);
		strcat(status_topic.topics, mqtt_publish_topics_info[MQTT_STATE_TOPIC].postID_topic);
	}
	strcat(status_topic.topics, "/");
	strcat(status_topic.topics, sensors_config[_state_change_data->state_info.service_info.sensor].sensor_name);
	strcat(status_topic.topics, "/");
	strcat(status_topic.topics, sensors_config[_state_change_data->state_info.service_info.sensor].sensor_services_config[_state_change_data->state_info.service_info.service]->service_name);

	status_topic.qos = mqtt_publish_topics_info[MQTT_STATE_TOPIC].QoS;

	switch(_state_change_data->state_info.service_info.status)
	{
	case SERVICE_STATUS_IDLE:
		json_status_data_buff = json_get_status_data("idle", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case SERVICE_STATUS_RUNNING:
		json_status_data_buff = json_get_status_data("running", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	case SERVICE_STATUS_FAILURE:
		json_status_data_buff = json_get_status_data("failure", _state_change_data->error_codes, _state_change_data->error_code_amount);
		break;
	default:
		mqtt_notify_system_event(events_array, MP_UNKNOWN_SERVICE_STATUS);
		return return_value;
		break;
	}

	if(NULL != json_status_data_buff)
	{
		mqtt_rv = absl_mqtt_publish(&cmd_mqtt,
								status_topic.topics,
								json_status_data_buff,
								strlen(json_status_data_buff),
								status_topic.qos,
								true);

		if(ABSL_MQTT_RV_OK == mqtt_rv)
		{
			return_value = true;
		}
		else if(ABSL_MQTT_RV_DISCONNETED == mqtt_rv)
		{
			absl_debug_printf("MQTT disconnected from broker! Service status not send\n");
			return_value = true;
		}

		json_clear(json_status_data_buff);
	}
	else
	{
		mqtt_notify_system_event(events_array, MP_SERVICE_STATUS_JSON_GET_ERROR);
	}

	return return_value;
}

static protocol_rv_t mqtt_protocol_config_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	protocol_rv_t 	return_received_msg = PROTOCOL_INVALID_CONFIG_MESSAGE;

	ABSL_UNUSED_ARG(_service);

	for(uint32_t sensor_index = 0; sensor_index < sensor_amount; sensor_index++)
	{
		if (strstr(_sensor, sensors_config[sensor_index].sensor_name) != NULL)
		{
			if(true == sensors_config[sensor_index].sensor_config_cb(_rx_buffer, sensors_config[sensor_index].sensor_config_data))
			{
				return_received_msg = PROTOCOL_CONFIG_MSG_RECIEVED;
			}
		}
	}

	return return_received_msg;
}

protocol_rv_t mqtt_protocol_desconnected(void)
{
	protocol_rv_t return_value = PROTOCOL_ERROR;

	return return_value;
}

static protocol_rv_t mqtt_protocol_start_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	protocol_rv_t 	return_received_msg = PROTOCOL_INVALID_START_MESSAGE;

	if(NULL == _rx_buffer)
	{
		return_received_msg = PROTOCOL_START_CMD_RECIEVED;
	}
	else
	{
		/* TODO - Refactor */
		for(uint32_t sensor_index = 0; sensor_index < sensor_amount; sensor_index++)
		{
			if (strstr(_sensor, sensors_config[sensor_index].sensor_name) != NULL)
			{
				for(uint32_t service_index = 0; service_index < sensors_config[sensor_index].service_amount; service_index++)
				{
					if (strstr(_service, sensors_config[sensor_index].sensor_services_config[service_index]->service_name) != NULL)
					{
						if(sensors_config[sensor_index].sensor_services_config[service_index]->service_config_cb != NULL)
						{
							if(true == sensors_config[sensor_index].sensor_services_config[service_index]->service_config_cb(_rx_buffer,
									   sensors_config[sensor_index].sensor_services_config[service_index]->service_config_data))
							{
								if(sensors_config[sensor_index].sensor_services_config[service_index]->service_tx_config_cb != NULL)
								{
									if(true == sensors_config[sensor_index].sensor_services_config[service_index]->service_tx_config_cb(_rx_buffer,
											   sensors_config[sensor_index].sensor_services_config[service_index]->service_tx_config_data))
									{
										return_received_msg = PROTOCOL_START_CMD_RECIEVED;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return return_received_msg;
}

protocol_rv_t mqtt_protocol_stop_messager_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_STOP_CMD_RECIEVED;
}

static protocol_rv_t mqtt_protocol_reset_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_RESET_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_reset_sensor_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_SENSOR_RESET_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_reboot_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_REBOOT_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_update_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_UPDATE_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_sync_time_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	protocol_rv_t return_received_msg = PROTOCOL_INVALID_SYNC_MESSAGE;

	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);

	if(true == json_get_time_sync_config(&last_sync_config, _rx_buffer))
	{
		return_received_msg = PROTOCOL_TIME_SYNC_MSG_RECIEVED;
	}
	
	return return_received_msg;
}

static protocol_rv_t mqtt_protocol_get_timestamp_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_GET_TIMESTAMP_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_get_info_message_received(char* _sensor, char* _service, char* _rx_buffer)
{

	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_GET_INFO_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_manufactur_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	protocol_rv_t return_received_msg = PROTOCOL_INVALID_MANUFACTUR_MESSAGE;

	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);

	if(true == json_get_sensor_manufactur(_rx_buffer, &last_manufacturing))
	{
		return_received_msg = PROTOCOL_MANUFACTUR_MSG_RECIEVED;
	}

	return return_received_msg;
}

#ifdef FABRICATION_TEST
static protocol_rv_t mqtt_protocol_request_id_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_ID_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_request_irq0_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_IRQ0_MSG_RECIEVED;
}

static protocol_rv_t mqtt_protocol_request_reset_message_received(char* _sensor, char* _service, char* _rx_buffer)
{
	ABSL_UNUSED_ARG(_sensor);
	ABSL_UNUSED_ARG(_service);
	ABSL_UNUSED_ARG(_rx_buffer);

	return PROTOCOL_RST_MSG_RECIEVED;
}
#endif

static void mqtt_protocol_get_sensor_and_service_from_topic(uint8_t _topic_index, char* _topic, char* _sensor, char* _service)
{
	char* delim = "/";
	char* ptr = strtok(_topic, delim);

	memset(_sensor, 0, 20);
	memset(_service, 0, 20);

	while(ptr != NULL)
	{
		if(strstr(topics_to_subscribe[_topic_index].topics, ptr) == NULL)
		{
			if(strlen(_sensor) == 0)
			{
				memcpy(_sensor, ptr, strlen(ptr));
			}
			else
			{
				memcpy(_service, ptr, strlen(ptr));
			}
		}

		ptr = strtok(NULL, delim);
	}
}

void mqtt_protocol_reset(void)
{
	absl_mqtt_reset(&cmd_mqtt);
}
