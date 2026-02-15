/*
 * json.h
 *
 *  Created on: Apr 19, 2023
 *      Author: abolinaga
 */

#ifndef JSON_H_
#define JSON_H_

#include "protocol_types.h"

typedef enum json_protocol_events
{
	JSON_EVENTS_NO_FAST_VARS_CONFIG = 0,
	JSON_EVENTS_NO_FAST_TX_CONFIG,
	JSON_EVENTS_NO_SLOW_VARS_CONFIG,
	JSON_EVENTS_NO_SLOW_TX_CONFIG,
	JSON_EVENTS_MAXVALUE
}json_protocol_events_t;

typedef enum json_mode
{
	JSON_REQUEST = 0,
	JSON_NOTIFY,
	JSON_STREAMING,
	JSON_UNKNOWN
}json_mode_t;

typedef enum json_raw_variable_type
{
	JSON_SLOW_VAR_TYPE = 0,
	JSON_FAST_VAR_TYPE,
	JSON_UNKNOWN_VAR_TYPE
}json_raw_variable_type_t;

bool json_init(void* _events_array);

json_mode_t json_get_mode(char* _data_buff);

char* json_get_discovery_data(uint8_t _sensor_amount, sensor_config_t* _sensors_config, char* _model, char* _fw_v);

char* json_get_device_status_data(char* _status);

char* json_get_status_data(char* _status, error_code_t* _error_codes, uint8_t _error_codes_size);

char* json_get_system_alert_data(uint64_t _timestamp, char* _level, error_code_t* _error_codes);

char* json_get_sensor_service_alert_data(uint64_t _timestamp, char* _level, char* _domain, char* _source, error_code_t* _error_codes);

char* json_get_time_data(uint64_t _time);

char* json_get_info_data(device_status_information_t* _sensor_info, char* _fw_version);

char* json_get_id_data(uint32_t _id);

char* json_get_irq0_data(bool _irq0_detected);

char* json_get_reset_data(bool _reset_done);

char* json_get_manufactur_written_data(bool _written);

bool json_clear(char* _json_buff);

bool json_get_sensor_manufactur(char* _manufactur_data_buff, manufacturing_t* _sensor_manufacturing);

bool json_get_energy_sensor_config(char* _config_data_buff, void* _config_data);

bool json_get_raw_service_config(char* _config_data_buff, void* _raw_vars_config);

bool json_get_vars_service_config(char* _config_data_buff, void* _vars_config);

bool json_get_test_service_config(char* _config_data_buff, void* _test_config);

bool json_get_raw_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf);

bool json_get_vars_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf);

bool json_get_time_sync_config(time_sync_config_t* _sync_config, char* _config_data_buff);

json_raw_variable_type_t json_get_raw_variable_type(char* _data_buff);


#endif /* JSON_H_ */
