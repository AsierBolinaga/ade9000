/*
 * fw_update.c
 *
 *  Created on: Mar 29, 2023
 *      Author: abolinaga
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fw_update.h"

#include "interfaces.h"

#include "pl_system.h"
#include "pl_nvm.h"
#include "pl_thread.h"
#include "pl_system.h"
#include "pl_debug.h"
#include "pl_hw_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static fw_update_task_states_t 	fw_update_state;

static fw_update_thread_config_t* fw_update_config;

static pl_nvm_t		fw_update_nvm;

static uint32_t actual_fw_slot_index;
static uint32_t slot_to_update_index;

static uint32_t partition_size;

static bool 	area_is_empty;

static bool 	erase_done;
static uint32_t	erase_offset;
static uint32_t chunk_offset;
static bool 	update_error_detected;

/*******************************************************************************
 * Private functions
 ******************************************************************************/
static void fw_update_idle_state(uint32_t _events);
static void fw_update_active_state(uint32_t _events);

static void fw_update_erase_new_fw_area(void);
static void fw_update_reset_update_vars(void);
static bool fw_update_check_new_fw_area(void);
static bool fw_update_check_and_erase_new_fw_area(void);

/*******************************************************************************
 * Code
 *******************************************************************************/

bool fw_update_task_initialize(fw_update_thread_config_t* _fw_update_task_config)
{
	bool return_value = false;

	pl_nvm_config_t*	fw_update_nvm_config;

	if(NULL != _fw_update_task_config)
	{
		fw_update_config = _fw_update_task_config;
		fw_update_nvm_config = pl_config_get_nvm_conf(fw_update_config->nvm_conf_index);

		if((PL_EVENT_RV_OK == pl_event_create(fw_update_config->fw_update_event_group)) &&
		   (PL_NVM_RV_OK == pl_nvm_init(&fw_update_nvm, fw_update_nvm_config)))
        {
			if(true == fw_update_check_image(&fw_update_nvm, &fw_update_config->fw_update_handling_conf,
					   fw_update_config->fw_update_done_flag, fw_update_config->fw_update_started_flag))
			{
				actual_fw_slot_index = fw_update_get_fw_slot();
				slot_to_update_index = fw_update_get_update_slot();
				partition_size =  pl_nvm_get_size(&fw_update_nvm, actual_fw_slot_index);

				fw_update_reset_update_vars();
				area_is_empty = fw_update_check_and_erase_new_fw_area();

				if(true == area_is_empty)
				{
					fw_update_config->fw_update_initialized = true;
					return_value = true;
				}
			}
        }
	}

	return return_value;
}

/*!
 * @brief comm task entry point
 *
 * @param arg unused
 */
void fw_update_task(void *arg)
{
	uint32_t		 recieved_events;
	system_states_t* system_state = (system_states_t*)arg;

	if(!fw_update_config->fw_update_initialized)
	{
		pl_debug_printf("ERROR! fw update thread has not been initialized!\n");
		pl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	while(1)
	{
        if(PL_EVENT_RV_OK == pl_event_wait(fw_update_config->fw_update_event_group, FW_UPDATE_EVENTS, &recieved_events))
		{
            fw_update_state = fw_update_config->sensor_to_task_state[*system_state];
            switch(fw_update_state)
            {
            case FW_UPDATE_TASK_STATE_IDLE:
                fw_update_idle_state(recieved_events);
            break;

            case FW_UPDATE_TASK_STATE_ACTIVE:
                fw_update_active_state(recieved_events);
            break;

            default:
                pl_hardfault_handler(UNKNOWN_SWITCH_CASE_ERROR);
            break;
            }
        }
        else
        {
            pl_hardfault_handler(UNKNOWN_EVENT_ERROR);
        }
	}
}

static void fw_update_idle_state(uint32_t _events)
{
	if(FW_UPDATE_RESET_FW_UPDATE == (_events & FW_UPDATE_RESET_FW_UPDATE))
	{
		fw_update_reset_update_vars();
		area_is_empty = fw_update_check_and_erase_new_fw_area();
		if(true == area_is_empty)
		{
			pl_event_set(fw_update_config->system_event_group, fw_update_config->erase_done_event);
			pl_event_clear_all(fw_update_config->fw_update_event_group);
		}
	}
}

static void fw_update_active_state(uint32_t _events)
{
    uint8_t		data[500];
	uint32_t	length;

	if(true == area_is_empty)
	{
		if((FW_UPDATE_CHUNK_RECEIVED == (_events & FW_UPDATE_CHUNK_RECEIVED)) ||
		(FW_UPDATE_RECEPTION_FINISHED == (_events & FW_UPDATE_RECEPTION_FINISHED)))
		{
			if(false == update_error_detected)
			{			
				pl_nvm_rv_t nvm_write_rv;

				length = fw_update_data_get(data);
				
				if(chunk_offset == 0)
				{
					if(false == fw_update_validate_received_header(data))
					{
						update_error_detected = true;
					}
				}

				nvm_write_rv = pl_nvm_write(&fw_update_nvm, slot_to_update_index, chunk_offset, length, (void*)&data);
				if(PL_NVM_RV_ERROR == nvm_write_rv)
				{
					pl_debug_printf("\rFlash writing operation failed\n");
					update_error_detected = true;
					fw_update_notify_system_event(fw_update_config->event_info_array, FWU_EVENTS_WRITE_FAILED);
				}
				else if(PL_NVM_RV_AREA_NOT_EMPTY == nvm_write_rv)
				{
					pl_debug_printf("\rFlash writing area not empty\n");
					update_error_detected = true;
					fw_update_notify_system_event(fw_update_config->event_info_array, FWU_EVENTS_UPDATE_AREA_NOT_EMPTY);
				}

				chunk_offset += length;
				if (chunk_offset >= partition_size)
				{
					/* Partition boundary exceeded */
					pl_debug_printf("\rstore_update_image: partition boundary exceeded\n");
					update_error_detected = true;
					fw_update_notify_system_event(fw_update_config->event_info_array, FWU_EVENTS_AREA_EXCEEDED);
				}
			}
		}

		if(FW_UPDATE_RECEPTION_FINISHED == (_events & FW_UPDATE_RECEPTION_FINISHED))
		{
			if(false == update_error_detected)
			{
				pl_debug_printf("\nstore_update_image: processed %i bytes\r\n", chunk_offset);

				if( true == fw_update_validate_written_image(chunk_offset))
				{
					pl_system_set_fwu_flag(fw_update_config->fw_update_done_flag);
					pl_thread_sleep(5000);
					pl_system_reboot();
				}
				else
				{
					fw_update_reset_update_vars();
					pl_event_set(fw_update_config->system_event_group, fw_update_config->erase_needed);
				}
			}
			else
			{
				fw_update_reset_update_vars();
				pl_event_set(fw_update_config->system_event_group, fw_update_config->erase_needed);
			}
		}
	}
	else
	{
		fw_update_notify_system_event(fw_update_config->event_info_array, FWU_EVENTS_UPDATE_AREA_NOT_EMPTY);
		pl_event_set(fw_update_config->system_event_group, fw_update_config->erase_needed);
	}

	if(FW_UPDATE_RESET_FW_UPDATE == (_events & FW_UPDATE_RESET_FW_UPDATE))
	{
		fw_update_reset_update_vars();
		area_is_empty = fw_update_check_and_erase_new_fw_area();
		if(true == area_is_empty)
		{
			pl_event_set(fw_update_config->system_event_group, fw_update_config->erase_done_event);
			pl_event_clear_all(fw_update_config->fw_update_event_group);
		}
	}
}

static void fw_update_erase_new_fw_area(void)
{
	if(PL_NVM_RV_ERROR == pl_nvm_erase(&fw_update_nvm, slot_to_update_index, erase_offset, fw_update_config->erase_chunks))
	{
		pl_debug_printf("Error erasing flash area\n");
		update_error_detected = true;
		fw_update_notify_system_event(fw_update_config->event_info_array, FWU_EVENTS_ERASE_FAILED);
	}
	else
	{
		erase_offset += fw_update_config->erase_chunks;
		if(partition_size <= erase_offset)
		{
			erase_done = true;
		}
	}
}

static void fw_update_reset_update_vars(void)
{
	erase_done   = false;
	erase_offset = 0;
	chunk_offset = 0;
	update_error_detected = false;
}

static bool fw_update_check_new_fw_area(void)
{
	uint32_t read_data;

	for(uint32_t area_offset = 0; area_offset < partition_size; area_offset+=4)
	{
		pl_nvm_read(&fw_update_nvm, slot_to_update_index, area_offset, 4, (void*)&read_data);
		if(read_data != PL_NMV_EMPTY_ADRESS)
		{
			return true;
		}
	}

	return false;
}

static bool fw_update_check_and_erase_new_fw_area(void)
{
	bool area_is_empty = false;
	bool data_found = fw_update_check_new_fw_area();

	if(data_found)
	{
		pl_debug_printf("Data found in new fw sector. Erasing sector.\n");
		while((false == erase_done) && (false == update_error_detected))
		{
			fw_update_erase_new_fw_area();
		}
		pl_debug_printf("Erase done.\n");

		if(false == update_error_detected)
		{
			area_is_empty = true;
		}
	}
	else
	{
		area_is_empty = true;
	}

	return area_is_empty;
}

