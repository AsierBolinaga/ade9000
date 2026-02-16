/*
 * protocol_types.h
 *
 *  Created on: Nov 15, 2024
 *      Author: abolinaga
 */

#ifndef PROTOCOL_TYPES_H_
#define PROTOCOL_TYPES_H_

#include "absl_types.h"

#define ALERT_LEVEL_WARNING			"warning"
#define ALERT_LEVEL_INFO			"info"

#define HW_CONF_4WIRE_WYE_NEUTRAL_STRING					"4-Wire Wye Neutral"
#define HW_CONF_4WIRE_WYE_ISOLATED_STRING					"4-Wire Wye Isolated"
#define HW_CONF_3WIRE_DELTA_PHASEB_STRING					"3-Wire Delta Phase B"
#define HW_CONF_3WIRE_DELTA_ISOLATED_VB_STRING				"3-Wire Delta Isolated vb"
#define HW_CONF_3WIRE_DELTA_ISOLATED_VA_VB_VC_STRING		"3-Wire Delta Isolated va vb vc"
#define HW_CONF_4WIRE_DELTA_NEUTRAL_STRING					"4-Wire Delta Neutral"
#define HW_CONF_4WIRE_WYE_NOBLONDEL_NEUTRAL_STRING			"4-Wire Wye, Non-Blondel Compliant Neutral"
#define HW_CONF_4WIRE_DELTA_NOBLONDEL_NEUTRAL_STRING		"4-Wire Delta, Non-Blondel Compliant Neutral"
#define HW_CONF_3WIRE_1PH_NEUTRAL_STRING					"3-Wire 1PH Neutral"
#define HW_CONF_3WIRE_NETWORK_NEUTRAL_STRING				"3-Wire Network Neutral"
#define HW_CONF_MULTIPLE_1PH_NEUTRAL_STRING					"Multiple 1PH Circuits Neutral"

#define SYNC_NOW_STRING			"now"
#define SYNC_POLL_STRING		"poll"

#define TX_FORMAT_BIN_STRING	"binary"
#define TX_FORMAT_JSON_STRING	"json"

#define TX_PROTOCOL_UDP_STRING	"udp"
#define TX_PROTOCOL_TCP_STRING	"tcp"

#define	ID_NUM_SIZE			32
#define MAC_ADDRESS_SIZE	6
#define MODEL_SIZE			16
#define BOARD_AMOUNT 		2

#define BOARD_SIZE			16
#define SERIAL_NUM_SIZE		30
#define MANUFACTURER_SIZE	16
#define HW_VERSION_SIZE		17


#define SLOW_VAR_AIRMS_NAME 	 	"AIRMS"
#define SLOW_VAR_AVRMS_NAME 	 	"AVRMS"
#define SLOW_VAR_BIRMS_NAME 	 	"BIRMS"
#define SLOW_VAR_BVRMS_NAME 	 	"BVRMS"
#define SLOW_VAR_CIRMS_NAME 	 	"CIRMS"
#define SLOW_VAR_CVRMS_NAME 	 	"CVRMS"
#define SLOW_VAR_APF_NAME 	 		"APF"
#define SLOW_VAR_BPF_NAME 	 		"BPF"
#define SLOW_VAR_CPF_NAME 	 		"CPF"
#define SLOW_VAR_AVTHD_NAME 	 	"AVTHD"
#define SLOW_VAR_AITHD_NAME 	 	"AITHD"
#define SLOW_VAR_BVTHD_NAME 	 	"BVTHD"
#define SLOW_VAR_BITHD_NAME 	 	"BITHD"
#define SLOW_VAR_CVTHD_NAME 	 	"CVTHD"
#define SLOW_VAR_CITHD_NAME 	 	"CITHD"
#define SLOW_VAR_AWATTHR_NAME 	 	"AWATTHR"
#define SLOW_VAR_BWATTHR_NAME 	 	"BWATTHR"
#define SLOW_VAR_CWATTHR_NAME 	 	"CWATTHR"
#define SLOW_VAR_AVARHR_NAME 	 	"AVARHR"
#define SLOW_VAR_BVARHR_NAME 	 	"BVARHR"
#define SLOW_VAR_CVARHR_NAME 	 	"CVARHR"
#define SLOW_VAR_AVAHR_NAME 	 	"AVAHR"
#define SLOW_VAR_BVAHR_NAME 	 	"BVAHR"
#define SLOW_VAR_CVAHR_NAME 	 	"CVAHR"
#define SLOW_VAR_AFWATTHR_NAME 	 	"AFWATTHR"
#define SLOW_VAR_BFWATTHR_NAME 	 	"BFWATTHR"
#define SLOW_VAR_CFWATTHR_NAME 	 	"CFWATTHR"
#define SLOW_VAR_AFVARHR_NAME 	 	"AFVARHR"
#define SLOW_VAR_BFVARHR_NAME 	 	"BFVARHR"
#define SLOW_VAR_CFVARHR_NAME 	 	"CFVARHR"
#define SLOW_VAR_AFVAHR_NAME 	 	"AFVAHR"
#define SLOW_VAR_BFVAHR_NAME 	 	"BFVAHR"
#define SLOW_VAR_CFVAHR_NAME 	 	"CFVAHR"
#define SLOW_VAR_OIA_NAME 	 		"OIA"
#define SLOW_VAR_OIB_NAME 	 		"OIB"
#define SLOW_VAR_OIC_NAME 	 		"OIC"
#define SLOW_VAR_CFVANL_NAME 	 	"CFVANL"
#define SLOW_VAR_CFVARNL_NAME 	 	"CFVARNL"
#define SLOW_VAR_CFWATTNL_NAME 	 	"CFWATTNL"
#define SLOW_VAR_CVANL_NAME 	 	"CVANL"
#define SLOW_VAR_CVARNL_NAME 	 	"CVARNL"
#define SLOW_VAR_CWATTNL_NAME 	 	"CWATTNL"
#define SLOW_VAR_BFVANL_NAME 	 	"BFVANL"
#define SLOW_VAR_BFVARNL_NAME 	 	"BFVARNL"
#define SLOW_VAR_BFWATTNL_NAME 	 	"BFWATTNL"
#define SLOW_VAR_BVANL_NAME 	 	"BVANL"
#define SLOW_VAR_BVARNL_NAME 	 	"BVARNL"
#define SLOW_VAR_BWATTNL_NAME 	 	"BWATTNL"
#define SLOW_VAR_AFVANL_NAME 	 	"AFVANL"
#define SLOW_VAR_AFVARNL_NAME 	 	"AFVARNL"
#define SLOW_VAR_AFWATTNL_NAME 	 	"AFWATTNL"
#define SLOW_VAR_AVANL_NAME 	 	"AVANL"
#define SLOW_VAR_AVARNL_NAME 	 	"AVARNL"
#define SLOW_VAR_AWATTNL_NAME 	 	"AWATTNL"
#define SLOW_VAR_IPPHASEA_NAME 	 	"IPPHASEA"
#define SLOW_VAR_IPPHASEB_NAME 	 	"IPPHASEB"
#define SLOW_VAR_IPPHASEC_NAME 	 	"IPPHASEC"
#define SLOW_VAR_IPEAK_NAME 	 	"IPEAK"
#define SLOW_VAR_VPPHASEA_NAME 	 	"VPPHASEA"
#define SLOW_VAR_VPPHASEB_NAME 	 	"VPPHASEB"
#define SLOW_VAR_VPPHASEC_NAME 	 	"VPPHASEC"
#define SLOW_VAR_VPEAK_NAME 	 	"VPEAK"
#define SLOW_VAR_DIPA_NAME 	 		"DIPA"
#define SLOW_VAR_DIPB_NAME 	 		"DIPB"
#define SLOW_VAR_DIPC_NAME 	 		"DIPC"
#define SLOW_VAR_SWELLA_NAME 	 	"SWELLA"
#define SLOW_VAR_SWELLB_NAME 	 	"SWELLB"
#define SLOW_VAR_SWELLC_NAME 	 	"SWELLC"
#define SLOW_VAR_OIPHASEA_NAME 	 	"OIPHASEA"
#define SLOW_VAR_OIPHASEB_NAME 	 	"OIPHASEB"
#define SLOW_VAR_OIPHASEC_NAME 	 	"OIPHASEC"
#define SLOW_VAR_ANGL_VA_VB_NAME 	"ANGL_VA_VB"
#define SLOW_VAR_ANGL_VB_VC_NAME 	"ANGL_VB_VC"
#define SLOW_VAR_ANGL_VA_VC_NAME 	"ANGL_VA_VC"
#define SLOW_VAR_ANGL_IA_IB_NAME 	"ANGL_IA_IB"
#define SLOW_VAR_ANGL_IB_IC_NAME 	"ANGL_IB_IC"
#define SLOW_VAR_ANGL_IA_IC_NAME 	"ANGL_IA_IC"
#define SLOW_VAR_APERIOD_NAME 	 	"APERIOD"
#define SLOW_VAR_BPERIOD_NAME 	 	"BPERIOD"
#define SLOW_VAR_CPERIOD_NAME 	 	"CPERIOD"
#define SLOW_VAR_ISUMRMS_NAME 	 	"ISUMRMS"
#define SLOW_VAR_PHASE_SEQERR 	 	"PHASE_SEQERR"

#define WFB_VAR_IA_STRING 	"IA[A]"
#define WFB_VAR_VA_STRING 	"VA[V]"
#define WFB_VAR_IB_STRING 	"IB[A]"
#define WFB_VAR_VB_STRING 	"VB[V]"
#define WFB_VAR_IC_STRING 	"IC[A]"
#define WFB_VAR_VC_STRING 	"VC[V]"
#define WFB_VAR_SA_STRING 	"SA[VA]"
#define WFB_VAR_SB_STRING 	"SB[VA]"
#define WFB_VAR_SC_STRING 	"SC[VA]"
#define WFB_VAR_PA_STRING 	"PA[W]"
#define WFB_VAR_PB_STRING 	"PB[W]"
#define WFB_VAR_PC_STRING 	"PC[W]"
#define WFB_VAR_QA_STRING 	"QA[VAR]"
#define WFB_VAR_QB_STRING 	"QB[VAR]"
#define WFB_VAR_QC_STRING 	"QC[VAR]"

typedef struct hw_version
{
	char	board[BOARD_SIZE];
	char 	serial[SERIAL_NUM_SIZE];
	char	manufacturer[MANUFACTURER_SIZE];
	char 	hw_version[HW_VERSION_SIZE];
}hw_version_t;

typedef struct manufacturing
{
	char 			id_mumber[ID_NUM_SIZE];
	uint8_t			mac_address[MAC_ADDRESS_SIZE];
	char			model[MODEL_SIZE];
	hw_version_t 	hw_version[BOARD_AMOUNT];
}manufacturing_t;

typedef enum protocol_rv
{
	PROTOCOL_INVALID_MESSAGE = 0,
	PROTOCOL_INVALID_CONFIG_MESSAGE,
	PROTOCOL_CONFIG_MSG_RECIEVED,
	PROTOCOL_RESET_MSG_RECIEVED,
	PROTOCOL_SENSOR_RESET_MSG_RECIEVED,
	PROTOCOL_REBOOT_MSG_RECIEVED,
	PROTOCOL_INVALID_START_MESSAGE,
	PROTOCOL_START_CMD_RECIEVED,
	PROTOCOL_STOP_CMD_RECIEVED,
	PROTOCOL_ALERT_MSG_RECIEVED,
	PROTOCOL_TIMEOUT,
	PROTOCOL_INVALID_SYNC_MESSAGE,
	PROTOCOL_TIME_SYNC_MSG_RECIEVED,
	PROTOCOL_GET_TIMESTAMP_MSG_RECIEVED,
	PROTOCOL_GET_INFO_MSG_RECIEVED,
	PROTOCOL_INVALID_MANUFACTUR_MESSAGE,
	PROTOCOL_MANUFACTUR_MSG_RECIEVED,
	PROTOCOL_FACTORY_CONFIG_MSG_RECIEVED,
	PROTOCOL_UPDATE_MSG_RECIEVED,
	PROTOCOL_DISCONNECTED,
	PROTOCOL_NO_CONNECTED,
#ifdef FABRICATION_TEST
	PROTOCOL_ID_MSG_RECIEVED,
	PROTOCOL_IRQ0_MSG_RECIEVED,
	PROTOCOL_IRQ1_MSG_RECIEVED,
	PROTOCOL_RST_MSG_RECIEVED,
	PROTOCOL_UDP_START_MSG_RECIEVED,
	PROTOCOL_UDP_STOP_MSG_RECIEVED,
	PROTOCOL_TCP_START_MSG_RECIEVED,
	PROTOCOL_TCP_STOP_MSG_RECIEVED,
#endif
	PROTOCOL_ERROR
}protocol_rv_t;

typedef enum state_type
{
	DEVICE_STATE = 0,
	SENSOR_STATE,
	SERVICE_STATE
}state_type_t;

typedef enum device_status
{
	DEVICE_STATUS_OFFLINE = 0,
	DEVICE_STATUS_ONLINE,
	DEVICE_STATUS_FAILURE
}device_status_t;

typedef enum sensor_status
{
	SENSOR_STATUS_IDLE = 0,
	SENSOR_STATUS_CONFIGURED,
	SENSOR_STATUS_FAILURE
}sensor_status_t;

typedef enum service_status
{
	SERVICE_STATUS_IDLE = 0,
	SERVICE_STATUS_RUNNING,
	SERVICE_STATUS_FAILURE
}service_status_t;

typedef enum domain
{
	DOMAIN_SYSTEM = 0,
	DOMAIN_SENSOR,
	DOMAIN_SERVICE
}domain_t;
typedef struct device_info_t
{
	device_status_t  status;
}device_info_t;

typedef struct sensor_info_t
{
	sensor_status_t  status;
	uint32_t		 sensor;
}sensor_info_t;

typedef struct service_info_t
{
	service_status_t  status;
	uint32_t		  sensor;
	uint32_t		  service;
}service_info_t;

typedef union state_info
{
	device_info_t 	device_info;
	sensor_info_t	sensor_info;
	service_info_t	service_info;
}state_info_t;

typedef struct error_code
{
	uint32_t 	error_code_num;
	char*		error_code_description;
}error_code_t;

typedef struct alert_data
{
	uint64_t		time;
	char			event_level[10];
	domain_t  		domain;
	char			source[30];
	error_code_t* 	error_code;
}alert_data_t;

typedef struct state_change_data
{
	state_type_t    state_type;
	state_info_t	state_info;
	error_code_t* 	error_codes;
	uint32_t		error_code_amount;
}state_change_data_t;

typedef struct device_status_information
{
	uint64_t 		timestamp;
	char* 			ip_address;
	float 			temperature;
	float 			cpu_load;
	float 			heap_usage;
	char* 			sync_interval;
	hw_version_t*	hw_version;
}device_status_information_t;


typedef enum adc_redirect
{
	IA_ADC_READ = 0,
	IB_ADC_READ,
	IC_ADC_READ,
	VA_ADC_READ,
	VB_ADC_READ,
	VC_ADC_READ,
	ADC_READ_NUM
}adc_redirect_t;

typedef enum adc_redirect_values
{
	IA_ADC_VALUE = 0,
	IB_ADC_VALUE = 1,
	IC_ADC_VALUE = 2,
	VA_ADC_VALUE = 4,
	VB_ADC_VALUE = 5,
	VC_ADC_VALUE = 6,
	ADC_DEFAULT_VALUE = 7
}adc_redirect_valuest;

typedef struct energy_sensor_received_config
{
	uint32_t 		tc_primary_side;
	uint32_t 		tc_secondary_side;
	uint32_t 		fundamental_frequency;
	bool 			high_pass_filter_disable;
	char			hw_config[20];
	uint32_t		OI_threshold;
	uint32_t		SWELL_threshold;
	uint32_t		DIP_threshold;
	bool			ia_invert;
	bool			ib_invert;
	bool			ic_invert;
	adc_redirect_t	adc_redirect[ADC_READ_NUM];
	int32_t			v_gain[3];
	int32_t			i_gain[3];
	int32_t	    	p_gain[3];
}energy_sensor_received_config_t;

typedef struct time_sync_config
{
	char		when[10];
	char		interval[10];
}time_sync_config_t;

typedef struct settings
{
	char	 ip[20];
	uint32_t port;
}settings_t;

typedef struct transmission_config
{
	char		format[10];
	char 		protocol[10];
	settings_t	settings;
}transmission_config_t;

typedef struct raw_config
{
	uint32_t	raw_vars_amount;
	char		raw_vars_list[20][8];
}raw_config_t;

typedef struct vars_config
{
	uint32_t	vars_amount;
	char		vars_list[90][15];
	float		period;
}vars_config_t;

typedef struct test_config
{
	uint32_t	data_size;
	uint32_t	period;
}test_config_t;

typedef bool (*service_config_cb_t)(char* _config_data_buff, void* _config_data);
typedef bool (*tx_config_cb_t)(char* _config_data_buff, transmission_config_t* _config_data);

typedef struct service_config
{
	char*						service_name;
	service_config_cb_t			service_config_cb;
	void*						service_config_data;
	tx_config_cb_t				service_tx_config_cb;
	transmission_config_t*		service_tx_config_data;
}service_config_t;

typedef bool (*sensor_config_cb_t)(char* _config_data_buff, void* _config_data);

typedef struct sensor_config
{
	char*				sensor_name;
	sensor_config_cb_t	sensor_config_cb;
	void*				sensor_config_data;
	uint8_t				service_amount;
	service_config_t**	sensor_services_config;
}sensor_config_t;

typedef struct protocol_config
{
	uint8_t 			mqtt_conf_index;
	uint8_t				sensor_amount;
	sensor_config_t*	sensors_config;
	char*				fw_version;
	char*				device_ID;
	char*				model;
	void* 				mqtt_events_array;
	void* 				json_events_array;
}protocol_config_t;


typedef union state_data_to_send
{
	alert_data_t 			alert_data;
	state_change_data_t		state_change_data;
}state_data_to_send_t;


#endif /* PROTOCOL_TYPES_H_ */
