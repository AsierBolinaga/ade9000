/*
 * absl_json.h
 *
 *  Created on: Apr 4, 2025
 *      Author: abolinaga
 */

#ifndef ABSL_JSON_H_
#define ABSL_JSON_H_

#include "absl_config.h"
#ifdef ABSL_JSON

#include "absl_types.h"

#include "absl_mutex.h"

#ifdef ABSL_CJSON
#include "cJSON.h"
#endif

typedef enum absl_json_rv
{
    ABSL_JSON_RV_OK = 0x0U,
	ABSL_JSON_RV_ERROR
} absl_json_rv_t;

typedef struct absl_json_object
{
#if defined(ABSL_CJSON)
	cJSON* 		cjson_item;
#endif
}absl_json_object_t;

typedef struct absl_json
{
	char* 		json_buff;
	absl_mutex_t 	json_mutex;
#if defined(ABSL_CJSON)
	cJSON* 		cjson_item;
#endif
}absl_json_t;

#if defined(ABSL_CJSON)
#define absl_json_init						absl_json_init_cjson
#define absl_json_parse						absl_json_parse_cjson
#define absl_json_get_buff					absl_json_get_buff_cjson
#define absl_json_get_item_string_value		absl_json_get_item_string_value_cjson
#define absl_json_get_item_object				absl_json_get_item_object_cjson
#define absl_json_get_array_size				absl_json_get_array_size_cjson
#define absl_json_get_array_item				absl_json_get_array_item_cjson
#define absl_json_create_object				absl_json_create_object_cjson
#define absl_json_create_array				absl_json_create_array_cjson
#define absl_json_create_string_array			absl_json_create_string_array_cjson
#define absl_json_add_string					absl_json_add_string_cjson
#define absl_json_add_number					absl_json_add_number_cjson
#define absl_json_add_bool					absl_json_add_bool_cjson
#define absl_json_add_item					absl_json_add_item_cjson
#define absl_json_add_item_to_array			absl_json_add_item_to_array_cjson
#define absl_json_free						absl_json_free_cjson
#define absl_json_delete						absl_json_delete_cjson
#elif defined(ABSL_BEAGLEBONE)
#else
#error Json parser not defined
#endif

absl_json_rv_t absl_json_init(absl_json_t *_json);

absl_json_rv_t absl_json_parse(char* _buff, absl_json_object_t* _json_object);

char* absl_json_get_buff(absl_json_t*_json, absl_json_object_t* _json_object);

char* absl_json_get_item_string_value(absl_json_t *_json, absl_json_object_t* _json_object, char* _item);

absl_json_object_t absl_json_get_item_object(absl_json_object_t* _json_object, char* _item);

uint32_t absl_json_get_array_size(absl_json_object_t* _json_array);

absl_json_object_t absl_json_get_array_item(absl_json_object_t* _json_array, uint32_t _index);

absl_json_object_t absl_json_create_object(void);

absl_json_object_t absl_json_create_array(void);

absl_json_object_t absl_json_create_string_array(char** strings, uint8_t _item_count);

void absl_json_add_string(absl_json_object_t* _json_object, char* _item, char* _item_value);

void absl_json_add_number(absl_json_object_t* _json_object, char* _item, double _item_value);

void absl_json_add_bool(absl_json_object_t* _json_object, char* _item, bool _bool);

void absl_json_add_item(absl_json_object_t* _json_object, char* _item, absl_json_object_t* _json_object_to_add);

void absl_json_add_item_to_array(absl_json_object_t* _json_array, absl_json_object_t* _json_item);

absl_json_rv_t absl_json_free(absl_json_t *_json);

void absl_json_delete(absl_json_object_t* _json_object);

#endif
#endif /* PORTABILITY_LAYER_ABSL_JSON_H_ */
