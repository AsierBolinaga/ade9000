/*
 * board_types.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef INTERFACES_H_
#define INTERFACES_H_

#include "pl_types.h"

#include "system_handler.h"

#define ade9000_get_tc_primary_config						system_get_tc_primary_config
#define ade9000_get_tc_secondary_config						system_get_tc_secondary_config
#define ade9000_get_fund_freq_config						system_get_fund_freq_config
#define ade9000_get_hw_config								system_get_hw_config
#define ade9000_get_high_pass_filter_config					system_get_high_pass_filter_config
#define ade9000_get_oi_threshold_config						system_get_oi_threshold_config
#define ade9000_get_swell_threshold_config					system_get_swell_threshold_config
#define ade9000_get_dip_threshold_config					system_get_dip_threshold_config
#define ade9000_get_current_a_invert 						system_get_current_a_invert
#define ade9000_get_current_b_invert 						system_get_current_b_invert
#define ade9000_get_current_c_invert						system_get_current_c_invert
#define ade9000_get_adc_redirect							system_get_adc_redirect
#define ade9000_get_slow_vars_period						system_get_slow_vars_period

#define time_sync_get_sync_type								system_sync_get_sync_type
#define time_sync_get_sync_period							system_sync_get_sync_period

#include "ade9000.h"

#define	energy_init						                    ade9000_init
#define energy_startup 					                    ade9000_startup
#define energy_configure				                    ade9000_configure
#define energy_enable_data_obtaining_fast	                ade9000_continue_obtaining_fast
#define energy_enable_data_obtaining_slow	                ade9000_continue_obtaining_slow
#define energy_clear_waveform			                    ade9000_clear_waveform
#define energy_clear_slow_vars			                    ade9000_clear_slow_vars
#define energy_fill_energy_data			                    ade9000_fill_energy_data
#define energy_get_data_to_send			                    ade9000_get_data_to_send
#define energy_get_event_time			                    ade9000_get_event_time
#define energy_get_event_time_ns		                    ade9000_get_event_time_ns
#define energy_reboot					                    ade9000_reboot

#define slow_variables_read									ade9000_read_slow_variables

#define fast_variables_read									ade9000_read_waveform

#define json_get_register_info			                    ade9000_get_register_info

#include "mqtt_protocol.h"

#define cmd_protocol_init									mqtt_protocol_init
#define cmd_protocol_connect								mqtt_protocol_connect
#define cmd_protocol_reconnect 								mqtt_protocol_reconnect
#define cmd_protocol_send_device_status						mqtt_protocol_device_status_changed
#define cmd_protocol_send_status							mqtt_protocol_status_changed
#define cmd_protocol_send_discovery							mqtt_protocol_send_discovery
#define cmd_protocol_listen									mqtt_protocol_listen
#define cmd_protocol_messages_to_get						mqtt_protocol_messages_to_get
#define cmd_protocol_frame_type								mqtt_protocol_frame_type
#define cmd_protocol_disconnect								mqtt_protocol_disconnect
#define cmd_protocol_get_timestamp							mqtt_protocol_get_timestamp
#define cmd_protocol_get_service_config						mqtt_protocol_get_service_config
#define cmd_protocol_get_manufactur							mqtt_protocol_get_manufactur
#define cmd_protocol_reset									mqtt_protocol_reset
#define cmd_protocol_send_timestamp							mqtt_protocol_send_timestamp
#define cmd_protocol_send_info								mqtt_protocol_send_info
#define cmd_protocol_config_for_fw_update_file_reception	mqtt_protocol_config_for_fw_update_file_reception
#define cmd_protocol_config_for_normal_cmd_reception		mqtt_protocol_config_for_normal_cmd_reception
#define cmd_protocol_get_ip									mqtt_protocol_get_ip
#define cmd_protocol_get_sync_interval						mqtt_protocol_get_sync_interval
#define cmd_protocol_send_event								mqtt_protocol_signalize_event

#define fw_update_data_get									mqtt_protocol_get_frame

#include "event_handler.h"

#define energy_notify_system_event							event_handler_notify_system_event
#define ade9000_notify_system_event							event_handler_notify_system_event
#define vars_read_notify_system_event						event_handler_notify_system_event	
#define cmd_task_notify_system_event						event_handler_notify_system_event
#define fw_update_notify_system_event						event_handler_notify_system_event
#define sfw_notify_system_event								event_handler_notify_system_event
#define streaming_task_notify_system_event					event_handler_notify_system_event
#define time_sync_notify_system_event						event_handler_notify_system_event
#define manufacturing_notify_system_event					event_handler_notify_system_event
#define mqtt_notify_system_event							event_handler_notify_system_event
#define json_notify_system_event							event_handler_notify_system_event

#include "sfw_handling.h"

#define fw_update_check_image                               sfw_handling_check_image 
#define fw_update_get_fw_slot                               sfw_handling_get_fw_slot
#define fw_update_get_update_slot                           sfw_handling_get_update_slot
#define fw_update_validate_received_header                  sfw_handling_validate_received_header
#define fw_update_validate_written_image                    sfw_handling_validate_written_image
#include "manufacturing.h"

#define json_get_mac                               			manufacturing_get_mac
#define cmd_protocol_manufactur_write						manufacturing_check_and_write

#endif /* INTERFACES_H_ */
