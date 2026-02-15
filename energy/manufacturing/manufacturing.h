/*
 * manufacturing.h
 *
 *  Created on: Dec 22, 2023
 *      Author: abolinaga
 */

#ifndef MANUFACTURING_H_
#define MANUFACTURING_H_

#include "system_types.h"

#include "pl_types.h"
#include "pl_nvm.h"

typedef enum manufacturing_events
{
	MANU_ID_NUM_INCORRECT = 0,
	MANU_MAC_INCORRECT,
	MANU_MODEL_INCORRECT,
	MANU_MODEL_EMPTY,
	MANU_BOARD_EMPTY,
	MANU_SERIAL_EMPTY,
	MANU_MANUFACTURER_EMPTY,
	MANU_VERSION_EMPTY,
	MANU_EVENTS_MAXVALUE
}manufacturing_events_t;


typedef struct manufacturing_config
{
	pl_nvm_t* 			nvm;
	uint32_t 			sector_index;
	manufacturing_t* 	default_manufacturing;
	char* 				system_model;
	void* 				manufacturing_events_info_array;
}manufacturing_config_t;

void manufacturing_initialize(manufacturing_config_t* manufacturing_config);

bool manufacturing_get_data(manufacturing_t* _manufacturing_data);

bool manufacturing_check_and_write(manufacturing_t* _manufacturing_data);

void manufacturing(manufacturing_t* _manufacturing_data);

bool manufacturing_get_mac(char* _received_mac, uint8_t	*_mac_address);

#endif /* MANUFACTURING_H_ */
