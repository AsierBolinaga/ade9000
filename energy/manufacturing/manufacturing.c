/*
 * manufacturing.c
 *
 *  Created on: Dec 22, 2023
 *      Author: abolinaga
 */

#include "manufacturing.h"

#include "debug_shell.h"
#include "interfaces.h"

#include "pl_debug.h"

#include "pl_debug.h"
#include "pl_thread.h"
#include "pl_hw_config.h"
#include "pl_macros.h"
#include "pl_nvm.h"
#include "pl_gpio.h"
#include "pl_timer.h"
#include "pl_system.h"

#include "pl_hw_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEBUG_SHELL_PRIORITY 		6
#define DEBUG_SHELL_STACK_SIZE 		500

#define MANUFACTURING_READ_DATA			0x00000001
#define MANUFACTURING_WRITE_DATA		0x00000002
#define MANUFACTURING_DELETE_DATA		0x00000004
#define MANUFACTURING_DEFAULT_DATA  	0x00000008
#define MANUFACTURING_EXIT      		0x00000010

#define MANUFACTURING_EVENTS 	MANUFACTURING_READ_DATA 			| \
						        MANUFACTURING_WRITE_DATA		   	| \
						        MANUFACTURING_DELETE_DATA           | \
								MANUFACTURING_DEFAULT_DATA			| \
						        MANUFACTURING_EXIT

typedef enum manufacturing_cmds
{
	CMD_READ = 0,
	CMD_WRITE,
	CMD_DELETE,
	CMD_DEFAULT,
	CMD_EXIT,
	CMDS_MAXVALUE
}manufacturing_cmds_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static pl_event_t			manufacturing_events;

static pl_nvm_t*		    manufacturing_nvm;

static uint32_t             manufacturing_sector_index;

static pl_timer_t			manufacturing_timeout;

static manufacturing_t  	manufacturing_data;

static manufacturing_t*		default_manufacturing;
static char*				system_model;

static event_info_t* 		event_info_array;

static bool manufacturing_initialized = false;

/***************************************************************************
 * DEBUG SHELL configuration
****************************************************************************/

static debug_shell_config_t debug_shell_config =
{
	"\nManufactur>> ",
	false
};

static pl_thread_t debug_shell_thread;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void manufacturing_execute(void);

static void manufacturing_handler_show_menu(void);
static bool manufacturing_read(void);
static void manufacturing_write(void);
static void manufacturing_default(void);
static void manufacturing_delete(void);

static bool manufacturing_check_data(manufacturing_t* _manufacturing);
static bool manufacturing_value_is_empty(char* _value, uint32_t length);
static bool manufacturing_check_serial_num(char* _serial_num, uint32_t* _sn_length);
static bool manufacturing_check_mac(uint8_t* _mac);
static bool manufacturing_check_model(char* _model, uint32_t* _model_length);
static bool manufacturing_get_data_length(char* _data, uint32_t* _data_length);

static void manufacturing_read_cmd(char** _arg);
static void manufacturing_write_cmd(char** _arg);
static void manufacturing_delete_cmd(char** _arg);
static void manufacturing_default_cmd(char** _arg);
static void manufacturing_exit_cmd(char** _arg);

static void manufaturing_print_data(manufacturing_t* _data_to_print);

debug_shell_cmd_t manufacturing_cmds[CMDS_MAXVALUE] =
{
	{"read",	"\r\"read\": Read manufacturing information from memory.",
				manufacturing_read_cmd},
	{"write",	"\r\"write\": Write new manufacturing data in memory. Next commands need to be "
			    "used depending on the field we want to specify (use as many as needed).\n"
				"\t\"id\" Specify the ID number (0-0000001 to 999-999999999).\n"
				"\t\"mac\" Specify the mac address in hexadecimal (XX:XX:XX:XX:XX:XX).\n"
				"\t\"model\" Specify the model, energy, accelerometer, SARA or LARA.\n"
				"\t\"hw1board\" Specify board 1 name.\n"
				"\t\"hw1sn\" Specify board 1 serial number.\n"
				"\t\"hw1mf\" Specify the manufacturer of the board 1.\n"
				"\t\"hw1v\" Specify the version of the board 1 (vX.X.X or vX.X.X-rX).\n"
				"\t\"hw2board\" Specify board 2 name.\n"
				"\t\"hw2sn\" Specify board 2 serial number.\n"
				"\t\"hw2mf\" Specify the manufacturer of the board 2.\n"
				"\t\"hw2v\" Specify the version of the board 2 (vX.X.X or vX.X.X-rX).",
				manufacturing_write_cmd},
	{"delete",	"\r\"delete\": Delete manufacturing data from memory.\n\r",
				manufacturing_delete_cmd},
	{"default",	"\r\"default\": Write default manufacturing data.\n\r",
				manufacturing_default_cmd},
    {"exit",	"\r\"exit\": Finish manufacturing info writing process.\n\r",
				manufacturing_exit_cmd},
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static void manufacturing_timeout_call(void* arg)
{
	PL_UNUSED_ARG(arg);

	pl_event_set_fromISR(&manufacturing_events, MANUFACTURING_DEFAULT_DATA | MANUFACTURING_EXIT);
}

void manufacturing_initialize(manufacturing_config_t* _manufacturing_config)
{
	pl_time_t manufaturing_timeout_time;

	manufacturing_nvm = _manufacturing_config->nvm;
    manufacturing_sector_index = _manufacturing_config->sector_index;
    default_manufacturing = _manufacturing_config->default_manufacturing;
    system_model = _manufacturing_config->system_model;
    event_info_array = _manufacturing_config->manufacturing_events_info_array;

    pl_event_create(&manufacturing_events);

    manufaturing_timeout_time.seconds = 30;
    manufaturing_timeout_time.nseconds = 0;

    pl_timer_create(&manufacturing_timeout, &manufacturing_timeout_call, NULL, manufaturing_timeout_time, false, false);

	memset(&manufacturing_data, 0, sizeof(manufacturing_t));

    manufacturing_initialized = true;
}

bool manufacturing_get_data(manufacturing_t* _manufacturing_data)
{
    bool data_available = false;

    if(false == manufacturing_read())
	{
		pl_debug_printf("Manufacturing data is invalid\n");
	}
	else
	{
        memcpy(_manufacturing_data, &manufacturing_data, sizeof(manufacturing_t));
        data_available = true;
	}

    return data_available;
}

void manufacturing(manufacturing_t* _manufacturing_data)
{
    uint32_t events;

    bool  exit = false;

    if(true == manufacturing_initialized)
    {
        pl_timer_start(&manufacturing_timeout);
        manufacturing_execute();

        while(!exit)
        {
            pl_event_wait_freertos(&manufacturing_events, MANUFACTURING_EVENTS, &events);

            pl_timer_stop(&manufacturing_timeout);

            if(MANUFACTURING_READ_DATA == (events & MANUFACTURING_READ_DATA))
            {
                manufacturing_read();
            }
            if(MANUFACTURING_WRITE_DATA == (events & MANUFACTURING_WRITE_DATA))
            {
                manufacturing_write();
            }
            if(MANUFACTURING_DELETE_DATA == (events & MANUFACTURING_DELETE_DATA))
            {
                manufacturing_delete();
            }
			if(MANUFACTURING_DEFAULT_DATA == (events & MANUFACTURING_DEFAULT_DATA))
			{
				manufacturing_default();
			}
            if(MANUFACTURING_EXIT == (events & MANUFACTURING_EXIT))
            {
				if(false == manufacturing_read())
				{
					pl_debug_printf("Invalid manufacturing data, cannot exit until it is correctly fulfilled!\r\n");
				}
				else
				{
               	 	debug_shell_finish();
                	memcpy(_manufacturing_data, &manufacturing_data, sizeof(manufacturing_t));
                	exit = true;
				}
            }
        }
    }
    else
    {
        pl_hardfault_handler(MANU_NOT_INIT_ERROR);
    }
}

bool manufacturing_check_and_write(manufacturing_t* _manufacturing_data)
{
	bool return_value = false;

	if(true == manufacturing_check_data(_manufacturing_data))
	{
		manufacturing_data = *_manufacturing_data;
		manufacturing_write();

		return_value = true;
	}

	return return_value;
}

static void manufacturing_execute(void)
{
    if(!debug_shell_initialize(&debug_shell_config))
    {
        pl_debug_printf("Debug shell initialization failed!\r\n");
        pl_hardfault_handler(DEBUG_SHELL_NOT_INIT_ERROR);
    }

    debug_shell_add_commads(manufacturing_cmds, CMDS_MAXVALUE);

    pl_debug_printf("Spawning threads!\n");

    if (PL_THREAD_RV_OK != pl_thread_create(&debug_shell_thread, "Debug shell", debug_shell,
                        DEBUG_SHELL_PRIORITY, DEBUG_SHELL_STACK_SIZE, NULL))
    {
        pl_debug_printf("Debug shell creation failed!.\r\n");
        pl_hardfault_handler(THREAD_CREATE_ERROR);
    }

    manufacturing_handler_show_menu();
}

static void manufacturing_handler_show_menu(void)
{
    for(uint32_t commands_index = 0; commands_index < CMDS_MAXVALUE; commands_index++)
	{
		pl_debug_printf("%s", manufacturing_cmds[commands_index].descriptor);
	}
}

static bool manufacturing_read(void)
{
    bool data_available = false;

    pl_nvm_read(manufacturing_nvm, manufacturing_sector_index, 0,
                sizeof(manufacturing_t), (void*)&manufacturing_data);

	if(true == manufacturing_check_data(&manufacturing_data))
	{
		pl_debug_printf("\nActual manufacturing data:\n");
        data_available = true;
    }
	else
	{
		pl_debug_printf("\nThere is missing manufacturing data:\n");
	}

	manufaturing_print_data(&manufacturing_data);

    return data_available;
}

static void manufacturing_write(void)
{
    manufacturing_t written_manufacturing_data;

	volatile uint32_t primask = DisableGlobalIRQ();

    pl_nvm_erase(manufacturing_nvm, manufacturing_sector_index, 0, sizeof(manufacturing_t));
    pl_nvm_write(manufacturing_nvm, manufacturing_sector_index, 0,
    		     sizeof(manufacturing_t),(void*)& manufacturing_data);

	EnableGlobalIRQ(primask);

	pl_nvm_read(manufacturing_nvm, manufacturing_sector_index, 0,
	                sizeof(manufacturing_t), (void*)&written_manufacturing_data);

	pl_debug_printf("\nWritten data:\n");
	manufaturing_print_data(&written_manufacturing_data);
}

static void manufacturing_default(void)
{
	manufacturing_data = *default_manufacturing;
	manufacturing_write();
}

static void manufacturing_delete(void)
{   
    volatile uint32_t primask = DisableGlobalIRQ();

	pl_nvm_erase(manufacturing_nvm, manufacturing_sector_index, 0, sizeof(manufacturing_t));

	EnableGlobalIRQ(primask);

	manufacturing_read();
}

static bool manufacturing_check_data(manufacturing_t* _manufacturing)
{
	bool empty_or_not_valid_data_found = false;
	bool valid_data = false;

	uint32_t length;

	if(false == manufacturing_value_is_empty(_manufacturing->id_mumber, ID_NUM_SIZE))
	{
		memset(&_manufacturing->id_mumber, 0, ID_NUM_SIZE);
		empty_or_not_valid_data_found = true;
	}
	else
	{
		if(false == manufacturing_check_serial_num(_manufacturing->id_mumber, &length))
		{
			memset(&_manufacturing->id_mumber, 0, ID_NUM_SIZE);
			empty_or_not_valid_data_found = true;
			manufacturing_notify_system_event(event_info_array, MANU_ID_NUM_INCORRECT);
		}
	}

	if(false == manufacturing_value_is_empty((char*)&_manufacturing->mac_address, MAC_ADDRESS_SIZE))
	{
		memset(&_manufacturing->mac_address, 0, MAC_ADDRESS_SIZE);
		empty_or_not_valid_data_found = true;
	}
	else
	{
		if(false == manufacturing_check_mac((uint8_t*)_manufacturing->mac_address))
		{
			memset(&_manufacturing->mac_address, 0, MAC_ADDRESS_SIZE);
			empty_or_not_valid_data_found = true;
			manufacturing_notify_system_event(event_info_array, MANU_MAC_INCORRECT);
		}
	}

	if(false == manufacturing_value_is_empty((char*)&_manufacturing->model, MODEL_SIZE))
	{
		memset(&_manufacturing->model, 0, MODEL_SIZE);
		manufacturing_notify_system_event(event_info_array, MANU_MODEL_EMPTY);
	}
	else
	{
		if(false == manufacturing_check_model(_manufacturing->model, &length))
		{
			memset(&_manufacturing->model, 0, MODEL_SIZE);
			empty_or_not_valid_data_found = true;
			manufacturing_notify_system_event(event_info_array, MANU_MODEL_INCORRECT);
		}
	}

	for(uint32_t board_index = 0; board_index < BOARD_AMOUNT; board_index++)
	{
		if(false == manufacturing_value_is_empty(_manufacturing->hw_version[board_index].board, BOARD_SIZE))
		{
			memset(&_manufacturing->hw_version[board_index].board, 0, BOARD_SIZE);
			manufacturing_notify_system_event(event_info_array, MANU_BOARD_EMPTY);
		}

		if(false == manufacturing_value_is_empty(_manufacturing->hw_version[board_index].serial, SERIAL_NUM_SIZE))
		{
			memset(&_manufacturing->hw_version[board_index].serial, 0, SERIAL_NUM_SIZE);
			manufacturing_notify_system_event(event_info_array, MANU_SERIAL_EMPTY);
		}

		if(false == manufacturing_value_is_empty(_manufacturing->hw_version[board_index].manufacturer, MANUFACTURER_SIZE))
		{
			memset(&_manufacturing->hw_version[board_index].manufacturer, 0, MANUFACTURER_SIZE);
			manufacturing_notify_system_event(event_info_array, MANU_MANUFACTURER_EMPTY);
		}

		if(false == manufacturing_value_is_empty(_manufacturing->hw_version[board_index].hw_version, HW_VERSION_SIZE))
		{
			memset(&_manufacturing->hw_version[board_index].hw_version, 0, HW_VERSION_SIZE);
			manufacturing_notify_system_event(event_info_array, MANU_VERSION_EMPTY);
		}
	}


	if(false == empty_or_not_valid_data_found)
	{
		valid_data = true;
	}

	return valid_data;
}

static bool manufacturing_value_is_empty(char* _value, uint32_t length)
{
	bool data_found = false;

	for(uint8_t serial_index = 0; serial_index < length; serial_index++)
	{
		if(0xFF != _value[serial_index])
		{
			data_found = true;
		}
	}

	return data_found;
}

static bool manufacturing_check_serial_num(char* _serial_num, uint32_t* _sn_length)
{
    bool id = false;
	bool pre_dash = true;
	uint32_t count = 0;
	bool error_found = false;

	*_sn_length = 0;

	if(0 != _serial_num)
	{
		while(!error_found && !id)
		{
			if(isdigit((uint8_t)_serial_num [*_sn_length]))
			{
				if(pre_dash)
				{
					count++;
					if(count > 3)
					{
						error_found = true;
					}
					else
					{
						(*_sn_length)++;
					}
				}
				else
				{
					count++;
					if(count > 9)
					{
						error_found = true;
					}
					else
					{
						(*_sn_length)++;
					}
				}
			}
			else
			{
				if(pre_dash)
				{
					pre_dash = false;
					if('-' != _serial_num [*_sn_length])
					{
						error_found = true;
					}
					else
					{
						count = 0;
						(*_sn_length)++;
					}
				}
				else
				{
					if(count >= 6)
					{
						id = true;
					}
					else
					{
						error_found = true;
					}
				}
			}
		}
	}

	return id;
}

bool manufacturing_get_mac(char* _received_mac, uint8_t* _mac_address)
{
    bool mac_obtained = false;
	uint32_t arg_index = 0;

	if(isxdigit((uint8_t)_received_mac[0])  && isxdigit((uint8_t)_received_mac[1])  &&
	   isxdigit((uint8_t)_received_mac[3])  && isxdigit((uint8_t)_received_mac[4])  &&
	   isxdigit((uint8_t)_received_mac[6])  && isxdigit((uint8_t)_received_mac[7])  &&
	   isxdigit((uint8_t)_received_mac[9])  && isxdigit((uint8_t)_received_mac[10]) &&
	   isxdigit((uint8_t)_received_mac[12]) && isxdigit((uint8_t)_received_mac[13]) &&
	   isxdigit((uint8_t)_received_mac[15]) && isxdigit((uint8_t)_received_mac[16]) &&
	   (':' == _received_mac [2])  && (':' == _received_mac [5])  &&
	   (':' == _received_mac [8])  && (':' == _received_mac [11]) &&
	   (':' == _received_mac [14]))
	{
		for(uint32_t number_count = 0; number_count < MAC_ADDRESS_SIZE; number_count++)
		{
			char mac_number[18];
			memcpy(&mac_number, &_received_mac[arg_index], 2);
			_mac_address[number_count] = strtol(mac_number, NULL, 16);
			arg_index += 3;
		}

		mac_obtained = true;
	}

	return mac_obtained;
}

static bool manufacturing_check_mac(uint8_t* _mac)
{
    bool valid_num = true;

    PL_UNUSED_ARG(_mac);

    return valid_num;
}

static bool manufacturing_check_model(char* _model, uint32_t* _model_length)
{
    bool correct_model = false;

	*_model_length = 0;

	if(!strcmp(_model, system_model))
	{
		*_model_length = sizeof(system_model);
		correct_model = true;
	}
	else
	{
		/* not valid model */
	}

	return correct_model;
}

static bool manufacturing_get_data_length(char* _data, uint32_t* _data_length)
{
    bool data_available = false;

	*_data_length = 0;
	if(0 != _data)
	{
		while(0 != _data[*_data_length])
		{
			(*_data_length)++;
		}

		data_available = true;
	}

	return data_available;
}

static void manufacturing_write_cmd(char** _arg)
{
    uint32_t 	arg_index = 1;
	uint32_t 	data_length = 0;

    while(0 != _arg[arg_index])
	{
		if(!strcmp(_arg[arg_index], "id"))
		{
			if(manufacturing_check_serial_num(_arg[arg_index + 1], &data_length))
			{
				memset(manufacturing_data.id_mumber, 0, ID_NUM_SIZE);
				if(ID_NUM_SIZE > data_length)
				{
					memcpy(manufacturing_data.id_mumber, _arg[arg_index + 1], data_length);
				}
			}
			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "mac"))
		{
			manufacturing_get_mac(_arg[arg_index + 1], manufacturing_data.mac_address);
			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "model"))
		{
			if(manufacturing_check_model(_arg[arg_index + 1], &data_length))
			{
				if(MODEL_SIZE > data_length)
				{
					memset(&manufacturing_data.model, 0, MODEL_SIZE);
					memcpy(&manufacturing_data.model, _arg[arg_index + 1], data_length);
				}
			}
			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw1board"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(BOARD_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[0].board, 0, BOARD_SIZE);
					memcpy(&manufacturing_data.hw_version[0].board, _arg[arg_index + 1], data_length);
				}
			}
			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw1sn"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(SERIAL_NUM_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[0].serial, 0, SERIAL_NUM_SIZE);
					memcpy(&manufacturing_data.hw_version[0].serial, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw1mf"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(MANUFACTURER_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[0].manufacturer, 0, MANUFACTURER_SIZE);
					memcpy(&manufacturing_data.hw_version[0].manufacturer, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw1v"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(HW_VERSION_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[0].hw_version, 0, HW_VERSION_SIZE);
					memcpy(&manufacturing_data.hw_version[0].hw_version, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw2board"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(BOARD_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[1].board, 0, BOARD_SIZE);
					memcpy(&manufacturing_data.hw_version[1].board, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}

		else if(!strcmp(_arg[arg_index], "hw2sn"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(SERIAL_NUM_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[1].serial, 0, SERIAL_NUM_SIZE);
					memcpy(&manufacturing_data.hw_version[1].serial, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw2mf"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(MANUFACTURER_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[1].manufacturer, 0, MANUFACTURER_SIZE);
					memcpy(&manufacturing_data.hw_version[1].manufacturer, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else if(!strcmp(_arg[arg_index], "hw2v"))
		{
			if(true == manufacturing_get_data_length(_arg[arg_index + 1], &data_length))
			{
				if(HW_VERSION_SIZE > data_length)
				{
					memset(&manufacturing_data.hw_version[1].hw_version, 0, HW_VERSION_SIZE);
					memcpy(&manufacturing_data.hw_version[1].hw_version, _arg[arg_index + 1], data_length);
				}
			}

			arg_index += 2;
		}
		else
		{
			arg_index++;
		}
	}

    pl_event_set(&manufacturing_events, MANUFACTURING_WRITE_DATA);
}

static void manufacturing_read_cmd(char** _arg)
{
	PL_UNUSED_ARG(_arg);

    pl_event_set(&manufacturing_events, MANUFACTURING_READ_DATA);
}

static void manufacturing_delete_cmd(char** _arg)
{
	PL_UNUSED_ARG(_arg);

    pl_event_set(&manufacturing_events, MANUFACTURING_DELETE_DATA);
}

static void manufacturing_default_cmd(char** _arg)
{
	PL_UNUSED_ARG(_arg);

	pl_event_set(&manufacturing_events, MANUFACTURING_DEFAULT_DATA);
}

static void manufacturing_exit_cmd(char** _arg)
{
	PL_UNUSED_ARG(_arg);

	pl_event_set(&manufacturing_events, MANUFACTURING_EXIT);
}

void manufaturing_print_data(manufacturing_t* _data_to_print)
{
	pl_debug_printf("\nID number: %s\n", _data_to_print->id_mumber);
	pl_debug_printf("mac address: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", _data_to_print->mac_address[0],
																				_data_to_print->mac_address[1],
																				_data_to_print->mac_address[2],
																				_data_to_print->mac_address[3],
																				_data_to_print->mac_address[4],
																				_data_to_print->mac_address[5]);
	pl_debug_printf("Model: %s\n", _data_to_print->model);
	pl_debug_printf("\nBoard 1 data:\n board: %s\n", _data_to_print->hw_version[0].board);
	pl_debug_printf(" serial number: %s\n", _data_to_print->hw_version[0].serial);
	pl_debug_printf(" Manufacturer: %s\n", _data_to_print->hw_version[0].manufacturer);
	pl_debug_printf(" HW version: %s\n", _data_to_print->hw_version[0].hw_version);
	pl_debug_printf("\nBoard 2 data:\n board: %s\n", _data_to_print->hw_version[1].board);
	pl_debug_printf(" serial number: %s\n", _data_to_print->hw_version[1].serial);
	pl_debug_printf(" Manufacturer: %s\n", _data_to_print->hw_version[1].manufacturer);
	pl_debug_printf(" HW version: %s\n", _data_to_print->hw_version[1].hw_version);
}
