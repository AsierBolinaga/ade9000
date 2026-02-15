/*
 * events.h
 *
 *  Created on: Jun 29, 2023
 *      Author: abolinaga
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include "system_types.h"

/*************************************************************************/
/*							ENERGY EVENTS       					     */
/*************************************************************************/

#include "energy.h"

static error_code_t e_read_vars_event_timeout_error_code =
{CODE_TO_MANY_READ_EVENT_TIMEOUT,	"To much time without IRQ, energy sensor will be reconfigured and restarted."};

static error_code_t e_reset_not_accepted =
{CODE_ERROR_RESET_NOT_ACCEPTED,     "ADE9000 reset configuration command cannot be used when the sensor is not configured."};

static error_code_t e_start_raw_not_accepted =
{CODE_ERROR_START_NOT_ACCEPTED,     "Start raw command cannot be used when the sensor is not configured."};

static error_code_t e_stop_raw_not_accepted =
{CODE_ERROR_STOP_NOT_ACCEPTED,     "Stop raw command cannot be used when the sensor is not configured."};

static error_code_t e_start_register_not_accepted =
{CODE_ERROR_START_NOT_ACCEPTED,     "Start register command cannot be used when the sensor is not configured."};

static error_code_t e_stop_register_not_accepted =
{CODE_ERROR_STOP_NOT_ACCEPTED,     "Stop register command cannot be used when the sensor is not configured."};

static error_code_t e_reconfigured =
{CODE_ERROR_RECONFIGURED,    		"ADE9000 has been reconfigured."};

static error_code_t e_irq_not_working =
{CODE_ADE9000_IRQ_NOT_WORKING,    	"ADE9000 irq not working."};

static error_code_t e_fast_vars_could_not_start =
{CODE_FAST_VARS_COULD_NOT_START,    "Fast vars could not start."};

static error_code_t e_slow_vars_could_not_start =
{CODE_SLOW_VARS_COULD_NOT_START,    "Slow vars could not start."};

static error_code_t e_fast_vars_already_running =
{CODE_SERVICE_ALREADY_RUNNING,      "Raw service already running."};

static error_code_t e_slow_vars_already_running =
{CODE_SERVICE_ALREADY_RUNNING,      "Registers service already running."};

static error_code_t e_fast_vars_already_stopped =
{CODE_SERVICE_ALREADY_STOPPED,      "Raw service already stopped."};

static error_code_t e_slow_vars_already_stopped =
{CODE_SERVICE_ALREADY_STOPPED,      "Registers service already stopped."};

static event_info_t e_event_info[E_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_MAXVALUE,  &e_read_vars_event_timeout_error_code},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_MAXVALUE,  &e_reset_not_accepted},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW,  	 &e_start_raw_not_accepted},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW,  	 &e_stop_raw_not_accepted},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &e_start_register_not_accepted},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &e_stop_register_not_accepted},
	{EVENT_TYPE_INFO, 	 SENSOR_ADE9000,  SERVICE_MAXVALUE,  &e_reconfigured},
	{EVENT_TYPE_ERROR,   SENSOR_ADE9000,  SERVICE_MAXVALUE,  &e_irq_not_working},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, 	     &e_fast_vars_could_not_start},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &e_slow_vars_could_not_start},
	{EVENT_TYPE_INFO,    SENSOR_ADE9000,  SERVICE_RAW, 	     &e_fast_vars_already_running},
	{EVENT_TYPE_INFO,    SENSOR_ADE9000,  SERVICE_REGISTERS, &e_slow_vars_already_running},
	{EVENT_TYPE_INFO,    SENSOR_ADE9000,  SERVICE_RAW, 	     &e_fast_vars_already_stopped},
	{EVENT_TYPE_INFO,    SENSOR_ADE9000,  SERVICE_REGISTERS, &e_slow_vars_already_stopped}
};

/*************************************************************************/
/*							ADE9000 EVENTS       					     */
/*************************************************************************/

#include "ade9000.h"

static error_code_t ade9000_reconfig_error_code =
{CODE_ADE9000_RECONFIG, 			"ADE9000 reconfiguration error"};

static error_code_t ade9000_register_read_error_code =
{CODE_ADE9000_REGISTER_READ_ERROR, "ADE9000 register read error."};

static error_code_t ade9000_register_write_error_code =
{CODE_ADE9000_REGISTER_WRITE_ERROR, "ADE9000 register write error."};

static error_code_t ade9000_burst_error_code =
{CODE_ADE9000_BURST_READ_ERROR, 	"ADE9000 burst error."};

static error_code_t ade9000_wv_data_lost_error_code =
{CODE_ADE9000_WAVEFORM_DATA_LOST,	"ADE9000 too much time between waveform data reads detected, possible data lost."};

static error_code_t ade9000_reg_data_lost_error_code =
{CODE_ADE9000_REGISTERS_DATA_LOST, "ADE9000 too much time between registers reads detected, possible data lost."};

static error_code_t ade9000_incorrect_fund_freq =
{CODE_ADE9000_INVALID_FUND_FREQ, 	"Received fundamental frequency is invalid. It should be 50 or 60 Hz."};

static error_code_t ade9000_register_not_written =
{CODE_ADE9000_REGISTER_NOT_WRITTEN, "Value was not written in register. Make sure that the ADE9000 is correctly connected."};

static error_code_t ade9000_incorrect_hw_config =
{CODE_ADE9000_INVALID_HW_CONFIG, 	"Received hw config is invalid."};

static error_code_t ade9000_incorrect_tc_values =
{CODE_ADE9000_INVALID_TC_VALUES, 	"Received TC values are invalid. Make sure that configured TC values are not 0."};

static error_code_t ade9000_unknown_reg_type =
{CODE_UNKNOWN_REG_TYPE, 			"Unknown slow register type to read."};

static error_code_t ade9000_incorrect_fund_freq_error =
{CODE_INCORRECT_FUND_FREQ_ERROR, 	"Missing fundamental frequency configuration."};

static error_code_t ade9000_incorrect_tc_error =
{CODE_INCORRECT_TC_ERROR, 			"Missing tc primary and secondary configuration."};

static error_code_t ade9000_register_not_found =
{CODE_INCORRECT_REG_NOT_FOUND, 		"One of the received registers is not configurable in this service."};

static error_code_t ade9000_register_not_read =
{CODE_REGISTERS_NOT_READ, 			"Some of the registers could not be read. Registers service data not send."};

static error_code_t ade9000_register_read_long_time =
{CODE_REGISTERS_READ_LONG_TIME, 	"Reading the registers took longer than expected."};

static event_info_t ade9000_event_info[ADE9000_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_MAXVALUE, 	&ade9000_reconfig_error_code},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE,SERVICE_MAXVALUE, 	&ade9000_register_read_error_code},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE,SERVICE_MAXVALUE, 	&ade9000_register_write_error_code},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE,SERVICE_RAW,  	 	&ade9000_burst_error_code},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_RAW,  	  	&ade9000_wv_data_lost_error_code},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_REGISTERS, &ade9000_reg_data_lost_error_code},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_MAXVALUE,  &ade9000_incorrect_fund_freq},
	{EVENT_TYPE_ERROR,   SENSOR_ADE9000, SERVICE_MAXVALUE,  &ade9000_register_not_written},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_MAXVALUE,  &ade9000_incorrect_hw_config},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_MAXVALUE,  &ade9000_incorrect_tc_values},
	{EVENT_TYPE_ERROR,   SENSOR_ADE9000, SERVICE_REGISTERS, &ade9000_unknown_reg_type},
	{EVENT_TYPE_ERROR,   SENSOR_ADE9000, SERVICE_MAXVALUE, 	&ade9000_incorrect_fund_freq_error},
	{EVENT_TYPE_ERROR,   SENSOR_ADE9000, SERVICE_MAXVALUE, 	&ade9000_incorrect_tc_error},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_REGISTERS, &ade9000_register_not_found},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000, SERVICE_REGISTERS, &ade9000_register_not_read},
	{EVENT_TYPE_INFO, 	 SENSOR_ADE9000, SERVICE_REGISTERS, &ade9000_register_read_long_time}
};

/*************************************************************************/
/*							CMD_TASK EVENTS       					     */
/*************************************************************************/

#include "cmd_task.h"

static error_code_t cmd_error_sending_discovery =
{CODE_ERROR_SENDING_DISCOVERY, 		"Error sending discovery."};

static error_code_t cmd_error_sending_status =
{CODE_ERROR_SENDING_STATUS, 		"Error sending status."};

static error_code_t cmd_invalid_command =
{CODE_ERROR_INVALID_CMD, 		    "Unknown mqtt command received."};

static error_code_t cmd_update_incorrect_state =
{CODE_ERROR_UPDATE_INCORRECT_STATE, "Complete update received without going to fw update state."};

static error_code_t cmd_error_getting_cmd =
{CODE_ERROR_GETTING_CMD,            "Error getting mqtt command."};

static error_code_t cmd_full_rx_buffer =
{CODE_MQTT_RX_BUFF_FULL,            "MQTT reception buffer is full, command may have been lost."};

static error_code_t cmd_incorrect_manu_data =
{CODE_INCORRECT_MANU_DATA,          "Incorrect manufacturing data received. New data not stored in flash."};

static error_code_t cmd_state_change_data_timeout =
{CODE_STATE_CHNAGE_DATA_RX_TIMEOUT,  "No available state change data to change to online state."};

static error_code_t cmd_invalid_config_payload =
{CODE_ERROR_INVALID_CONFIG_PAYLOD, 	 "Received sensor configuration mqtt command has incorrect payload."};

static error_code_t cmd_invalid_start_payload =
{CODE_ERROR_INVALID_START_PAYLOD, 	 "Received service start mqtt command has incorrect payload."};

static error_code_t cmd_invalid_sync_payload =
{CODE_ERROR_INVALID_SYNC_PAYLOD, 	 "Received synchronization mqtt command has incorrect payload."};

static error_code_t cmd_invalid_manu_payload =
{CODE_ERROR_INVALID_MANU_PAYLOD, 	 "Received manufacturing mqtt command has incorrect payload."};

static event_info_t cmd_event_info[CMD_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_ERROR,   	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_error_sending_discovery},
    {EVENT_TYPE_ERROR,   	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_error_sending_status},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_invalid_command},
    {EVENT_TYPE_ERROR,   	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_update_incorrect_state},
    {EVENT_TYPE_ERROR,   	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_error_getting_cmd},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_full_rx_buffer},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_incorrect_manu_data},
    {EVENT_TYPE_ERROR, 	 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_state_change_data_timeout},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_invalid_config_payload},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_invalid_start_payload},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_invalid_sync_payload},
    {EVENT_TYPE_WARNING, 	SENSOR_MAXVALUE, SERVICE_MAXVALUE, &cmd_invalid_manu_payload}
};

/*************************************************************************/
/*							FW_UPDATE EVENTS       					     */
/*************************************************************************/

#include "fw_update.h"

static error_code_t fwu_update_flash_erase_failed =
{CODE_FLASH_ERASE_FAILED, 				"Flash erase operation failed while FW update."};

static error_code_t fwu_update_flash_write_failed =
{CODE_FLASH_WRITING_FAILED, 			"Flash writing operation failed while FW update."};

static error_code_t fwu_update_flash_area_exceeded =
{CODE_FLASH_AREA_EXCEEDED, 				"Flash partition boundary exceeded while FW update."};

static error_code_t fwu_are_not_empty =
{CODE_FW_UPDATE_AREA_NOT_EMPTY, 		"Update area not empty. New FW could not be written."};

static event_info_t fwu_event_info[FWU_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE, SERVICE_MAXVALUE, &fwu_update_flash_erase_failed},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE, SERVICE_MAXVALUE, &fwu_update_flash_write_failed},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &fwu_update_flash_area_exceeded},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &fwu_are_not_empty}
};

/*************************************************************************/
/*							SFW_HANDLING EVENTS       					     */
/*************************************************************************/

#include "sfw_handling.h"

static error_code_t sfw_update_detected =
{CODE_FW_DETECTED, 						"FW update done."};

static error_code_t sfw_no_update_detected =
{CODE_NO_FW_DETECTED, 					"No FW update detected."};

static error_code_t sfw_image_incopatible_boot =
{CODE_FW_UPDATE_INCOPATIBLE_BOOT, 		"New fw is not compatible with the version of the bootloader in use. Update not done."};

static error_code_t sfw_image_size_not_ok =
{CODE_FW_UPDATE_IMAGE_SIZE_NOK, 		"New fw written size and the specified in header are not equal. Corrupted image, update not done."};

static error_code_t sfw_incopatible_header =
{CODE_FW_UPDATE_INCOPATIBLE_HEADER, 	"New fw has incompatible header size. Update not done."};

static error_code_t sfw_incopatible_magic_numer =
{CODE_FW_UPDATE_INCOPATIBLE_MAGIC_NUM,  "New fw has incompatible magic number. Update not done."};

static error_code_t sfw_unknown_slot =
{CODE_UNKNOWN_FW_SLOT,  				"No slot number detected, assumed that FW is running in slot number one."};

static error_code_t sfw_update_done_fail =
{CODE_FWU_DONE_FAIL, 					"FW UPDATE FAILURE! New FW was successfully written, but SBL did not make the update, please try again."};

static error_code_t sfw_update_started_fail =
{CODE_FWU_STARTED_FAIL, 				"FW UPDATE FAILURE! System was restarted without correctly writing the new FW, please try again."};

#ifdef DEBUG_ALERTS
static error_code_t sfw_debug_alert =
{CODE_DEBUG_ALERT, 						"Debug sfw alert!"};
#endif

static event_info_t sfw_event_info[SFW_HANDLING_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_INFO,    SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_update_detected},
	{EVENT_TYPE_INFO, 	 SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_no_update_detected},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_image_incopatible_boot},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_image_size_not_ok},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_incopatible_header},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_incopatible_magic_numer},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_unknown_slot},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_update_done_fail},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_update_started_fail},
#ifdef DEBUG_ALERTS
	{EVENT_TYPE_INFO, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &sfw_debug_alert}
#endif
};

/*************************************************************************/
/*						   FAST VARS READ EVENTS       					 */
/*************************************************************************/
#include "vars_read.h"

static error_code_t vt_fast_queue_full =
{CODE_FAST_VARS_QUEUE_FULL,    		"Fast queue full. Data lost!"};

static error_code_t vt_invalid_fast_vars_config =
{CODE_INVALID_VARS_CONFIG,    		"Fast vars configuration is incorrect. Service not started"};

static event_info_t vr_fast_event_info[VR_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &vt_fast_queue_full},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &vt_invalid_fast_vars_config}
};

/*************************************************************************/
/*						   SLOW VARS READ EVENTS       					 */
/*************************************************************************/
static error_code_t vt_slow_queue_full =
{CODE_SLOW_VARS_QUEUE_FULL,    		"slow queue full. Data lost!"};

static error_code_t vt_invalid_slow_vars_config =
{CODE_INVALID_VARS_CONFIG,    		"Slow vars configuration is incorrect. Service not started"};

static event_info_t vr_slow_event_info[VR_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &vt_slow_queue_full},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &vt_invalid_slow_vars_config}
};

/*************************************************************************/
/*						STREAMING EVENTS       					     */
/*************************************************************************/

#include "streaming_task.h"

static error_code_t st_unknown_protcol =
{CODE_UNKNOWN_PROTOCOL,    		"Configured protocol is unknown or not supported."};

static error_code_t st_unknown_format =
{CODE_UNKNOWN_FORMAT,    		"Configured format is unknown or not supported."};

static error_code_t st_udp_not_send =
{CODE_UDP_NOT_SEND,    	    	"UDP data could not be send."};

static error_code_t st_udp_sending_error =
{CODE_UDP_SENDING_ERROR,    	"UDP data sending error."};

static error_code_t st_tcp_not_send =
{CODE_TCP_NOT_SEND,    	    	"TCP data could not be send."};

static error_code_t st_tcp_sending_error =
{CODE_TCP_SENDING_ERROR,    	"TCP data sending error."};

static error_code_t st_trying_to_read_with_no_prot =
{CODE_TRYING_TO_READ_WITH_NO_PROT,  "Trying to read sensor data, with no protocol defined."};

static error_code_t st_could_not_connect_tcp =
{CODE_NOT_CONNTECTED_WITH_TCP_SERVER,  "Could not connect with TCP server."};

static error_code_t st_could_not_reconnect_tcp =
{CODE_NOT_RECONNTECTED_WITH_TCP_SERVER,  "Could not reconnect with TCP server, make sure there is a source."};

static error_code_t st_incorrect_ip =
{CODE_INCORRECT_IP_CONFIGURED,  "Incorrect IP configured. Make sure configured IP is not localhost or the IP of the remote sensor."};

static event_info_t st_fast_event_info[ST_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_unknown_protcol},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_unknown_format},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_udp_not_send},
    {EVENT_TYPE_ERROR,   SENSOR_ADE9000,  SERVICE_RAW, &st_udp_sending_error},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_tcp_not_send},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_tcp_sending_error},
    {EVENT_TYPE_ERROR,   SENSOR_ADE9000,  SERVICE_RAW, &st_trying_to_read_with_no_prot},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_could_not_connect_tcp},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_could_not_reconnect_tcp},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW, &st_incorrect_ip}
};

static event_info_t st_slow_event_info[ST_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_unknown_protcol},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_unknown_format},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_udp_not_send},
    {EVENT_TYPE_ERROR,   SENSOR_ADE9000,  SERVICE_REGISTERS, &st_udp_sending_error},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_tcp_not_send},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_tcp_sending_error},
    {EVENT_TYPE_ERROR,   SENSOR_ADE9000,  SERVICE_REGISTERS, &st_trying_to_read_with_no_prot},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_could_not_connect_tcp},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_could_not_reconnect_tcp},
    {EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &st_incorrect_ip}
};

/*************************************************************************/
/*							TIME_SYNC EVENTS       					     */
/*************************************************************************/

#include "time_sync.h"

static error_code_t ts_time_sync_timeout =
{CODE_TIME_NOT_SYNCRHONIZED, 	"Time synchronized timeout. System time not updated."};

static error_code_t ts_time_sync =
{CODE_TIME_SYNCRHONIZED, 	     "Time synchronized with server."};

static error_code_t ts_sync_type_unknown =
{CODE_UNKNOWN_SYNC_TYPE, 	     "Time synchronization type is unknown, no synchronization done."};

static event_info_t ts_event_info[TS_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &ts_time_sync_timeout},
	{EVENT_TYPE_INFO,    SENSOR_MAXVALUE, SERVICE_MAXVALUE, &ts_time_sync},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &ts_sync_type_unknown}
};

/*************************************************************************/
/*							MANUFACTURING EVENTS       					     */
/*************************************************************************/

#include "manufacturing.h"

static error_code_t manu_id_num_incorrect =
{CODE_INCORRECT_MANU_ID, 		"Incorrect ID number format, it should be X-XXXXXX."};

static error_code_t manu_mac_num_incorrect =
{CODE_INCORRECT_MANU_MAC, 		"Incorrect MAC number format, it should be XX:XX:XX:XX:XX:XX."};

static error_code_t manu_model_num_incorrect =
{CODE_INCORRECT_MANU_MODEL, 	"Incorrect sensor model read in memory."};

static error_code_t manu_model_num_empty =
{CODE_MANU_MODEL_EMPTY, 		"Sensor model not read in memory manufacturing information."};

static error_code_t manu_hw_v_board_empty =
{CODE_MANU_BOARD_EMPTY, 		"Sensor hw board not read in memory manufacturing information."};

static error_code_t manu_hw_v_serial_empty =
{CODE_MANU_SERIAL_EMPTY, 		"Sensor hw serial number not read in memory manufacturing information."};

static error_code_t manu_hw_v_manufacturer_empty =
{CODE_MANU_MANUFACTURER_EMPTY, 	"Sensor hw manufacturer not read in memory manufacturing information."};

static error_code_t manu_hw_v_version_empty =
{CODE_MANU_VERSION_EMPTY, 		"Sensor hw version not read in memory manufacturing information."};

static event_info_t manu_event_info[MANU_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_id_num_incorrect},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_mac_num_incorrect},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_model_num_incorrect},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_model_num_empty},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_hw_v_board_empty},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_hw_v_serial_empty},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_hw_v_manufacturer_empty},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE, &manu_hw_v_version_empty}
};

/***************************************************************************
 * System handler error codes
****************************************************************************/

#include "system_handler.h"

static error_code_t sh_watchdog_reset =
{CODE_WATCHDOG_RESET,					"Watchdog reset detected on system startup."};

static error_code_t sh_power_up_reset =
{CODE_POWERUP_RESET,					"Power on reset detected on system startup."};

static error_code_t sh_security_reset =
{CODE_SECURITY_RESET,					"Security reset detected on system startup."};

static error_code_t sh_sw_reset =
{CODE_SW_RESET,							"Software reset detected on system startup."};

static error_code_t sh_jtag_hw_reset =
{CODE_JTAG_HW_RESET,					"JTAG hardware reset detected on system startup."};

static error_code_t sh_jtag_sw_reset =
{CODE_JTAG_SW_RESET,					"JTAG software reset detected on system startup."};

static error_code_t sh_temp_sense_reset =
{CODE_TEMP_SENSE_RESET,					"Temperature sensor reset detected on system startup."};

static error_code_t sh_fw_disconnection_interrupt =
{CODE_FW_UPDATE_DISCONNECTION,			"FW update interrupted due to a disconnection. Try again."};

static error_code_t sh_fw_update_erased =
{CODE_FW_UPDATE_ERASED,					"FW update interrupted, new fw was not compatible or corrupted. Memory erased, try again."};

static error_code_t sh_unknown_fast_comm_format =
{CODE_UNKNOWN_COMM_FORMAT,				"Introduced fast vars communication data format is unknown or not compatible."};

static error_code_t sh_unknown_slow_comm_format =
{CODE_UNKNOWN_COMM_FORMAT,				"Introduced slow vars communication data format is unknown or not compatible."};

static error_code_t sh_unknown_fast_comm_protocol=
{CODE_UNKNOWN_COMM_FORMAT,				"Introduced communication protocol is unknown or not compatible."};

static error_code_t sh_unknown_slow_comm_protocol =
{CODE_UNKNOWN_COMM_PROTOCOL,			"Introduced slow vars communication protocol is unknown or not compatible."};

static error_code_t sh_time_sync_process_error =
{CODE_TIME_SYNC_PROCESS_ERROR,			"Time sync process never finished, make sure that the module in charge is well configured."};

static error_code_t sh_flase_erase_timeout_error =
{CODE_FLASH_ERASE_TIMEOUT_ERROR,		"To much time erasing new FW area."};

static error_code_t sh_ade9000_register_not_found =
{CODE_INCORRECT_TC_ERROR, 				"One of the received registers is not configurable in this service."};

static error_code_t sh_hardfault_generic_reset =
{CODE_HARDFAULT_GENERIC_RESET, 			"Hardfault generic error reset detected."};

static error_code_t sh_hardfault_nmi_reset =
{CODE_HARDFAULT_NMI_RESET, 				"Non-Maskable Interrupt Hardfault error reset detected."};

static error_code_t sh_hardfault_mem_man_reset =
{CODE_HARDFAULT_MEM_MAN_RESET, 			"Memory Management Hardfault error reset detected."};

static error_code_t sh_hardfault_bus_reset =
{CODE_HARDFAULT_BUS_RESET, 				"Bus Hardfault error reset detected."};

static error_code_t sh_hardfault_usage_reset =
{CODE_HARDFAULT_USAGE_RESET, 			"Usage Hardfault error reset detected."};

static error_code_t sh_hardfault_svc_reset =
{CODE_HARDFAULT_SVC_RESET, 				"Supervisor Call (SVC) Hardfault error reset detected."};

static error_code_t sh_hardfault_debugmon_reset =
{CODE_HARDFAULT_DEBUGMON_RESET, 		"Debug monitoring Hardfault error reset detected."};

static error_code_t sh_hardfault_pendsv_reset =
{CODE_HARDFAULT_PENDSV_RESET, 			"Pending service Hardfault error reset detected."};

static error_code_t sh_hardfault_systick_reset =
{CODE_HARDFAULT_SYSTICK_RESET, 			"System tick Hardfault error reset detected."};

static error_code_t sh_hardfault_inthand_reset =
{CODE_HARDFAULT_INTHAND_RESET, 			"Interruption handler Hardfault error reset detected."};

static error_code_t sh_hardfault_stackof_reset =
{CODE_HARDFAULT_STACKOF_RESET, 			"Stack overflow Hardfault error reset detected."};

static error_code_t sh_hardfault_malloc_reset =
{CODE_HARDFAULT_MALLOC_RESET, 			"Malloc Hardfault error reset detected."};

static error_code_t sh_hardfault_assert_reset =
{CODE_HARDFAULT_ASSERT_RESET, 			"Assert Hardfault error reset detected."};

static error_code_t sh_hardfault_thread_create_reset =
{CODE_HARDFAULT_THREAD_CREATE_RESET,	"Thread creation Hardfault error reset detected."};

static error_code_t sh_hardfault_thread_init_reset =
{CODE_HARDFAULT_THREAD_INIT_RESET,		"Thread initialization Hardfault error reset detected."};

static error_code_t sh_hardfault_thread_not_init_reset =
{CODE_HARDFAULT_THREAD_NOT_INIT_RESET,	"Thread not initialized Hardfault error reset detected."};

static error_code_t sh_hardfault_wdog_init_reset =
{CODE_HARDFAULT_WDOG_INIT_RESET,		"Watchdog initialization Hardfault error reset detected."};

static error_code_t sh_hardfault_wdog_not_init_reset =
{CODE_HARDFAULT_WDOG_NOT_INIT_RESET,	"Watchdog not initialized Hardfault error reset detected."};

static error_code_t sh_hardfault_manu_not_init_reset =
{CODE_HARDFAULT_MANU_NOT_INIT_RESET,	"Manufacturing module not initialized Hardfault error reset detected."};

static error_code_t sh_hardfault_debug_not_init_reset =
{CODE_HARDFAULT_DEBUG_NOT_INIT_RESET,	"Debug shell not initialized Hardfault error reset detected."};

static error_code_t sh_hardfault_switch_case_reset =
{CODE_HARDFAULT_SWITCH_CASE_RESET,		"Unknown switch case Hardfault error reset detected."};

static error_code_t sh_hardfault_queue_create_reset =
{CODE_HARDFAULT_QUEUE_CREATE_RESET,		"Queue creation Hardfault error reset detected."};

static error_code_t sh_hardfault_event_create_reset =
{CODE_HARDFAULT_EVENT_CREATE_RESET,		"Event creation Hardfault error reset detected."};

static error_code_t sh_hardfault_unknown_event_reset =
{CODE_HARDFAULT_UNKNOWN_EVENT_RESET,	"Unknown event Hardfault error reset detected."};

static error_code_t sh_hardfault_lazy_stacking_reset =
{CODE_HARDFAULT_LAZY_STACKING_RESET,	"Lazy stacking of the MPU Hardfault error reset detected."};

static error_code_t sh_hardfault_heap_push_reset =
{CODE_HARDFAULT_HEAP_PUSH_RESET,		"Heap push Hardfault error reset detected."};

static error_code_t sh_hardfault_heap_unstack_reset =
{CODE_HARDFAULT_HEAP_UNSTACK_RESET,		"Heap unstack Hardfault error reset detected."};

static error_code_t sh_hardfault_data_access_reset =
{CODE_HARDFAULT_DATA_ACCESS_RESET,		"Data access violation Hardfault error reset detected."};

static error_code_t sh_hardfault_inst_access_reset =
{CODE_HARDFAULT_INST_ACCESS_RESET,		"Instruction access violation Hardfault error reset detected."};

static error_code_t sh_hardfault_bus_lazy_stack_reset =
{CODE_HARDFAULT_BUS_LAZY_STACK_RESET,	"Bus lazy stacking Hardfault error reset detected."};

static error_code_t sh_hardfault_bus_heap_stack_reset =
{CODE_HARDFAULT_BUS_HEAP_STACK_RESET,	"Bus heap stacking Hardfault error reset detected."};

static error_code_t sh_hardfault_bus_heap_unstack_reset =
{CODE_HARDFAULT_BUS_HEAP_UNSTACK_RESET,	"Bus heap unstacking Hardfault error reset detected."};

static error_code_t sh_hardfault_imp_data_bus_reset =
{CODE_HARDFAULT_IMP_DATA_BUS_RESET,		"Imprecise data bus Hardfault error reset detected."};

static error_code_t sh_hardfault_pre_data_bus_reset =
{CODE_HARDFAULT_PRE_DATA_BUS_RESET,		"Precise data bus Hardfault error reset detected."};

static error_code_t sh_hardfault_inst_bus_reset =
{CODE_HARDFAULT_INST_BUS_RESET,			"Instruction bus Hardfault error reset detected."};

static error_code_t sh_hardfault_divided_zero_reset =
{CODE_HARDFAULT_DIVIDED_ZERO_RESET,		"Divided by zero usage Hardfault error reset detected."};

static error_code_t sh_hardfault_unaligned_access_reset =
{CODE_HARDFAULT_UNALIGNED_RESET,		"Unaligned access usage Hardfault error reset detected."};

static error_code_t sh_hardfault_no_coproc_reset =
{CODE_HARDFAULT_NO_COPROC_RESET,		"No supported coprocessor instruction usage Hardfault error reset detected."};

static error_code_t sh_hardfault_invalid_pc_reset =
{CODE_HARDFAULT_INVALID_PC_RESET,		"Invalid PC load usage Hardfault error reset detected."};

static error_code_t sh_hardfault_invalid_state_reset =
{CODE_HARDFAULT_INVALID_STATE_RESET,	"Invalid state usage Hardfault error reset detected."};

static error_code_t sh_hardfault_undef_inst_reset =
{CODE_HARDFAULT_UNDEF_INST_RESET,		"Undefined instruction usage Hardfault error reset detected."};

static error_code_t sh_registers_high_read_time =
{CODE_REGISTERS_HIGH_READ_TIME,			"With the selected period and variable amount, the estimated time needed "
										"to read the registers exceeds the period time. There might be data looses."};

static error_code_t sh_registers_raw_high_read_time =
{CODE_REGISTERS_RAW_HIGH_READ_TIME,		"With the selected period and variable amount, if raw service is also active, the estimated time needed "
										"to read the registers exceeds the period time. There might be data looses."};

static event_info_t sh_event_info[SH_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_watchdog_reset},
	{EVENT_TYPE_INFO, 	 SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_power_up_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_security_reset},
	{EVENT_TYPE_INFO, 	 SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_sw_reset},
	{EVENT_TYPE_INFO, 	 SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_jtag_hw_reset},
	{EVENT_TYPE_INFO, 	 SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_jtag_sw_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_temp_sense_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_fw_disconnection_interrupt},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_fw_update_erased},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW,       &sh_unknown_fast_comm_format},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &sh_unknown_slow_comm_format},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_RAW,       &sh_unknown_fast_comm_protocol},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &sh_unknown_slow_comm_protocol},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE, SERVICE_REGISTERS, &sh_time_sync_process_error},
	{EVENT_TYPE_ERROR,   SENSOR_MAXVALUE, SERVICE_REGISTERS, &sh_flase_erase_timeout_error},
	{EVENT_TYPE_WARNING, SENSOR_ADE9000,  SERVICE_REGISTERS, &sh_ade9000_register_not_found},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_generic_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_nmi_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_mem_man_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_bus_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_usage_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_svc_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_debugmon_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_pendsv_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_systick_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_inthand_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_stackof_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_malloc_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_assert_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_thread_create_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_thread_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_thread_not_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_wdog_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_wdog_not_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_manu_not_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_debug_not_init_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_switch_case_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_queue_create_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_event_create_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_unknown_event_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_lazy_stacking_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_heap_push_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_heap_unstack_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_data_access_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_inst_access_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_bus_lazy_stack_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_bus_heap_stack_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_bus_heap_unstack_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_imp_data_bus_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_pre_data_bus_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_inst_bus_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_divided_zero_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_unaligned_access_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_no_coproc_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_invalid_pc_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_invalid_state_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_hardfault_undef_inst_reset},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_registers_high_read_time},
	{EVENT_TYPE_WARNING, SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &sh_registers_raw_high_read_time}
};

/*************************************************************************/
/*							MQTT PROTOCOL EVENTS       					 */
/*************************************************************************/

#include "mqtt_protocol.h"

static error_code_t mp_alert_json_obtaining_error =
{CODE_ALERT_JSON_OBTAINING_ERROR,		"Error getting the JSON to send an alert mqtt message."};

static error_code_t mp_ts_json_obtaining_error =
{CODE_TS_JSON_OBTAINING_ERROR,			"Error getting the JSON to send a timestamp mqtt message."};

static error_code_t mp_info_json_obtaining_error =
{CODE_INFO_JSON_OBTAINING_ERROR,		"Error getting the JSON to send an info message."};

static error_code_t mp_unknown_sensor_status =
{CODE_UNKNOWN_SENSOR_STATUS,			"Obtained sensor status is invalid."};

static error_code_t mp_status_json_obtaining_error =
{CODE_SENSOR_STATUS_JSON_ERROR,			"Error getting the JSON to send sensor status message."};

static error_code_t mp_unknown_service_status =
{CODE_UNKNOWN_SERVICE_STATUS,			"Obtained sensor service is invalid."};

static error_code_t mp_service_json_obtaining_error =
{CODE_SERVICE_STATUS_JSON_ERROR,		"Error getting the JSON to send service status message."};

static event_info_t mp_event_info[MP_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_ERROR, 		SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &mp_alert_json_obtaining_error},
	{EVENT_TYPE_ERROR, 		SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &mp_ts_json_obtaining_error},
	{EVENT_TYPE_ERROR, 		SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &mp_info_json_obtaining_error},
	{EVENT_TYPE_ERROR, 		SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &mp_unknown_sensor_status},
	{EVENT_TYPE_ERROR, 		SENSOR_MAXVALUE, SERVICE_MAXVALUE,  &mp_status_json_obtaining_error},
	{EVENT_TYPE_ERROR, 		SENSOR_ADE9000,  SERVICE_MAXVALUE,  &mp_unknown_service_status},
	{EVENT_TYPE_ERROR, 		SENSOR_ADE9000,  SERVICE_MAXVALUE,  &mp_service_json_obtaining_error}
};

/*************************************************************************/
/*							JSON PROTOCOL EVENTS       					 */
/*************************************************************************/

#include "json.h"

static error_code_t json_fast_service_no_vars_config =
{CODE_NO_SERVICE_VARS_CONFIG,			"No correct variable configuration received in raw service start command."};

static error_code_t json_fast_service_no_tx_config =
{CODE_NO_SERVICE_TX_CONFIG,				"No correct transmission configuration received in raw service start command."};

static error_code_t json_slow_service_no_vars_config =
{CODE_NO_SERVICE_VARS_CONFIG,			"No correct variable configuration received in registers service start command."};

static error_code_t json_slow_service_no_tx_config =
{CODE_NO_SERVICE_TX_CONFIG,				"No correct transmission configuration received in registers service start command."};

static event_info_t json_event_info[JSON_EVENTS_MAXVALUE] =
{
	{EVENT_TYPE_WARNING, 	SENSOR_ADE9000, SERVICE_RAW,  		&json_fast_service_no_vars_config},
	{EVENT_TYPE_WARNING, 	SENSOR_ADE9000, SERVICE_RAW,  		&json_fast_service_no_tx_config},
	{EVENT_TYPE_WARNING, 	SENSOR_ADE9000, SERVICE_REGISTERS,  &json_slow_service_no_vars_config},
	{EVENT_TYPE_WARNING, 	SENSOR_ADE9000, SERVICE_REGISTERS,  &json_slow_service_no_tx_config}
};


#endif /* EVENTS_H_ */
