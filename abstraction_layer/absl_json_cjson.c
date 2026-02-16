
#include "absl_json.h"
#if defined(ABSL_JSON) && defined(ABSL_CJSON)

absl_json_rv_t absl_json_init_cjson(absl_json_t *_json)
{
	absl_json_rv_t json_ret_val = ABSL_JSON_RV_ERROR;

	cJSON_Hooks json_hooks;

	json_hooks.malloc_fn = pvPortMalloc;
	json_hooks.free_fn = vPortFree;

	cJSON_InitHooks(&json_hooks);

	if(ABSL_MUTEX_RV_OK == absl_mutex_create(&_json->json_mutex))
	{
		json_ret_val = ABSL_JSON_RV_OK;
	}

	return json_ret_val;
}

absl_json_rv_t absl_json_parse_cjson(char* _buff, absl_json_object_t* _json_object)
{
	absl_json_rv_t json_ret_val = ABSL_JSON_RV_ERROR;

	_json_object->cjson_item = cJSON_Parse(_buff);

	if(NULL != _json_object->cjson_item)
	{
		json_ret_val = ABSL_JSON_RV_OK;
	}

	return json_ret_val;
}

char* absl_json_get_buff_cjson(absl_json_t *_json, absl_json_object_t* _json_object)
{
	absl_mutex_take(&_json->json_mutex);

	_json->json_buff = cJSON_Print(_json_object->cjson_item);

	cJSON_Delete(_json_object->cjson_item);

	return _json->json_buff;
}

char* absl_json_get_item_string_value_cjson(absl_json_t *_json, absl_json_object_t* _json_object, char* _item)
{
	char* item_value = NULL;

	_json->cjson_item = cJSON_GetObjectItem(_json_object->cjson_item, _item);
	if(_json->cjson_item != NULL)
	{
		item_value = _json->cjson_item->valuestring;
	}

	return item_value;
}

absl_json_object_t absl_json_get_item_object_cjson(absl_json_object_t* _json_object, char* _item)
{
	absl_json_object_t object;

	object.cjson_item = cJSON_GetObjectItem(_json_object->cjson_item, _item);

	return object;
}

uint32_t absl_json_get_array_size_cjson(absl_json_object_t* _json_array)
{
	return cJSON_GetArraySize(_json_array->cjson_item);
}

absl_json_object_t absl_json_get_array_item_cjson(absl_json_object_t* _json_array, uint32_t _index)
{
	absl_json_object_t object;

	object.cjson_item = cJSON_GetArrayItem(_json_array->cjson_item, _index);

	return object;
}

absl_json_object_t absl_json_create_object_cjson(void)
{
	absl_json_object_t json_object;

	json_object.cjson_item = cJSON_CreateObject();

	return json_object;
}

absl_json_object_t absl_json_create_array_cjson(void)
{
	absl_json_object_t json_array;

	json_array.cjson_item = cJSON_CreateArray();

	return json_array;
}

absl_json_object_t absl_json_create_string_array_cjson(char** strings, uint8_t _item_count)
{
	absl_json_object_t json_array;

	json_array.cjson_item = cJSON_CreateStringArray((const char *const *)strings, _item_count);

	return json_array;
}

void absl_json_add_string_cjson(absl_json_object_t* _json_object, char* _item, char* _item_value)
{
	cJSON_AddStringToObject(_json_object->cjson_item , _item, _item_value);
}

void absl_json_add_number_cjson(absl_json_object_t* _json_object, char* _item, double _item_value)
{
	cJSON_AddNumberToObject(_json_object->cjson_item , _item, _item_value);
}

void absl_json_add_bool_cjson(absl_json_object_t* _json_object, char* _item, bool _bool)
{
	cJSON_AddBoolToObject(_json_object->cjson_item , _item, _bool);
}

void absl_json_add_item(absl_json_object_t* _json_object, char* _item, absl_json_object_t* _json_object_to_add)
{
	cJSON_AddItemToObject(_json_object->cjson_item , _item, _json_object_to_add->cjson_item);
}

void absl_json_add_item_to_array_cjson(absl_json_object_t* _json_array, absl_json_object_t* _json_item)
{
	cJSON_AddItemToArray(_json_array->cjson_item , _json_item->cjson_item);
}

absl_json_rv_t absl_json_free_cjson(absl_json_t *_json)
{
	absl_json_rv_t json_ret_val = ABSL_JSON_RV_ERROR;

	cJSON_free(_json->json_buff);

	if(ABSL_MUTEX_RV_OK == absl_mutex_give(&_json->json_mutex))
	{
		json_ret_val = ABSL_JSON_RV_OK;
	}

	return json_ret_val;
}

void absl_json_delete_cjson(absl_json_object_t* _json_object)
{
	cJSON_Delete(_json_object->cjson_item);
}

#endif
