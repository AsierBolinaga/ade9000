/*
 * mqtt_protocol.h
 *
 *  Created on: Apr 19, 2023
 *      Author: abolinaga
 */

#ifndef MQTT_PROTOCOL_H_
#define MQTT_PROTOCOL_H_

#include "protocol_types.h"

#include "pl_event.h"

typedef enum mqtt_protocol_events
{
	MP_ALERT_JSON_GET_ERROR = 0,
	MP_TS_JSON_GET_ERROR,
	MP_INFO_JSON_GET_ERROR,
	MP_UNKNOWN_SENSOR_STATUS,
	MP_SENSOR_STATUS_JSON_GET_ERROR,
	MP_UNKNOWN_SERVICE_STATUS,
	MP_SERVICE_STATUS_JSON_GET_ERROR,
	MP_ALERT_NO_VARS_CONFG,
	MP_ALERT_NO_TX_CONFG,
	MP_EVENTS_MAXVALUE
}mqtt_protocol_events_t;

bool mqtt_protocol_init(protocol_config_t* _protocol_config, pl_event_t* _event_group,
						uint32_t _rx_event, uint32_t _frame_rx_event, uint32_t _server_timeout_event, 
						uint32_t _server_disconnected_event, uint32_t _full_buff_event);
bool mqtt_protocol_connect(void);
bool mqtt_protocol_reconnect(void);
bool mqtt_protocol_send_discovery(void);
bool mqtt_protocol_status_changed(state_change_data_t* _state_change_data);
bool mqtt_protocol_device_status_changed(state_change_data_t* _state_change_data);
bool mqtt_protocol_signalize_event(alert_data_t* _alert_data);
bool mqtt_protocol_send_timestamp(uint64_t _time);
bool mqtt_protocol_send_info(device_status_information_t* _device_status_info);
bool mqtt_protocol_disconnect(void);

manufacturing_t mqtt_protocol_get_manufactur(void);
char* 			mqtt_protocol_get_ip(void);
char* 			mqtt_protocol_get_sync_when(void);
char* 			mqtt_protocol_get_sync_interval(void);

protocol_rv_t mqtt_protocol_listen(char* _sensor, char* _service, uint32_t _msg_get_timeout_seg, uint32_t *_messages_left);
protocol_rv_t mqtt_protocol_messages_to_get(char* _sensor, char* _service, uint32_t *_messages_left);
uint32_t mqtt_protocol_get_frame(uint8_t* _frame);
protocol_rv_t mqtt_protocol_frame_type(char* _sensor, char* _service);

void mqtt_protocol_reset(void);

#ifdef FABRICATION_TEST
void mqtt_protocol_send_id(uint32_t _id);
void mqtt_protocol_irq0_detected(bool _irq0_detected);
void mqtt_protocol_reset_done(bool _reset_done);
void mqtt_protocol_send_written_manufacturing(bool _written);
#endif

#endif /* MQTT_PROTOCOL_H_ */
