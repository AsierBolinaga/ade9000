/*
 * pl_json.h
 *
 *  Created on: Apr 4, 2025
 *      Author: abolinaga
 */

#ifndef PL_JSON_H_
#define PL_JSON_H_

#include "pl_config.h"
#ifdef PL_JSON

#include "pl_types.h"

#include "pl_mutex.h"

#ifdef PL_CJSON
#include "cJSON.h"
#endif

typedef enum pl_json_rv
{
    PL_JSON_RV_OK = 0x0U,
	PL_JSON_RV_ERROR
} pl_json_rv_t;

typedef struct pl_json_object
{
#if defined(PL_CJSON)
	cJSON* 		cjson_item;
#endif
}pl_json_object_t;

typedef struct pl_json
{
	char* 		json_buff;
	pl_mutex_t 	json_mutex;
#if defined(PL_CJSON)
	cJSON* 		cjson_item;
#endif
}pl_json_t;

#if defined(PL_CJSON)
#define pl_json_init						pl_json_init_cjson
#define pl_json_parse						pl_json_parse_cjson
#define pl_json_get_buff					pl_json_get_buff_cjson
#define pl_json_get_item_string_value		pl_json_get_item_string_value_cjson
#define pl_json_get_item_object				pl_json_get_item_object_cjson
#define pl_json_get_array_size				pl_json_get_array_size_cjson
#define pl_json_get_array_item				pl_json_get_array_item_cjson
#define pl_json_create_object				pl_json_create_object_cjson
#define pl_json_create_array				pl_json_create_array_cjson
#define pl_json_create_string_array			pl_json_create_string_array_cjson
#define pl_json_add_string					pl_json_add_string_cjson
#define pl_json_add_number					pl_json_add_number_cjson
#define pl_json_add_bool					pl_json_add_bool_cjson
#define pl_json_add_item					pl_json_add_item_cjson
#define pl_json_add_item_to_array			pl_json_add_item_to_array_cjson
#define pl_json_free						pl_json_free_cjson
#define pl_json_delete						pl_json_delete_cjson
#elif defined(PL_BEAGLEBONE)
#else
#error Json parser not defined
#endif

pl_json_rv_t pl_json_init(pl_json_t *_json);

pl_json_rv_t pl_json_parse(char* _buff, pl_json_object_t* _json_object);

char* pl_json_get_buff(pl_json_t*_json, pl_json_object_t* _json_object);

char* pl_json_get_item_string_value(pl_json_t *_json, pl_json_object_t* _json_object, char* _item);

pl_json_object_t pl_json_get_item_object(pl_json_object_t* _json_object, char* _item);

uint32_t pl_json_get_array_size(pl_json_object_t* _json_array);

pl_json_object_t pl_json_get_array_item(pl_json_object_t* _json_array, uint32_t _index);

pl_json_object_t pl_json_create_object(void);

pl_json_object_t pl_json_create_array(void);

pl_json_object_t pl_json_create_string_array(char** strings, uint8_t _item_count);

void pl_json_add_string(pl_json_object_t* _json_object, char* _item, char* _item_value);

void pl_json_add_number(pl_json_object_t* _json_object, char* _item, double _item_value);

void pl_json_add_bool(pl_json_object_t* _json_object, char* _item, bool _bool);

void pl_json_add_item(pl_json_object_t* _json_object, char* _item, pl_json_object_t* _json_object_to_add);

void pl_json_add_item_to_array(pl_json_object_t* _json_array, pl_json_object_t* _json_item);

pl_json_rv_t pl_json_free(pl_json_t *_json);

void pl_json_delete(pl_json_object_t* _json_object);

#endif
#endif /* PORTABILITY_LAYER_PL_JSON_H_ */
