/*
 * sensors_types.h
 *
 *  Created on: Dec 12, 2023
 *      Author: abolinaga
 */

#ifndef SENSORS_TYPES_H_
#define SENSORS_TYPES_H_

#include "pl_types.h"

#define ENERGY_VARS_DATA_SIZE (300)

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

typedef struct slow_vars_data
{
	uint64_t timestamp;
	uint32_t data[ENERGY_VARS_DATA_SIZE];
}slow_vars_data_t;

typedef enum ade9000_wfb_samples
{
    IA = 0,
    VA,
    IB,
    VB,
    IC,
    VC,
	PA,
	PB,
	PC,
	SA,
	SB,
	SC,
	QA,
	QB,
	QC,
	WFB_AMOUNT
}ade9000_wfb_samples_t;


typedef enum ade9000_vars
{
	/* RMS */
	ADE9000_IA_RMS,
	ADE9000_VA_RMS,
	ADE9000_IB_RMS,
	ADE9000_VB_RMS,
	ADE9000_IC_RMS,
	ADE9000_VC_RMS,
	/* PF */
	ADE9000_PF_A,
	ADE9000_PF_B,
	ADE9000_PF_C,
	/* THD */
	ADE9000_VTHD_A,
	ADE9000_ITHD_A,
	ADE9000_VTHD_B,
	ADE9000_ITHD_B,
	ADE9000_VTHD_C,
	ADE9000_ITHD_C,
	/* wattvar */
	ADE9000_WATTHR_A,
	ADE9000_WATTHR_B,
	ADE9000_WATTHR_C,
	ADE9000_VARHR_A,
	ADE9000_VARHR_B,
	ADE9000_VARHR_C,
	ADE9000_VAHR_A,
	ADE9000_VAHR_B,
	ADE9000_VAHR_C,
	ADE9000_AFWATTHR_A,
	ADE9000_AFWATTHR_B,
	ADE9000_AFWATTHR_C,
	ADE9000_FVARHR_A,
	ADE9000_FVARHR_B,
	ADE9000_FVARHR_C,
	ADE9000_AFVAHR_A,
	ADE9000_AFVAHR_B,
	ADE9000_AFVAHR_C,
	/*OI*/
	ADE9000_OI_A,
	ADE9000_OI_B,
	ADE9000_OI_C,
	/*No load*/
	ADE9000_CFVANL,
	ADE9000_CFVARNL,
	ADE9000_CFWATTNL,
	ADE9000_CVANL,
	ADE9000_CVARNL,
	ADE9000_CWATTNL,
	ADE9000_BFVANL,
	ADE9000_BFVARNL,
	ADE9000_BFWATTNL,
	ADE9000_BVANL,
	ADE9000_BVARNL,
	ADE9000_BWATTNL,
	ADE9000_AFVANL,
	ADE9000_AFVARNL,
	ADE9000_AFWATTNL,
	ADE9000_AVANL,
	ADE9000_AVARNL,
	ADE9000_AWATTNL,
	ADE9000_IPPHASEA,
	ADE9000_IPPHASEB,
	ADE9000_IPPHASEC,
	ADE9000_IPEAKVAL,
	ADE9000_VPPHASEA,
	ADE9000_VPPHASEB,
	ADE9000_VPPHASEC,
	ADE9000_VPEAKVAL,
	ADE9000_DIP_A,
	ADE9000_DIP_B,
	ADE9000_DIP_C,
	ADE9000_SWELL_A,
	ADE9000_SWELL_B,
	ADE9000_SWELL_C,
	ADE9000_OIPHASEA,
	ADE9000_OIPHASEB,
	ADE9000_OIPHASEC,
	ADE9000_ANGL_V_A_V_B,
	ADE9000_ANGL_V_B_V_C,
	ADE9000_ANGL_V_A_V_C,
	ADE9000_ANGL_I_A_I_B,
	ADE9000_ANGL_I_B_I_C,
	ADE9000_ANGL_I_A_I_C,
	ADE9000_A_PERIOD,
	ADE9000_B_PERIOD,
	ADE9000_C_PERIOD,
	ADE9000_ISUM_RMS,
	ADE9000_SEQERR,
	ADE9000_VARS_MAXVALUE
}ade9000_vars_t;

typedef enum ade9000_phases
{
	PHASE_A = 0,
	PHASE_B,
	PHASE_C,
	PHASE_MAXNUM
}ade9000_phases_t;

typedef enum ade9000_fundamental_freq
{
	FUNDAMENTAL_FREQ_50_HZ = 0,
	FUNDAMENTAL_FREQ_60_HZ,
	FUNDAMENTAL_FREQ_INVALID
}energy_fundamental_freq_t;

typedef enum ade9000_hw_config
{
	HW_CONF_UNKNOWN = 0,
	HW_CONF_4_WIRE_WYE_NEUTRAL,
	HW_CONF_4_WIRE_WYE_ISOLATED,
	HW_CONF_3_WIRE_DELTA_PHASEB,
	HW_CONF_3_WIRE_DELTA_ISOLATED_VB,
	HW_CONF_3_WIRE_DELTA_ISOLATED_VA_VB_VC,
	HW_CONF_4_WIRE_DELTA_NEUTRAL,
	HW_CONF_4_WIRE_WYE_NONBLONDEL_NEUTRAL,
	HW_CONF_4_WIRE_DELTA_NONBLONDEL_NEUTRAL,
	HW_CONF_3_WIRE_1PH_NEUTRAL,
	HW_CONF_3_WIRE_NETWORK_NEUTRAL,
	HW_CONF_MULTIPLE_1PH_NEUTRAL
}energy_hw_config_t;

typedef enum ade9000_adc_redirect
{
	IA_DIN,
	IB_DIN,
	IC_DIN,
	IN_DIN,
	VA_DIN,
	VB_DIN,
	VC_DIN,
	ADC_DIN_MAXVALUE
}ade9000_adc_redirect_t;

#define WAVEFORM_BUFF_SIZE     	0xC00

#define NUM_CHANNELS_PER_PHASE	2
#define NUM_CHANNELS			(PHASE_MAXNUM * NUM_CHANNELS_PER_PHASE)

#define WAFEFORM_DATA_SAMPLE_SIZE ((WAVEFORM_BUFF_SIZE / sizeof(float)) / NUM_CHANNELS)

typedef struct power
{
	uint32_t watthr_lo;
	uint32_t watthr_hi;
	uint32_t varhr_lo;
	uint32_t varhr_hi;
	uint32_t vahr_lo;
	uint32_t vahr_hi;
	uint32_t fwatthr_lo;
	uint32_t fwatthr_hi;
	uint32_t fvarhr_lo;
	uint32_t fvarhr_hi;
	uint32_t fvahr_lo;
	uint32_t fvahr_hi;
}power_t;

typedef struct rms_pf
{
	uint32_t irms;
	uint32_t vrms;
	uint32_t pf;
}rms_pf_t;

typedef struct others
{
	uint32_t ph_noload;
	uint32_t i_peak;
	uint32_t v_peak;
	uint32_t oi_status;
	uint32_t oi_a;
	uint32_t oi_b;
	uint32_t oi_c;
}others_t;

typedef enum register_var_type
{
	VAR_NONE = 0,
    INTENSITY,
    VOLTAGE,
    POWER,
    WATVAR,
    THD,
    OI,
	IPEAK,
	VPEAK,
	DIP_SWELL,
	ANGLE,
	ISUM,
	SEQERR,
	PERIOD
}register_var_type_t;

typedef struct resgister_value
{
	register_var_type_t type;
	uint32_t			num;
	uint32_t 			value;
}resgister_value_t;

typedef struct waveform_data_samples
{
    float wvf_var_1[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_2[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_3[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_4[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_5[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_6[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_7[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_8[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_9[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_10[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_11[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_12[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_13[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_14[WAFEFORM_DATA_SAMPLE_SIZE];
    float wvf_var_15[WAFEFORM_DATA_SAMPLE_SIZE];
}waveform_data_samples_t;

typedef struct waveform_data
{
	uint64_t		  			initial_timestamp;
	uint64_t		  			final_timestamp;
	waveform_data_samples_t  	waveform_samples;
}waveform_data_t;

typedef enum vars_to_calc
{
	CALC_NONE        = 0x00,
	CALC_I           = 0x01,
	CALC_V           = 0x02,
	CALC_V_I         = 0x03,
	CALC_S           = 0x04,
	CALC_I_S         = 0x05,
	CALC_V_S         = 0x06,
	CALC_I_V_S       = 0x07,
	CALC_P           = 0x08,
	CALC_I_P         = 0x09,
	CALC_V_P         = 0x0A,
	CALC_I_V_P       = 0x0B,
	CALC_I_S_P       = 0x0D,
	CALC_S_P         = 0x0C,
	CALC_V_S_P       = 0x0E,
	CALC_I_V_S_P     = 0x0F,
	CALC_Q           = 0x10,
	CALC_I_Q         = 0x11,
	CALC_V_Q         = 0x12,
	CALC_I_V_Q       = 0x13,
	CALC_S_Q         = 0x14,
	CALC_I_S_Q       = 0x15,
	CALC_V_S_Q       = 0x16,
	CALC_I_V_S_Q     = 0x17,
	CALC_P_Q         = 0x18,
	CALC_I_P_Q       = 0x19,
	CALC_V_P_Q       = 0x1A,
	CALC_I_V_P_Q     = 0x1B,
	CALC_S_P_Q       = 0x1C,
	CALC_I_S_P_Q     = 0x1D,
	CALC_V_S_P_Q     = 0x1E,
	CALC_I_V_S_P_Q   = 0x1F
}vars_to_calc_t;

typedef enum pwr_p_q_calc
{
	CALC_PWR_NONE,
	CALC_PWR_P,
	CALC_PWR_Q,
	CALC_PWR_BOTH
}pwr_p_q_calc_t;


typedef void (*calc_cb_t)(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);

typedef struct fast_vars_config
{
	uint32_t		variable_count;
	vars_to_calc_t	vars_to_calc[PHASE_MAXNUM];
	bool			calc_p_q;
	pwr_p_q_calc_t	pwr_p_q_calc[PHASE_MAXNUM];
	calc_cb_t		var_calculation_cb[PHASE_MAXNUM];
}fast_vars_config_t;

typedef struct energy_sensor_init_conf
{
	uint8_t 					spi_conf_index;
	uint8_t 					enet_event_irq0_conf_index;
	uint8_t 					ade9000_reset_gpio_index;
	waveform_data_samples_t*	wf_pointers;
#ifdef DEBUG_PIN
	uint8_t						debug_gpio_index;
#endif
	void* 						ade9000_events_info_array;
}energy_sensor_init_conf_t;

typedef struct reg_address_t
{
    uint16_t	address;
	bool		reg_already_read;
	uint32_t	value;
}reg_address_t;

typedef struct reg_info
{
	uint8_t			initial_bit;
	uint8_t			final_bit;
	reg_address_t*	reg_address;
}reg_info_t;

typedef struct resgister_var_entry
{
    register_var_type_t type;
	uint32_t			reg_num;
	reg_info_t			reg_info[2];
}register_var_entry_t;

typedef struct ade9000_register_info
{
	char*					var_name;
	register_var_entry_t	register_var_entry;
}ade9000_register_info_t;

typedef struct _vars_config
{
	uint32_t				variable_count;
	register_var_entry_t 	registers[128];
	float					vars_read_period;
	uint32_t				data_lost_time;
	uint32_t				vars_read_time;
	uint32_t				fast_vars_time;
}slow_vars_config_t;

typedef union energy_data_types
{
	slow_vars_data_t 		slow_vars_data;
	waveform_data_t			waveform_data;
}energy_data_types_t;


#endif /* SENSORS_TYPES_H_ */
