/*
 * sfw_handling.h
 *
 *  Created on: Dec 20, 2023
 *      Author: abolinaga
 */

#ifndef SFW_HANDLING_H_
#define SFW_HANDLING_H_

#include "system_types.h"

#include "pl_nvm.h"

typedef enum sfw_handling_events
{
	SFW_HANDLING_FWU_DETECTED = 0,
	SFW_HANDLING_NO_FWU_DETECTED,
	SFW_HANDLING_NO_COMPATIBLE_WITH_BOOT,
	SFW_HANDLING_IMAGE_SIZE_NOT_OK,
	SFW_HANDLING_INCOPATIBLE_HEADER,
	SFW_HANDLING_INCOPATIBLE_MAGIC_NUM,
	SFW_HANDLING_NO_SLOT_INFO,
	SFW_HANDLING_FWU_DONE_FAILED,
	SFW_HANDLING_FWU_STARTED_FAILED,
#ifdef DEBUG_ALERTS
	SFW_HANDLING_DEBUG,
#endif
	SFW_HANDLING_EVENTS_MAXVALUE
}sfw_handling_events_t;

bool sfw_handling_check_image(pl_nvm_t*	_fw_nvm, fw_update_handling_conf_t* _sfw_handling_conf,
							  uint32_t _fw_update_done_flag, uint32_t _fw_update_started__flag);
bool sfw_handling_validate_received_header(uint8_t* _rx_data);
bool sfw_handling_validate_written_image(uint32_t _new_image_size);

uint32_t sfw_handling_get_fw_slot(void);
uint32_t sfw_handling_get_update_slot(void);

#endif /* SFW_HANDLING_H_ */
