/*
 * sensors_types.h
 *
 *  Created on: Dec 12, 2023
 *      Author: abolinaga
 */

#ifndef SENSORS_TYPES_H_
#define SENSORS_TYPES_H_

#include "absl_types.h"

#include "ade9000_types.h"

#define SENSOR_NAME					"ade9000"

#define ADE9000_SERVICE_RAW			(char*)"raw"
#define ADE9000_SERVICE_REGISTERS	"registers"

typedef enum sensors
{
	SENSOR_ADE9000 = 0,
	SENSOR_MAXVALUE 		// used also to indicate that this is no sensor event
}sensors_t;

typedef enum services
{
	SERVICE_RAW = 0,
	SERVICE_REGISTERS,
	SERVICE_MAXVALUE		// used also to indicate that this is no service event
}services_t;

#endif /* SENSORS_TYPES_H_ */
