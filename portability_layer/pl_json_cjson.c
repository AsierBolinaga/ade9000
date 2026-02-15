
#include "pl_json.h"
#ifdef PL_CJSON

pl_json_rv_t pl_json_init_cjson(pl_json_t *_json)
{
	pl_json_rv_t json_ret_val = PL_JSON_RV_ERROR;

	cJSON_Hooks json_hooks;

	json_hooks.malloc_fn = pvPortMalloc;
	json_hooks.free_fn = vPortFree;

	cJSON_InitHooks(&json_hooks);

	if(PL_MUTEX_RV_OK == pl_mutex_create(&_json->json_mutex))
	{
		json_ret_val = PL_JSON_RV_OK;
	}

	return json_ret_val;
}

pl_json_rv_t pl_json_parse_cjson(char* _buff, pl_json_object_t* _json_object)
{
	pl_json_rv_t json_ret_val = PL_JSON_RV_ERROR;

	_json_object->cjson_item = cJSON_Parse(_buff);

	if(NULL != _json_object->cjson_item)
	{
		json_ret_val = PL_JSON_RV_OK;
	}

	return json_ret_val;
}

char* pl_json_get_buff_cjson(pl_json_t *_json, pl_json_object_t* _json_object)
{
	pl_mutex_take(&_json->json_mutex);

	_json->json_buff = cJSON_Print(_json_object->cjson_item);

	cJSON_Delete(_json_object->cjson_item);

	return _json->json_buff;
}

char* pl_json_get_item_string_value_cjson(pl_json_t *_json, pl_json_object_t* _json_object, char* _item)
{
	char* item_value = NULL;

	_json->cjson_item = cJSON_GetObjectItem(_json_object->cjson_item, _item);
	if(_json->cjson_item != NULL)
	{
		item_value = _json->cjson_item->valuestring;
	}

	return item_value;
}

pl_json_object_t pl_json_get_item_object_cjson(pl_json_object_t* _json_object, char* _item)
{
	pl_json_object_t object;

	object.cjson_item = cJSON_GetObjectItem(_json_object->cjson_item, _item);

	return object;
}

uint32_t pl_json_get_array_size_cjson(pl_json_object_t* _json_array)
{
	return cJSON_GetArraySize(_json_array->cjson_item);
}

pl_json_object_t pl_json_get_array_item_cjson(pl_json_object_t* _json_array, uint32_t _index)
{
	pl_json_object_t object;

	object.cjson_item = cJSON_GetArrayItem(_json_array->cjson_item, _index);

	return object;
}

pl_json_object_t pl_json_create_object_cjson(void)
{
	pl_json_object_t json_object;

	json_object.cjson_item = cJSON_CreateObject();

	return json_object;
}

pl_json_object_t pl_json_create_array_cjson(void)
{
	pl_json_object_t json_array;

	json_array.cjson_item = cJSON_CreateArray();

	return json_array;
}

pl_json_object_t pl_json_create_string_array_cjson(char** strings, uint8_t _item_count)
{
	pl_json_object_t json_array;

	json_array.cjson_item = cJSON_CreateStringArray((const char *const *)strings, _item_count);

	return json_array;
}

void pl_json_add_string_cjson(pl_json_object_t* _json_object, char* _item, char* _item_value)
{
	cJSON_AddStringToObject(_json_object->cjson_item , _item, _item_value);
}

void pl_json_add_number_cjson(pl_json_object_t* _json_object, char* _item, double _item_value)
{
	cJSON_AddNumberToObject(_json_object->cjson_item , _item, _item_value);
}

void pl_json_add_bool_cjson(pl_json_object_t* _json_object, char* _item, bool _bool)
{
	cJSON_AddBoolToObject(_json_object->cjson_item , _item, _bool);
}

void pl_json_add_item(pl_json_object_t* _json_object, char* _item, pl_json_object_t* _json_object_to_add)
{
	cJSON_AddItemToObject(_json_object->cjson_item , _item, _json_object_to_add->cjson_item);
}

void pl_json_add_item_to_array_cjson(pl_json_object_t* _json_array, pl_json_object_t* _json_item)
{
	cJSON_AddItemToArray(_json_array->cjson_item , _json_item->cjson_item);
}

pl_json_rv_t pl_json_free_cjson(pl_json_t *_json)
{
	pl_json_rv_t json_ret_val = PL_JSON_RV_ERROR;

	cJSON_free(_json->json_buff);

	if(PL_MUTEX_RV_OK == pl_mutex_give(&_json->json_mutex))
	{
		json_ret_val = PL_JSON_RV_OK;
	}

	return json_ret_val;
}

void pl_json_delete_cjson(pl_json_object_t* _json_object)
{
	cJSON_Delete(_json_object->cjson_item);
}

#endif
