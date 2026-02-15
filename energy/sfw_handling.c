/*
 * sfw_handling.c
 *
 *  Created on: Dec 20, 2023
 *      Author: abolinaga
 */

#include "sfw_handling.h"

#include "sbl_ota_flag.h"
#include "interfaces.h"

#include "pl_debug.h"
#include "pl_system.h"
#include "pl_macros.h"

#define SFW_FW_FIRST_SLOT    	        1
#define SFW_FW_SECOND_SLOT		        2

#define SFW_REMAP_FLAG_IMAGE_POS_INDEX  -32
#define SFW_UPDATE_TYPE_INDEX           -28
#define SFW_MQTT_UPDATE_TYPE_MQTT		0x05U

typedef struct sfw_update_sbl_version 
{
    uint8_t     iv_major;
    uint8_t     iv_minor;
    uint16_t    iv_revision;
}sfw_update_sbl_version_t;

typedef struct sfw_header_image_info
{
	uint32_t                    magic;
	uint16_t                    header_size;
	uint32_t                    image_size;
	sfw_update_sbl_version_t	sbl_version;
}sfw_header_image_info;

static pl_nvm_t* fw_nvm;

static uint32_t nvm_primary_index;
static uint32_t nvm_secondary_index;

static uint32_t actual_fw_slot_index;
static uint32_t slot_to_update_index;

static sfw_header_image_info actual_image_info;

static sfw_update_sbl_version_t compatible_sbl_version = {1, 1, 0};

static event_info_t* sfw_event_info_array;

static void sfw_handling_get_image_position(void);
static void sfw_handling_get_image_info(uint32_t _image_index, sfw_header_image_info* _image_info);
static bool sfw_handling_check_update(uint32_t _fw_update_done_flag, uint32_t _fw_update_started_flag);

bool sfw_handling_check_image(pl_nvm_t*	_fw_nvm, fw_update_handling_conf_t* _sfw_handling_conf,
							  uint32_t _fw_update_done_flag, uint32_t _fw_update_started_flag)
{
    bool check_rv = false;

    if(NULL != _fw_nvm)
    {
        fw_nvm = _fw_nvm;

        nvm_primary_index = _sfw_handling_conf->nvm_primary_index;
        nvm_secondary_index = _sfw_handling_conf->nvm_secondary_index;

        sfw_event_info_array = _sfw_handling_conf->fwu_handling_events_array;

        sfw_handling_get_image_position();

        if(true == sfw_handling_check_update(_fw_update_done_flag, _fw_update_started_flag))
        {
            check_rv = true;
        }
    }

    return check_rv;
}

bool sfw_handling_validate_received_header(uint8_t* _rx_data)
{
    bool valid_data = false;

	sfw_header_image_info new_fw_info;

	memcpy(&new_fw_info.magic, &_rx_data[0], sizeof(uint32_t));
	pl_debug_printf("\nReceived magic number 0x%08x\n", new_fw_info.magic) ;

	memcpy(&new_fw_info.header_size,  &_rx_data[8], sizeof(uint16_t));
	pl_debug_printf("\nReceived header size %d\n", new_fw_info.header_size) ;

	memcpy(&new_fw_info.image_size, &_rx_data[12], sizeof(uint32_t));
	pl_debug_printf("\nReceived image size %d\n", new_fw_info.image_size);

	memcpy(&new_fw_info.sbl_version, &_rx_data[20], sizeof(sfw_update_sbl_version_t));
	pl_debug_printf("\nReceived image compatible with sbl: %d.%d.%d\r\n", new_fw_info.sbl_version.iv_major,
																          new_fw_info.sbl_version.iv_minor,
																          new_fw_info.sbl_version.iv_revision);

	if(actual_image_info.magic == new_fw_info.magic)
	{
		if(actual_image_info.header_size == new_fw_info.header_size)
		{
			if((new_fw_info.sbl_version.iv_major ==     compatible_sbl_version.iv_major) &&
			   (new_fw_info.sbl_version.iv_minor ==     compatible_sbl_version.iv_minor) &&
			   (new_fw_info.sbl_version.iv_revision ==  compatible_sbl_version.iv_revision))
			{
				valid_data = true;
			}
			else
			{
				pl_debug_printf("\nCompatible bootloader version specified in the header is different from the one in use. New image not compatible.\n");
				sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_NO_COMPATIBLE_WITH_BOOT);
			}
		}
		else
		{
			pl_debug_printf("\nIncorrect header size. New image not compatible.\n");
			sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_INCOPATIBLE_HEADER);
		}
	}
	else
	{
		pl_debug_printf("\nIncorrect magic number. New image not compatible.\n");
		sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_INCOPATIBLE_MAGIC_NUM);
	}			

	return valid_data;	
}

bool sfw_handling_validate_written_image(uint32_t _new_image_size)
{
    bool valid_fw = false;

	sfw_header_image_info new_fw_info;
	
	pl_debug_printf("\nNew Image information:\n");
	sfw_handling_get_image_info(slot_to_update_index, &new_fw_info);

	if(actual_image_info.magic == new_fw_info.magic)
	{
		if(actual_image_info.header_size == new_fw_info.header_size)
		{
			if(new_fw_info.image_size == (_new_image_size - new_fw_info.header_size - 336))
			{
				if((new_fw_info.sbl_version.iv_major ==     compatible_sbl_version.iv_major) &&
				   (new_fw_info.sbl_version.iv_minor ==     compatible_sbl_version.iv_minor) &&
				   (new_fw_info.sbl_version.iv_revision ==  compatible_sbl_version.iv_revision))
				{
					write_update_type(SFW_MQTT_UPDATE_TYPE_MQTT);
					enable_image();

					valid_fw = true;
				}
				else
				{
					pl_debug_printf("\ncompatible bootloader version specified in the header is different from the one in use. New image not compatible.\n");
					sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_NO_COMPATIBLE_WITH_BOOT);
				}
			}
			else
			{
				pl_debug_printf("\nRecieved image size and the size specified in the header are different. Corrupt image.\n");
				sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_IMAGE_SIZE_NOT_OK);
			}
		}
		else
		{
			pl_debug_printf("\nIncorrect header size. New image not compatible.\n");
			sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_INCOPATIBLE_HEADER);
		}
	}
	else
	{
		pl_debug_printf("\nIncorrect magic number. New image not compatible.\n");
		sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_INCOPATIBLE_MAGIC_NUM);
	}

	return valid_fw;
}

uint32_t sfw_handling_get_fw_slot(void)
{
    return actual_fw_slot_index;
}

uint32_t sfw_handling_get_update_slot(void)
{
    return slot_to_update_index;
}

static void sfw_handling_get_image_position(void)
{
	uint8_t image_position;

	pl_nvm_read(fw_nvm, nvm_primary_index, SFW_REMAP_FLAG_IMAGE_POS_INDEX,
                	    sizeof(uint8_t), (void*)&image_position);

	if(SFW_FW_FIRST_SLOT == image_position)
	{
		actual_fw_slot_index = nvm_primary_index;
		slot_to_update_index = nvm_secondary_index;
	}
	else if(SFW_FW_SECOND_SLOT == image_position)
	{
		actual_fw_slot_index = nvm_secondary_index;
		slot_to_update_index = nvm_primary_index;
	}
	else
	{
		sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_NO_SLOT_INFO);
		actual_fw_slot_index = nvm_primary_index;
		slot_to_update_index = nvm_secondary_index;
	}

	sfw_handling_get_image_info(actual_fw_slot_index, &actual_image_info);
}

static void sfw_handling_get_image_info(uint32_t _image_index, sfw_header_image_info* _image_info)
{
	pl_nvm_read(fw_nvm, _image_index, 0, sizeof(uint32_t), (void*)&_image_info->magic);
	pl_debug_printf("\nMagic number 0x%08x\n", _image_info->magic) ;

	pl_nvm_read(fw_nvm, _image_index, 8, sizeof(uint16_t), (void*)&_image_info->header_size);
	pl_debug_printf("\nHeader size %d\n", _image_info->header_size) ;

	pl_nvm_read(fw_nvm, _image_index, 12, sizeof(uint32_t), (void*)&_image_info->image_size);
	pl_debug_printf("\nImage size %d\n", _image_info->image_size);

	pl_nvm_read(fw_nvm, _image_index, 20, sizeof(sfw_update_sbl_version_t), (void*)&_image_info->sbl_version);
	pl_debug_printf("\nImage compatible with sbl: %d.%d.%d\r\n", _image_info->sbl_version.iv_major,
																 _image_info->sbl_version.iv_minor,
																 _image_info->sbl_version.iv_revision);
}

static bool sfw_handling_check_update(uint32_t _fw_update_done_flag, uint32_t _fw_update_started_flag)
{
	bool 		update_check_ok = false;
	uint8_t 	last_update_type;
	uint32_t    fw_update_flag_value = pl_system_get_fwu_flag();

	pl_nvm_read(fw_nvm, nvm_primary_index, SFW_UPDATE_TYPE_INDEX,
				sizeof(uint8_t), (void*)&last_update_type);

	if(last_update_type == SFW_MQTT_UPDATE_TYPE_MQTT)
	{
		pl_debug_printf("Update done!\r\n");
        
		sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_FWU_DETECTED);

        write_image_ok();
        update_check_ok = true;
	}
	else if((0 == last_update_type) || (0xFF == last_update_type))
	{
		pl_debug_printf("No update detected.\r\n");
        
		if(fw_update_flag_value == _fw_update_done_flag)
		{
			sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_FWU_DONE_FAILED);
		}
		else if(fw_update_flag_value == _fw_update_started_flag)
		{
			sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_FWU_STARTED_FAILED);
		}
		else
		{
			sfw_notify_system_event((void*)sfw_event_info_array, SFW_HANDLING_NO_FWU_DETECTED);
		}

		update_check_ok = true;
	}
	else
	{
		pl_debug_printf("ERROR! Unknown update type. FW was not done with using a valid method!\r\n");
		pl_debug_printf("Reboot without ok flag to go back to the previous image!\r\n");
		pl_system_reboot();
	}

	pl_system_get_fwu_clear_flag();

	return update_check_ok;
}
