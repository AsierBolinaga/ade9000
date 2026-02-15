/*
 * json.c
 *
 *  Created on: Apr 19, 2023
 *      Author: abolinaga
 */
#include "json.h"

#include "cJSON.h"

#include "interfaces.h"

#include "pl_debug.h"
#include "pl_mutex.h"

#include "pl_macros.h"

static pl_mutex_t json_mutex;

static void* events_array;

char* json_buff;

static char* adc_name[ADC_READ_NUM] = {"ia_din", "ib_din", "ic_din", "va_din", "vb_din", "vc_din"};

static bool json_get_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf);

bool json_init(void* _events_array)
{
	bool ret_val = false;

	cJSON_Hooks json_hooks;

	events_array = _events_array;

	json_hooks.malloc_fn = pvPortMalloc;
	json_hooks.free_fn = vPortFree;

	cJSON_InitHooks(&json_hooks);

	if(PL_MUTEX_RV_OK == pl_mutex_create(&json_mutex))
	{
		ret_val = true;
	}

	return ret_val;
}

json_mode_t json_get_mode(char* _data_buff)
{
	json_mode_t json_mode = JSON_UNKNOWN;

	cJSON* cjson_rx = NULL;
	cJSON* cjson_mode = NULL;

	cjson_rx = cJSON_Parse(_data_buff);
	if(cjson_rx != NULL)
	{
		cjson_mode = cJSON_GetObjectItem(cjson_rx, "mode");
		if (strcmp(cjson_mode->valuestring, "request") == 0)
		{
			json_mode = JSON_REQUEST;
		}
		else if(strcmp(cjson_mode->valuestring, "notify") == 0)
		{
			json_mode = JSON_NOTIFY;
		}
		else if(strcmp(cjson_mode->valuestring, "streaming") == 0)
		{
			json_mode = JSON_STREAMING;
		}
		else
		{

		}
	}

	return json_mode;
}

char* json_get_discovery_data(uint8_t _sensor_amount, sensor_config_t* _sensors_config, char* _model, char* _fw_v)
{
	cJSON* cjson_tx = NULL;
	cJSON* cjson_services = NULL;
	cJSON* cjson_sensor = NULL;
	cJSON* cjson_sensors;

	cjson_tx = cJSON_CreateObject();
	cjson_sensors = cJSON_CreateArray();

	cJSON_AddStringToObject(cjson_tx, "model", _model);
	cJSON_AddStringToObject(cjson_tx, "fw", _fw_v);

	for(uint8_t sensor_index = 0; sensor_index < _sensor_amount; sensor_index++)
	{
		cjson_sensor = cJSON_CreateObject();

		cJSON_AddStringToObject(cjson_sensor, "name", _sensors_config[sensor_index].sensor_name);

		char* services[_sensors_config[sensor_index].service_amount];
		for(uint8_t service_index = 0; service_index < _sensors_config[sensor_index].service_amount; service_index++)
		{
			services[service_index] = _sensors_config[sensor_index].sensor_services_config[service_index]->service_name;
		}
		cjson_services = cJSON_CreateStringArray((const char *const *)services, _sensors_config[sensor_index].service_amount);
		cJSON_AddItemToObject(cjson_sensor, "services", cjson_services);

		cJSON_AddItemToArray(cjson_sensors, cjson_sensor);
	}

	cJSON_AddItemToObject(cjson_tx, "sensors", cjson_sensors);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_tx);

	cJSON_Delete(cjson_tx);

	return json_buff;
}

char* json_get_device_status_data(char* _status)
{
	cJSON* cjson_tx = NULL;

	cjson_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_tx, "state", _status);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_tx);


	cJSON_Delete(cjson_tx);

	return json_buff;
}

char* json_get_status_data(char* _status, error_code_t* _error_codes, uint8_t _error_codes_size)
{
	cJSON* cjson_tx = NULL;
	cJSON* cjson_errors = NULL;
	cJSON* cjson_error = NULL;

	cjson_tx = cJSON_CreateObject();
	cJSON_AddStringToObject(cjson_tx, "state", _status);

	cjson_errors = cJSON_CreateArray();

	if(0 != _error_codes_size)
	{
		for(uint8_t error_index = 0; error_index < _error_codes_size; error_index++)
		{
			cjson_error = cJSON_CreateObject();

			cJSON_AddNumberToObject(cjson_error, "code", _error_codes[error_index].error_code_num);
			cJSON_AddStringToObject(cjson_error, "description", _error_codes[error_index].error_code_description);

			cJSON_AddItemToArray(cjson_errors, cjson_error);
		}
	}

	cJSON_AddItemToObject(cjson_tx, "errors", cjson_errors);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_tx);

	cJSON_Delete(cjson_tx);

	return json_buff;
}

char* json_get_system_alert_data(uint64_t _timestamp, char* _level, error_code_t* _error_codes)
{
	cJSON* cjson_alerts = NULL;
	cJSON* cjson_alert = NULL;

	cjson_alerts = cJSON_CreateArray();
	cjson_alert = cJSON_CreateObject();

	cJSON_AddNumberToObject(cjson_alert, "timestamp", _timestamp);
	cJSON_AddStringToObject(cjson_alert, "level", _level);

	cJSON_AddNumberToObject(cjson_alert, "code", _error_codes->error_code_num);
	cJSON_AddStringToObject(cjson_alert, "description", _error_codes->error_code_description);

	cJSON_AddStringToObject(cjson_alert, "domain", "system");

	cJSON_AddItemToArray(cjson_alerts, cjson_alert);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_alerts);

	cJSON_Delete(cjson_alerts);

	return json_buff;
}

char* json_get_sensor_service_alert_data(uint64_t _timestamp, char* _level, char* _domain, char* _source , error_code_t* _error_codes)
{
	cJSON* cjson_alerts = NULL;
	cJSON* cjson_alert = NULL;

	cjson_alerts = cJSON_CreateArray();
	cjson_alert = cJSON_CreateObject();

	cJSON_AddNumberToObject(cjson_alert, "timestamp", _timestamp);
	cJSON_AddStringToObject(cjson_alert, "level", _level);

	cJSON_AddNumberToObject(cjson_alert, "code", _error_codes->error_code_num);
	cJSON_AddStringToObject(cjson_alert, "description", _error_codes->error_code_description);

	cJSON_AddStringToObject(cjson_alert, "domain", _domain);
	cJSON_AddStringToObject(cjson_alert, "source", _source);

	cJSON_AddItemToArray(cjson_alerts, cjson_alert);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_alerts);

	cJSON_Delete(cjson_alerts);

	return json_buff;
}

char* json_get_time_data(uint64_t _time)
{
	cJSON* cjson_time = NULL;

	cjson_time = cJSON_CreateObject();

	cJSON_AddNumberToObject(cjson_time, "timestamp", _time);
	cJSON_AddStringToObject(cjson_time, "unit", "us");

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_time);

	cJSON_Delete(cjson_time);

	return json_buff;
}

char* json_get_info_data(device_status_information_t* _sensor_info, char* _fw_version)
{
	cJSON* cjson_info = NULL;
	cJSON* cjson_hw_version = NULL;



	cjson_info = cJSON_CreateObject();
	cjson_hw_version = cJSON_CreateArray();

	cJSON_AddNumberToObject(cjson_info, "time", _sensor_info->timestamp);
	cJSON_AddStringToObject(cjson_info, "ip_address", _sensor_info->ip_address);
	cJSON_AddNumberToObject(cjson_info, "temperature", _sensor_info->temperature);
	cJSON_AddNumberToObject(cjson_info, "CPU_load", _sensor_info->cpu_load);
	cJSON_AddNumberToObject(cjson_info, "Heap_usage", _sensor_info->heap_usage);
	cJSON_AddStringToObject(cjson_info, "sync_interval", _sensor_info->sync_interval);
	cJSON_AddStringToObject(cjson_info, "fw", _fw_version);

	for(uint32_t board_index = 0; board_index < BOARD_AMOUNT; board_index++)
	{
		cJSON* board_hw_version = cJSON_CreateObject();

		cJSON_AddStringToObject(board_hw_version, "board", _sensor_info->hw_version[board_index].board);
		cJSON_AddStringToObject(board_hw_version, "serial", _sensor_info->hw_version[board_index].serial);
		cJSON_AddStringToObject(board_hw_version, "manufacturer", _sensor_info->hw_version[board_index].manufacturer);
		cJSON_AddStringToObject(board_hw_version, "hwversion", _sensor_info->hw_version[board_index].hw_version);

		cJSON_AddItemToArray(cjson_hw_version, board_hw_version);
	}

	cJSON_AddItemToObject(cjson_info, "manufacturing", cjson_hw_version);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_info);

	cJSON_Delete(cjson_info);

	return json_buff;
}

char* json_get_id_data(uint32_t _id)
{
	cJSON* cjson_id = NULL;

	cjson_id = cJSON_CreateObject();

	cJSON_AddNumberToObject(cjson_id, "ID", _id);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_id);

	cJSON_Delete(cjson_id);

	return json_buff;
}

char* json_get_irq0_data(bool _irq0_detected)
{
	cJSON* cjson_irq0 = NULL;

	cjson_irq0 = cJSON_CreateObject();

	cJSON_AddBoolToObject(cjson_irq0, "detected", _irq0_detected);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_irq0);

	cJSON_Delete(cjson_irq0);

	return json_buff;
}

char* json_get_reset_data(bool _reset_done)
{
	cJSON* cjson_reset = NULL;

	cjson_reset = cJSON_CreateObject();

	cJSON_AddBoolToObject(cjson_reset, "done", _reset_done);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_reset);

	cJSON_Delete(cjson_reset);

	return json_buff;
}

char* json_get_manufactur_written_data(bool _written)
{
	cJSON* cjson_manu_written = NULL;

	cjson_manu_written = cJSON_CreateObject();

	cJSON_AddBoolToObject(cjson_manu_written, "data_written", _written);

	pl_mutex_take(&json_mutex);

	json_buff = cJSON_Print(cjson_manu_written);

	cJSON_Delete(cjson_manu_written);

	return json_buff;
}

bool json_clear(char* _json_buff)
{
	bool ret_val = false;
	cJSON_free(_json_buff);

	if(PL_MUTEX_RV_OK == pl_mutex_give(&json_mutex))
	{
		ret_val = true;
	}

	return ret_val;
}

bool json_get_sensor_manufactur(char* _manufactur_data_buff, manufacturing_t* _sensor_manufacturing)
{
	bool return_value = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_variable = NULL;
	cJSON* cjson_hw_versions = NULL;

	uint32_t hw_array_size;

	cjson_rx = cJSON_Parse(_manufactur_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_variable = cJSON_GetObjectItem(cjson_rx, "id");
		memcpy(_sensor_manufacturing->id_mumber, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "mac");
		if(true == json_get_mac(cjson_variable->valuestring, _sensor_manufacturing->mac_address))
		{
			cjson_variable = cJSON_GetObjectItem(cjson_rx, "model");
			memcpy(_sensor_manufacturing->model, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

			cjson_hw_versions = cJSON_GetObjectItem(cjson_rx, "hw_version");
			hw_array_size = cJSON_GetArraySize(cjson_hw_versions);
			for(uint32_t hw_index = 0; hw_index < hw_array_size; hw_index++)
			{
				cJSON* hw_version = NULL;

				hw_version = cJSON_GetArrayItem(cjson_hw_versions, hw_index);

				cjson_variable = cJSON_GetObjectItem(hw_version, "board");
				memcpy(_sensor_manufacturing->hw_version[hw_index].board, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

				cjson_variable = cJSON_GetObjectItem(hw_version, "serial");
				memcpy(_sensor_manufacturing->hw_version[hw_index].serial, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

				cjson_variable = cJSON_GetObjectItem(hw_version, "manufacturer");
				memcpy(_sensor_manufacturing->hw_version[hw_index].manufacturer, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

				cjson_variable = cJSON_GetObjectItem(hw_version, "hwversion");
				memcpy(_sensor_manufacturing->hw_version[hw_index].hw_version, cjson_variable->valuestring, strlen(cjson_variable->valuestring));
			}

			cJSON_Delete(cjson_rx);

			return_value = true;
		}
	}

	return return_value;
}

bool json_get_energy_sensor_config(char* _config_data_buff, void* _config_data)
{
	bool data_ok = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_variable = NULL;

	energy_sensor_received_config_t* sensor_config = (energy_sensor_received_config_t*)_config_data;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_variable = cJSON_GetObjectItem(cjson_rx, "tc_primary_side");
		sensor_config->tc_primary_side = cjson_variable->valueint;

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "tc_secondary_side");
		sensor_config->tc_secondary_side = cjson_variable->valueint;

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "fundamental_frequency");
		sensor_config->fundamental_frequency = cjson_variable->valueint;

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "high_pass_filter_disable");
		if(cJSON_IsTrue(cjson_variable))
		{
			sensor_config->high_pass_filter_disable = true;
		}
		else
		{
			sensor_config->high_pass_filter_disable = false;
		}

		memset(sensor_config->hw_config, 0, 20);
		cjson_variable = cJSON_GetObjectItem(cjson_rx, "hw_config");
		memcpy(sensor_config->hw_config, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "thresholds");
		if(cjson_variable != NULL)
		{
			cJSON* cjson_oi = NULL;
			cJSON* cjson_dip = NULL;
			cJSON* cjson_swell = NULL;

			cjson_oi = cJSON_GetObjectItem(cjson_variable, "oi");
			cjson_dip = cJSON_GetObjectItem(cjson_variable, "dip");
			cjson_swell = cJSON_GetObjectItem(cjson_variable, "swell");

			if((cjson_oi != NULL) && (cjson_dip != NULL) && (cjson_swell != NULL))
			{
				sensor_config->OI_threshold = cjson_oi->valueint;
				sensor_config->DIP_threshold = cjson_dip->valueint;
				sensor_config->SWELL_threshold = cjson_swell->valueint;
			}

			data_ok = true;
		}

		if(true == data_ok)
		{
			data_ok = false;

			cjson_variable = cJSON_GetObjectItem(cjson_rx, "reverse_direction");
			if(cjson_variable != NULL)
			{
				cJSON* cjson_a = NULL;
				cJSON* cjson_b = NULL;
				cJSON* cjson_c = NULL;

				cjson_a = cJSON_GetObjectItem(cjson_variable, "a");
				cjson_b = cJSON_GetObjectItem(cjson_variable, "b");
				cjson_c = cJSON_GetObjectItem(cjson_variable, "c");

				if((cjson_a != NULL) && (cjson_b != NULL) && (cjson_c != NULL))
				{
					if(cJSON_IsTrue(cjson_a))
					{
						sensor_config->ia_invert = true;
					}
					else
					{
						sensor_config->ia_invert = false;
					}

					if(cJSON_IsTrue(cjson_b))
					{
						sensor_config->ib_invert = true;
					}
					else
					{
						sensor_config->ib_invert = false;
					}

					if(cJSON_IsTrue(cjson_c))
					{
						sensor_config->ic_invert = true;
					}
					else
					{
						sensor_config->ic_invert = false;
					}

					data_ok = true;
				}
			}
		}

		if(true == data_ok)
		{
			data_ok = false;
			cjson_variable = cJSON_GetObjectItem(cjson_rx, "adc_redirect");

			if(cjson_variable != NULL)
			{
				cJSON* cjson_adc = NULL;
				bool invalid_value_detected = false;

				for(uint32_t adc_index = 0; adc_index < ADC_READ_NUM; adc_index++)
				{
					cjson_adc =  cJSON_GetObjectItem(cjson_variable, adc_name[adc_index]);
					if((ADC_DEFAULT_VALUE >= cjson_adc->valueint) && (3 != cjson_adc->valueint))
					{
						sensor_config->adc_redirect[adc_index] = (adc_redirect_t)cjson_adc->valueint;
					}
					else
					{
						invalid_value_detected = true;
					}
				}

				if(false == invalid_value_detected)
				{
					data_ok = true;
				}
			}
		}

		cJSON_Delete(cjson_rx);
	}

	return data_ok;
}

bool json_get_raw_service_config(char* _config_data_buff, void* _raw_vars_config)
{
	bool return_value = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_config = NULL;

	raw_config_t* raw_vars_config = (raw_config_t*)_raw_vars_config;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_config = cJSON_GetObjectItem(cjson_rx, "config");
		if(cjson_config != NULL)
		{
			cJSON* vars = NULL;
			uint32_t vars_index = 0;

			vars = cJSON_GetObjectItem(cjson_config, "vars");
			raw_vars_config->raw_vars_amount = cJSON_GetArraySize(vars);

			while(vars_index < raw_vars_config->raw_vars_amount)
			{
				cJSON* var = NULL;

				memset(raw_vars_config->raw_vars_list[vars_index], 0, sizeof(raw_vars_config->raw_vars_list[vars_index]));

				var = cJSON_GetArrayItem(vars, vars_index);
				memcpy(raw_vars_config->raw_vars_list[vars_index], var->valuestring, strlen(var->valuestring));

				vars_index++;
			}

			return_value = true;
		}
		else
		{
			json_notify_system_event(events_array, JSON_EVENTS_NO_FAST_VARS_CONFIG);
		}

		cJSON_Delete(cjson_rx);
	}
	else
	{
		json_notify_system_event(events_array, JSON_EVENTS_NO_FAST_VARS_CONFIG);
	}

	return return_value;
}

bool json_get_vars_service_config(char* _config_data_buff, void* _vars_config)
{
	bool return_value = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_config = NULL;

	vars_config_t* vars_config = (vars_config_t*)_vars_config;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_config = cJSON_GetObjectItem(cjson_rx, "config");
		if(cjson_config != NULL)
		{
			cJSON* vars = NULL;
			cJSON* period = NULL;
			uint32_t vars_index = 0;

			vars = cJSON_GetObjectItem(cjson_config, "vars");
			if(NULL != vars)
			{
				vars_config->vars_amount = cJSON_GetArraySize(vars);

				while(vars_index < vars_config->vars_amount)
				{
					cJSON* var = NULL;

					memset(vars_config->vars_list[vars_index], 0, sizeof(vars_config->vars_list[vars_index]));

					var = cJSON_GetArrayItem(vars, vars_index);
					memcpy(vars_config->vars_list[vars_index], var->valuestring, strlen(var->valuestring));

					vars_index++;
				}

				period = cJSON_GetObjectItem(cjson_config, "period");
				if(NULL != period)
				{
					vars_config->period = period->valuedouble;
					return_value = true;
				}
			}
		}
		else
		{
			json_notify_system_event(events_array, JSON_EVENTS_NO_SLOW_VARS_CONFIG);
		}

		cJSON_Delete(cjson_rx);
	}
	else
	{
		json_notify_system_event(events_array, JSON_EVENTS_NO_SLOW_VARS_CONFIG);
	}

	return return_value;
}

bool json_get_test_service_config(char* _config_data_buff, void* _test_config)
{
	bool return_value = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_config = NULL;

	test_config_t* test_config = (test_config_t*)_test_config;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_config = cJSON_GetObjectItem(cjson_rx, "config");
		if(cjson_config != NULL)
		{
			cJSON* period = NULL;
			cJSON* data_size = NULL;

			period = cJSON_GetObjectItem(cjson_config, "period");
			if(NULL != period)
			{
				test_config->period = period->valuedouble;

				data_size = cJSON_GetObjectItem(cjson_config, "data_size");
				if(NULL != period)
				{
					test_config->data_size = data_size->valueint;
					return_value = true;
				}
			}
		}
		else
		{
//			json_notify_system_event(events_array, JSON_EVENTS_NO_SLOW_VARS_CONFIG);
		}

		cJSON_Delete(cjson_rx);
	}
	else
	{
		json_notify_system_event(events_array, JSON_EVENTS_NO_SLOW_VARS_CONFIG);
	}

	return return_value;
}

bool json_get_raw_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf)
{
	bool return_value = false;

	if(true == json_get_transmission_config(_config_data_buff, _transmission_conf))
	{
		return_value = true;
	}
	else
	{
		json_notify_system_event(events_array, JSON_EVENTS_NO_FAST_TX_CONFIG);
	}

	return return_value;
}

bool json_get_vars_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf)
{
	bool return_value = false;

	if(json_get_transmission_config(_config_data_buff, _transmission_conf))
	{
		return_value = true;
	}
	else
	{
		mqtt_notify_system_event(events_array, JSON_EVENTS_NO_SLOW_TX_CONFIG);
	}

	return return_value;
}

static bool json_get_transmission_config(char* _config_data_buff, transmission_config_t* _transmission_conf)
{
	bool return_value = false;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_transmission = NULL;
	cJSON* cjson_variable = NULL;
	cJSON* cjson_settings = NULL;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_transmission = cJSON_GetObjectItem(cjson_rx, "transmission");

		if(cjson_transmission != NULL)
		{
			cjson_variable = cJSON_GetObjectItem(cjson_transmission, "format");
			memset(_transmission_conf->format, 0 , 10);
			memcpy(_transmission_conf->format, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

			cjson_variable = cJSON_GetObjectItem(cjson_transmission, "protocol");
			memset(_transmission_conf->protocol, 0 , 10);
			memcpy(_transmission_conf->protocol, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

			cjson_settings = cJSON_GetObjectItem(cjson_transmission, "settings");
			if(cjson_settings != NULL)
			{
				cjson_variable = cJSON_GetObjectItem(cjson_settings, "IP");
				memset(_transmission_conf->settings.ip, 0, 20);
				memcpy(_transmission_conf->settings.ip, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

				cjson_variable = cJSON_GetObjectItem(cjson_settings, "port");
				_transmission_conf->settings.port = cjson_variable->valueint;
			}

			return_value = true;
		}
		else
		{

		}

		cJSON_Delete(cjson_rx);
	}
	else
	{

	}

	return return_value;
}

bool json_get_time_sync_config(time_sync_config_t* _sync_config, char* _config_data_buff)
{
	bool return_value = false;

	cJSON* cjson_rx = NULL;

	cjson_rx = cJSON_Parse(_config_data_buff);

	if(cjson_rx != NULL)
	{
		cJSON* cjson_variable = NULL;

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "when");
		memset(_sync_config->when, 0, 10);
		memcpy(_sync_config->when, cjson_variable->valuestring, strlen(cjson_variable->valuestring));

		cjson_variable = cJSON_GetObjectItem(cjson_rx, "interval");
		if(cjson_variable != NULL)
		{
			memset(_sync_config->interval, 0, 10);
			memcpy(_sync_config->interval, cjson_variable->valuestring, strlen(cjson_variable->valuestring));
		}

		cJSON_Delete(cjson_rx);

		return_value = true;
	}

	return return_value;
}

json_raw_variable_type_t json_get_raw_variable_type(char* _data_buff)
{
	json_raw_variable_type_t variable_type = JSON_UNKNOWN_VAR_TYPE;
	cJSON* cjson_rx = NULL;
	cJSON* cjson_variable = NULL;

	cjson_rx = cJSON_Parse(_data_buff);

	if(cjson_rx != NULL)
	{
		cjson_variable = cJSON_GetObjectItem(cjson_rx, "raw_var");
		if(!strcmp("slow", cjson_variable->valuestring))
		{
			variable_type = JSON_SLOW_VAR_TYPE;
		}
		else if(!strcmp("fast", cjson_variable->valuestring))
		{
			variable_type = JSON_FAST_VAR_TYPE;
		}
		else
		{
			/* Unknown */
		}
	}

	return variable_type;
}
