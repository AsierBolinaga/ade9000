/*
 * streaming_types.h
 *
 *  Created on: Nov 21, 2024
 *      Author: abolinaga
 */

#ifndef STREAMING_TYPES_H_
#define STREAMING_TYPES_H_

#include "absl_types.h"

typedef enum format
{
	FORMAT_BINARY = 0,
	FORMAT_JSON,
	FORMAT_UNKOWN
}format_t;

typedef enum protocol
{
	PROTCOL_UDP = 0,
	PROTCOL_TCP,
	PROTOCOL_MQTT,
	PROTOCOL_UNKNOWN
}protocol_t;

typedef struct stream_data_config
{
	format_t 		format;
	protocol_t 		protocol;
	char	 		ip[20];
	uint32_t		port;
	uint32_t		data_size;
}stream_data_config_t;

#endif /* STREAMING_TYPES_H_ */
