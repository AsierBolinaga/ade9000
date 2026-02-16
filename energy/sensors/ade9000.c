/*
 * ade9000.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */
#include "ade9000.h"
#include "ade9000_regs.h"

#include "interfaces.h"

#include "absl_debug.h"
#include "absl_hw_config.h"

#include "absl_spi.h"
#include "absl_mutex.h"
#include "absl_timer.h"
#include "absl_enet_event.h"
#include "absl_macros.h"
#include "absl_gpio.h"
#include "absl_thread.h"
#include "absl_system.h"

#include "absl_types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SIZE_TEMP_TRIM      			4

#define DISABLE_REG_PROT_CMD    		0x4AD1
#define ENABLE_REG_PROT_CMD				0x3C64

#define ADE9000_SAMPLING_FREQUENCY 		8000
#define ADE9000_ACCUMULATION_TIME		3600

#define ENERGY_ACCUMULATION_TIME 		0.1	 		//segs
#define ENERGY_SAMPLES_PER_SECOND 		8000

#define ADE9000_REGISTER_SIZE	2
#define ADE9000_READ_REGISTER	0x8
#define ADE9000_WRITE_REGISTER	0x0

#define ADE9000_TEMP_START      0x0008
#define ADE9000_TEMP_ENABLE     0x0004

#define	SWRST				BIT(0)
#define RUN_DSP				0x1
#define STOP_DSP			0x0
#define GAIN_1				0x0
#define HPFDIS				BIT(3)
#define INTEN				BIT(5)
#define RMS_SRC_SEL			BIT(7)
#define EGY_TMR_MODE		BIT(1)
#define	EGY_LD_ACCUM		BIT(4)
#define	RD_RST_EN			BIT(5)
#define WF_CAP_EN			BIT(4)
#define WF_CAP_SEL			BIT(5)
#define EGY_PWR_EN			BIT(0)
#define DREADY				BIT(15)
#define PAGE_FULL			BIT(17)
#define PHASE_SEQERR		BIT(18)
#define PAGE_IRQ_EN(x)		(BIT(x))
#define WFB_LAST_PAGE		GENMASK(15,12)
#define CONT_FILL			(BIT(7) | BIT(6))
#define CONT_FILL_FIELD		GENMASK(7,6)
#define SELFREQ				BIT(8)
#define BURST_CHAN			GENMASK(3,0)
#define OC_EN				GENMASK(15,12)
#define PEAKSEL				GENMASK(4,2)
#define VCONSEL_111			GENMASK(6,4)
#define VCONSEL_001			BIT(4)
#define VCONSEL_010			BIT(5)
#define VCONSEL_011			BIT(4) | BIT(5)
#define VCONSEL_100			BIT(6)
#define VARACC  			GENMASK(3,2)
#define WATTACC 			GENMASK(1,0)
#define NOLOAD_TMR 			GENMASK(15,13)
#define ISUM_CFG			GENMASK(1,0)
#define DISRPLPF			BIT(13)
#define DISAPLPF			BIT(12)
#define VARDIS				BIT(0)

#define READ_CMD    		0x0004
#define WRITE_CMD			0x0000
#define CMD_ADDR_OFFSET     0x2
#define CMD_SIZE_BYTES		0x2
#define	CMD_BURST_1ST_HALF	0x800
#define	CMD_BURST_2ND_HALF	0xC00
#define CMS_BURST_UNKNOWN 	0x000
#define PAGE_HALF_IDX		7
#define PAGE_FULL_IDX		15
#define PAGE_SIZE_BYTES		(WAVEFORM_BUFF_SIZE +  ADE9000_REGISTER_SIZE)

#ifdef FLOAT_PWR_CALC
#define SAMPLE_BYTES	 	sizeof(float)
#else
#define SAMPLE_BYTES	 	sizeof(double)
#endif

/* Math stuff */
#define _2PI			6.283185

/*
 * ADE9000 defines. System Parameters and Conversion Constants
 * https://www.analog.com/media/en/technical-documentation/user-guides/ADE9000-UG-1098.pdf
 * Page 51, 52
 */

#define FULLSCALE_VI_RMS_CODES 		52702092
#define FULLSCALE_VI_CODES 			74532013
#define FULLSCALE_POWER_CODES 		20694066

#define I_NOM 						10
#define V_NOM 						220
#define VOLTAGE_ADC_FULL_SCALE 		1
#define VOLTAGE_ADC_FULL_SCALE_RMS 	0.707

#define POWER_VARS_STRUCT_STRIDE 		1
#define POWER_FACTOR_COEF 				0.000000007450581		/* pow(2, -27) */
#define THRESHOLDS_FACTOR				0.03125     			/* pow(2,-5) */
#define FACTOR_COEF 					32.000000000000000 		/* pow(2, 5) */
#define POWER_KW_IN_W_COEF		 		0.001        		 	/* Convert W in KW */
#define ENERGY_DIP_LVL_THRESHOLD		220
#define ENERGY_SWELL_LVL_THRESHOLD		240
#define ENERGY_OVERCURRENT_THRESHOLD	1

#define	ANGLE_50HZ					0.017578125
#define	ANGLE_60HZ					0.02109375

#define PERIOD_FACTOR			 	524288000					/* 8000 * pow(2, 16) */

#define CURRENT_INVERTION_GAIN		0xF0000000					/* xIGAIN=ROUND(Channel Gain−1)×2^27 -> channel gain -1 to invert the current */

/**
 * Resistor values from the schematics
 * R1:               sum of R56, R57, R58 (phase A)
 * R2:               R59 value
 * Burden Resistor:  sum of R16 and R17 (phase A)
 */
#define R1 				990000
#define R2 				1020
#define BURDEN_RESISTOR 0.066

#define SLEEP_MICRO_SECOND		1/1000

#define ADE9000_TIME_BETWEEN_IRQ_MS					16000
#define ADE9000_TIME_BETWEEN_SAMPLES				ADE9000_TIME_BETWEEN_IRQ_MS / WAFEFORM_DATA_SAMPLE_SIZE

#define SAMPLES_POWER_CALC		512
#define COMPLEX_SAMPLES			(SAMPLES_POWER_CALC / 2)
#define MIN_FREE_BUFF_INDEX		(SAMPLES_POWER_CALC - WAFEFORM_DATA_SAMPLE_SIZE)

#define SAMPLES_CYCLE_60HZ	 	400		// 60HZ one sample every 128us => 400 samples every cycle
#define SAMPLES_CYCLE_50HZ	 	160		// 50HZ one sample every 128us => 160 samples every cycle

#define M_PI 3.14159265358979323846


/*******************************************************************************
 * type definitions
 ******************************************************************************/

typedef enum ade9000_wvf_half
{
	WVF_FIRST_HALF = 0,
	WVF_SECOND_HALF
}ade9000_wvf_half_t;

static float c_tf;
static float v_tf;
static float c_adc;
static float v_adc;
static float i_fsp;
static float v_fsp;

static float i_factor;
static float v_factor;
static float p_factor;
static float e_factor;
static float angle_factor;

typedef struct energy_pwr_sample
{
#ifdef FLOAT_PWR_CALC
	float  __attribute__((aligned(16))) s_buf[SAMPLES_POWER_CALC];
	float  __attribute__((aligned(32))) rms_buf[SAMPLES_POWER_CALC];
	float  __attribute__((aligned(16))) rms_acc;
#else
	double  __attribute__((aligned(16))) s_buf[SAMPLES_POWER_CALC];
	double  __attribute__((aligned(32))) rms_buf[SAMPLES_POWER_CALC];
	double  __attribute__((aligned(16))) rms_acc;
#endif
}energy_pwr_sample_t;

typedef struct energy_data_ph
{
    float i_rms;
    float v_rms;
} energy_data_ph_t;


static ALLOC_DATA_SEC energy_pwr_sample_t 	pwr_calc[NUM_CHANNELS];
static ALLOC_DATA_SEC energy_data_ph_t 	    e_data[PHASE_MAXNUM];

#ifdef FLOAT_PWR_CALC
static  __attribute__((aligned(16))) float va_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) float ia_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) float vb_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) float ib_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) float vc_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) float ic_complex[SAMPLES_POWER_CALC];

static  __attribute__((aligned(16))) float aux_buff[SAMPLES_POWER_CALC];
#else
static  __attribute__((aligned(16))) double va_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) double ia_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) double vb_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) double ib_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) double vc_complex[SAMPLES_POWER_CALC];
static  __attribute__((aligned(16))) double ic_complex[SAMPLES_POWER_CALC];

static  __attribute__((aligned(16))) double aux_buff[SAMPLES_POWER_CALC];
#endif

static float hamming_window[SAMPLES_POWER_CALC];

static int32_t  samples_s_cycle;
static int32_t  samples_index;
static int32_t  empty_initial_positons;

#ifdef FLOAT_PWR_CALC
static arm_rfft_fast_instance_f32 	fftInstance;
#else
static arm_rfft_fast_instance_f64 	fftInstance;
#endif
static float						fft_coef;

static ALLOC_DATA_SEC uint8_t  			waveform_buff[WAVEFORM_BUFF_SIZE];
static ALLOC_DATA_SEC resgister_value_t	registers[ENERGY_VARS_DATA_SIZE];

static bool 		first_waveform_read;
static uint64_t* 	irq_timestamp;

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/

static bool ade9000_quickstart(void);
static bool ade9000_reconfig(void);
static bool ade9000_configuration(void);

static bool	ade9000_set_config0(bool _hpf_dis);
static bool	ade9000_set_xpgain(uint32_t _x_pgain);
static bool	ade9000_set_accmode(energy_fundamental_freq_t _fund_freq, energy_hw_config_t _hw_config);
static bool	ade9000_config_adc_redirect(uint32_t* _adc_redirect);
static bool	ade9000_set_ep_cfg(void);
static bool	ade9000_set_mask0(void);
static bool	ade9000_set_config3(void);
static bool	ade9000_set_thresholds(float _oc_threshold, float _dip_threshold, float _swell_threshold);
static bool	ade9000_check_current_invert(uint16_t _xGAIN, bool _invert_current);
static bool	ade9000_set_wfb_cfg(void);
static bool	ade9000_set_wfb_pg_irq(void);
static bool ade9000_enable_energy_acc(void);

static bool  ade9000_check_fund_freq(energy_fundamental_freq_t _fund_freq, uint32_t* _reg);
static bool  ade9000_check_hw_config(energy_hw_config_t _hw_config, uint32_t* _reg);
static bool  ade9000_system_parameters(void);
static void  ade9000_conversion_factors(void);
static void  ade9000_print_config(void);
static void  ade9000_calc_cycle_values(int32_t _samples_in_cycle);
static void  ade9000_coeficients(void);
static bool  ade900_change_reg_protection(bool _enable);
static bool  ade9000_read_register(uint16_t _register, uint8_t* _read_register_data);
static bool  ade9000_ts_read_register(uint16_t _register, uint8_t* _read_register_data, uint64_t* _timestamp);
static bool  ade9000_write_register(uint16_t _register, uint8_t* _write_register_data);
static bool  ade9000_check_written_register(uint16_t _register, uint32_t _written_value, uint32_t _read_value);
static bool  ade9000_read_burst(uint16_t _page, uint8_t* _buff);
static void  ade9000_data_rearrange(void *_data, unsigned int _bytes);
static float ade9000_apply_intensity(int32_t _data);
static float ade9000_apply_voltage(int32_t _data);
static float ade9000_apply_power(int32_t _data);
static float ade9000_calc_s_pwr(float _v_sample, float _i_sample, ade9000_wfb_samples_t _v_sample_type,
							   ade9000_wfb_samples_t _i_sample_type, energy_data_ph_t* _e_data_ph_idx);
static void ade9000_fill_samples_buffers(float _sample, energy_pwr_sample_t* _pwr_sample);
#ifdef FLOAT_PWR_CALC
static void ade9000_calc_fft(float* _samples, float* _complex_buff, uint32_t _first_part, uint32_t _second_part);
#else
static void ade9000_calc_fft(double* _samples, double* _complex_buff, uint32_t _first_part, uint32_t _second_part);
#endif
static void ade9000_calc_q_and_p_pwr(fast_vars_config_t* _vars_config);
static double ade9000_apply_watvar(uint32_t _high, uint32_t _low);
static float  ade9000_apply_oi(uint32_t _data);
static float ade9000_apply_ipeak(uint32_t _data);
static float ade9000_apply_vpeak(uint32_t _data);
static float  ade9000_apply_dip_swell(uint32_t _data);
static float  ade9000_apply_thd(int32_t _data);
static float  ade9000_apply_angle(int32_t _data);
static float  ade9000_apply_period(int32_t _data);

static void ade9000_set_slow_vars_period(void);

static void ade9000_get_data_to_send(void *_data, fast_vars_config_t* _config);
static void ade9000_fill_energy_data(slow_vars_data_t* _energy_data, resgister_value_t*	_registers, uint32_t _var_count);

static uint32_t ade9000_extract_value(uint32_t _reg_value, uint8_t _initial_bit, uint8_t _final_bit);



/*******************************************************************************
 * Variables
 ******************************************************************************/
static absl_spi_t					spi_ade9000;
static absl_spi_config_t*  		spi_ade9000_config;

static absl_enet_event_t 			ade9000_irq0;
static absl_enet_event_config_t* 	ade9000_irq0_config;

static absl_gpio_t 				ade9000_reset_gpio;
static absl_gpio_config_t* 		ade9000_reset_gpio_config;

static absl_mutex_t	spi_buff_mutex;
static uint8_t 		transmit_buffer[PAGE_SIZE_BYTES] = {0};
static uint8_t 		receive_buffer[PAGE_SIZE_BYTES]  = {0};

static bool			slow_vars_start_wait_time;
static absl_timer_t	absl_timer_slow_vars;

static bool			slow_vars_running;
static bool			slow_vars_first_read;
static uint64_t 	next_packet_wf_init_timestamp;
static uint64_t 	last_obtained_power_vars_timestamp;

static uint32_t  	waveform_buffer_status = 0;

static ade9000_wvf_half_t wvf_half_to_expect = WVF_FIRST_HALF;

static void* 		ade9000_event_info_array;

static absl_event_t* 	slow_vars_event_group;
static uint32_t 	read_slow_vars_event;

#ifdef DEBUG_PIN
static absl_gpio_t            debug_gpio;
static absl_gpio_config_t*    debug_gpio_config;
#endif

static float* wvf_i[PHASE_MAXNUM];
static float* wvf_v[PHASE_MAXNUM];
static float* wvf_s[PHASE_MAXNUM];
static float* wvf_p[PHASE_MAXNUM];
static float* wvf_q[PHASE_MAXNUM];

static float* wvf_position[WFB_AMOUNT];

static ade9000_wfb_samples_t i_phases[PHASE_MAXNUM] = {IA, IB, IC};
static ade9000_wfb_samples_t v_phases[PHASE_MAXNUM] = {VA, VB, VC};

#ifdef FLOAT_PWR_CALC
static float* v_samples[PHASE_MAXNUM] =
#else
static double* v_samples[PHASE_MAXNUM] =
#endif
{
		pwr_calc[VA].s_buf,
		pwr_calc[VB].s_buf,
		pwr_calc[VC].s_buf
};

#ifdef FLOAT_PWR_CALC
static float* i_samples[PHASE_MAXNUM] =
#else
static double* i_samples[PHASE_MAXNUM] =
#endif
{
		pwr_calc[IA].s_buf,
		pwr_calc[IB].s_buf,
		pwr_calc[IC].s_buf
};

#ifdef FLOAT_PWR_CALC
static float* v_complex[PHASE_MAXNUM] =
#else
static double* v_complex[PHASE_MAXNUM] =
#endif
{
	va_complex,
	vb_complex,
	vc_complex
};

#ifdef FLOAT_PWR_CALC
static float* i_complex[PHASE_MAXNUM] =
#else
static double* i_complex[PHASE_MAXNUM] =
#endif
{
	ia_complex,
	ib_complex,
	ic_complex
};


/*******************************************************************************
 * Code
 ******************************************************************************/
void ade9000_slow_vars_call(void* _arg)
{
	ABSL_UNUSED_ARG(_arg);

	if(false == slow_vars_start_wait_time)
	{
		absl_event_set_fromISR(slow_vars_event_group, read_slow_vars_event);
	}
	else
	{
		slow_vars_start_wait_time = false;

		if(true == slow_vars_running)
		{
			ade9000_set_slow_vars_period();

			absl_event_set_fromISR(slow_vars_event_group, read_slow_vars_event);
		}
	}
}

/** @brief Do the necessary actions to start the ade9000.
 *  First it wait until the TEMP_TRIM register has a valid value in case it
 *  is coming from a reset, and it saves the trim values.
 *  After that the ade9000 is configured as needed, if needed.
 */
bool ade9000_init(energy_sensor_init_conf_t* _ade9000_conf, absl_event_t* _event_group, uint32_t _fast_event_mask, uint32_t _slow_evet_mask)
{
	bool return_value = false;
	absl_time_t slowvars_start;

	ade9000_event_info_array = _ade9000_conf->ade9000_events_info_array;

	spi_ade9000_config 		  =	absl_config_get_spi_conf(_ade9000_conf->spi_conf_index);
	ade9000_irq0_config 	  =	absl_config_get_enet_event_conf(_ade9000_conf->enet_event_irq0_conf_index);
	ade9000_reset_gpio_config = absl_config_get_gpio_conf(_ade9000_conf->ade9000_reset_gpio_index);

	if(ABSL_SPI_RV_OK == absl_spi_init(&spi_ade9000, spi_ade9000_config, transmit_buffer, receive_buffer))
	{
		if(ABSL_GPIO_RV_OK == absl_gpio_init(&ade9000_reset_gpio, ade9000_reset_gpio_config, ABSL_GPIO_NO_INT))
		{
			absl_mutex_create(&spi_buff_mutex);
			absl_gpio_on(&ade9000_reset_gpio);

			absl_enet_event_init(&ade9000_irq0, ade9000_irq0_config, _event_group, _fast_event_mask);

			irq_timestamp = absl_enet_event_get_pointer_to_event_time_us(&ade9000_irq0);

			for(uint8_t wvf_var_index = 0; wvf_var_index < WFB_AMOUNT; wvf_var_index++)
			{
				wvf_position[wvf_var_index] = (float*)_ade9000_conf->wf_pointers + (wvf_var_index * WAFEFORM_DATA_SAMPLE_SIZE);
			}

			slow_vars_event_group = _event_group;
			read_slow_vars_event = _slow_evet_mask;

			slow_vars_start_wait_time = true;
			slow_vars_running 		  = false;
			slow_vars_first_read	  = true;
			first_waveform_read		  = false;

			slowvars_start.seconds = 1;
			slowvars_start.nseconds = 0;

			absl_timer_create(&absl_timer_slow_vars, &ade9000_slow_vars_call, NULL, slowvars_start, false, false);

#ifdef DEBUG_PIN
			debug_gpio_config = absl_config_get_gpio_conf(_ade9000_conf->debug_gpio_index);

			absl_gpio_init(&debug_gpio, debug_gpio_config, ABSL_GPIO_NO_INT);
			absl_gpio_off(&debug_gpio);
#endif

			return_value = true;
		}
		else
		{
			/* gpio initialization error */
		}
	}
	else
	{
		/* spi initialization error */
	}

	return return_value;
}

void ade9000_set_i_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos)
{
	wvf_i[_phase] = wvf_position[_buf_pos];
}

void ade9000_set_v_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos)
{
	wvf_v[_phase] = wvf_position[_buf_pos];
}

void ade9000_set_s_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos)
{
	wvf_s[_phase] = wvf_position[_buf_pos];
}

void ade9000_set_p_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos)
{
	wvf_p[_phase] = wvf_position[_buf_pos];
}

void ade9000_set_q_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos)
{
	wvf_q[_phase] = wvf_position[_buf_pos];
}


void ade9000_reboot(void)
{
	absl_gpio_off(&ade9000_reset_gpio);
	absl_thread_sleep(1);
	absl_gpio_on(&ade9000_reset_gpio);
}

bool ade9000_startup(void)
{
	bool return_value = false;

	if(true == ade9000_system_parameters())
	{
		ade9000_conversion_factors();

		if(ade9000_quickstart())
		{
			ade9000_print_config();
			ade9000_coeficients();
			ade9000_clear_waveform();
			absl_debug_printf("Energy Initialized\n");

			absl_enet_event_enable(&ade9000_irq0);
			absl_timer_start(&absl_timer_slow_vars);

			next_packet_wf_init_timestamp = 0;
			last_obtained_power_vars_timestamp = 0;
			return_value = true;
		}
	}
	else
	{
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_TC_VALUES);
	}

	return return_value;
}

bool ade9000_configure(void)
{
	bool return_value = false;

	if(true == ade9000_system_parameters())
	{
		ade9000_conversion_factors();

		if(ade9000_reconfig())
		{
			ade9000_conversion_factors();
			ade9000_print_config();
			ade9000_coeficients();

			next_packet_wf_init_timestamp = 0;
			last_obtained_power_vars_timestamp = 0;
			return_value = true;
		}
		else
		{
			ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_RECONFIG);
		}
	}
	else
	{
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_TC_VALUES);
	}

	return return_value;
}

void ade9000_continue_obtaining_fast(void)
{
	uint32_t reg = PAGE_FULL;

	if(ade9000_write_register(ADE9000_STATUS0, (uint8_t*)&reg))
	{
		absl_enet_event_enable(&ade9000_irq0);
	}
}

void ade9000_continue_obtaining_slow(void)
{
	if(false == slow_vars_start_wait_time)
	{
		if(false == slow_vars_running)
		{
			ade9000_set_slow_vars_period();

			slow_vars_running = true;
		}
		else
		{
			absl_timer_start(&absl_timer_slow_vars);
		}
	}
	else
	{
		slow_vars_running = true;
	}
}

bool ade9000_read_waveform(void* _fast_vars_config, void* _waveform_data)
{
	bool	  waveform_obtained = false;

	ABSL_UNUSED_ARG(_fast_vars_config);

	waveform_data_t* 	waveform_data = (waveform_data_t* )_waveform_data;
	fast_vars_config_t*	vars_config = (fast_vars_config_t*)_fast_vars_config;

	uint8_t   waveform_buffer_page;

	if(0 != next_packet_wf_init_timestamp)
	{
		if(ade9000_read_register(ADE9000_WFB_TRG_STAT, (uint8_t *)&waveform_buffer_status))
		{
			waveform_buffer_page = (waveform_buffer_status & 0xF000) >> 12;
			if(ade9000_read_burst(waveform_buffer_page, waveform_buff))
			{
				if((*irq_timestamp - next_packet_wf_init_timestamp) > (ADE9000_TIME_BETWEEN_IRQ_MS + 1000))
				{
					if(true == first_waveform_read)
					{
						ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_WAVEFORM_DATA_LOST);
						absl_debug_printf("Fast vars diff: %lld\n", *irq_timestamp - next_packet_wf_init_timestamp);
					}

					waveform_data->initial_timestamp = *irq_timestamp - ADE9000_TIME_BETWEEN_IRQ_MS;
				}
				else
				{
					waveform_data->initial_timestamp = next_packet_wf_init_timestamp;
				}

				waveform_data->final_timestamp = *irq_timestamp;
				next_packet_wf_init_timestamp =  *irq_timestamp;

				ade9000_get_data_to_send(waveform_buff, vars_config);

				if(true == first_waveform_read)
				{
					waveform_obtained = true;
				}

				first_waveform_read = true;
			}
		}
	}
	else
	{
		next_packet_wf_init_timestamp =  *irq_timestamp;
	}

	return waveform_obtained;
}

void ade9000_clear_waveform(void)
{
	first_waveform_read = false;
	next_packet_wf_init_timestamp = 0;

	samples_index = 0;

	for(uint8_t channel_index = 0; channel_index < NUM_CHANNELS; channel_index++)
	{
		memset(pwr_calc[channel_index].s_buf, 0, SAMPLES_POWER_CALC * SAMPLE_BYTES);
		memset(pwr_calc[channel_index].rms_buf, 0, SAMPLES_POWER_CALC * SAMPLE_BYTES);
		pwr_calc[channel_index].rms_acc = 0;
	}
}

bool ade9000_read_slow_variables(void* _slow_vars_config, void* _slow_vars_data)
{
	bool 	   variables_obtained = false;

	slow_vars_data_t* slow_vars_data = (slow_vars_data_t*)_slow_vars_data;
	slow_vars_config_t* slow_vars_config = (slow_vars_config_t*)_slow_vars_config;

	uint32_t variable_index;
	uint32_t register_index;

	uint32_t registers_read = 0;
	uint32_t read_time = slow_vars_config->vars_read_time;

	bool read_failed = false;

	if(first_waveform_read)
	{
		read_time += slow_vars_config->fast_vars_time;
	}

	for(variable_index = 0; variable_index < slow_vars_config->variable_count; variable_index++)
	{
		for(register_index = 0; register_index < slow_vars_config->registers[variable_index].reg_num; register_index++)
		{
			slow_vars_config->registers[variable_index].reg_info[register_index].reg_address->reg_already_read = false;
		}
	}

	for(variable_index = 0; variable_index < slow_vars_config->variable_count; variable_index++)
	{
		registers[registers_read].type = slow_vars_config->registers[variable_index].type;
		for(register_index = 0; register_index < slow_vars_config->registers[variable_index].reg_num; register_index++)
		{
			reg_info_t* reg_info = &slow_vars_config->registers[variable_index].reg_info[register_index];
			registers[registers_read].num = slow_vars_config->registers[variable_index].reg_num;

			if(false == reg_info->reg_address->reg_already_read)
			{
				reg_info->reg_address->value = 0;
				if(false == ade9000_ts_read_register(reg_info->reg_address->address, (uint8_t*)&reg_info->reg_address->value,
													 &slow_vars_data->timestamp))
				{
					read_failed = true;
				}
				else
				{
					reg_info->reg_address->reg_already_read = true;
					registers[registers_read].value = ade9000_extract_value(reg_info->reg_address->value, reg_info->initial_bit, reg_info->final_bit);
				}
			}
			else
			{
				registers[registers_read].value = ade9000_extract_value(reg_info->reg_address->value, reg_info->initial_bit, reg_info->final_bit);
			}

			registers_read++;
		}
	}

	if(false == slow_vars_first_read)
	{
		if(false == read_failed)
		{
			ade9000_fill_energy_data(slow_vars_data, registers, registers_read);

			if((slow_vars_data->timestamp - last_obtained_power_vars_timestamp) > slow_vars_config->data_lost_time)
			{
				ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTERS_DATA_LOST);
			}
			else if((slow_vars_data->timestamp - last_obtained_power_vars_timestamp) > read_time)
			{
				ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTERS_DATA_READ_LONG_TIME);
			}

			last_obtained_power_vars_timestamp = slow_vars_data->timestamp;

			variables_obtained = true;
		}
		else
		{
			ade9000_notify_system_event(ade9000_event_info_array, ADE9000_REGISTER_NOT_READ);
		}
	}
	else
	{
		last_obtained_power_vars_timestamp = slow_vars_data->timestamp;
		slow_vars_first_read = false;
	}

	return variables_obtained;
}

void ade9000_clear_slow_vars(void)
{
	slow_vars_first_read = true;
	slow_vars_running = false;
	last_obtained_power_vars_timestamp = 0;
}

absl_time_t ade9000_get_event_time(void)
{
	absl_time_t event_time;

	absl_enet_event_get_event_time(&ade9000_irq0, &event_time);

	return event_time;
}

uint64_t ade9000_get_event_time_ns(void)
{
	return absl_enet_event_get_event_time_ns(&ade9000_irq0);
}

static uint32_t ade9000_extract_value(uint32_t _reg_value, uint8_t _initial_bit, uint8_t _final_bit)
{
	uint32_t extracted_value = 0;

	extracted_value = _reg_value & GENMASK(_final_bit, _initial_bit);
	extracted_value = extracted_value  >> _initial_bit;

	return extracted_value;
}

static void ade9000_fill_energy_data(slow_vars_data_t* _energy_data, resgister_value_t*	_registers, uint32_t _var_count)
{
    uint8_t *result_cursor = (uint8_t*)&_energy_data->data;
    uint32_t actual_var = 0;

    while(actual_var <  _var_count)
    {
    	switch(_registers[actual_var].type)
    	{
    	case INTENSITY:
    	{
    		float value = ade9000_apply_intensity(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
    	break;
		case VOLTAGE:
		{
			float value = ade9000_apply_voltage(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case POWER:
		{
			float value = ade9000_apply_power(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case WATVAR:
		{
			double value = ade9000_apply_watvar(_registers[actual_var+1].value, _registers[actual_var].value);

			// Add to result buffer
			*(double*) result_cursor = value;
			result_cursor += sizeof(double);
			actual_var++;
		}
		break;
		case OI:
		{
			float value = ade9000_apply_oi(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case IPEAK:
		{
			float value = ade9000_apply_ipeak(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case VPEAK:
		{
			float value = ade9000_apply_vpeak(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case DIP_SWELL:
		{
			float value = ade9000_apply_dip_swell(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case THD:
		{
			float value = ade9000_apply_thd(_registers[actual_var].value);

			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case VAR_NONE:
		{
			// Add to result buffer
			*(uint32_t*) result_cursor = _registers[actual_var].value;
			result_cursor += sizeof(uint32_t);
		}
		break;
		case ANGLE:
		{
			float value = ade9000_apply_angle(_registers[actual_var].value);
			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case PERIOD:
		{
			float value = ade9000_apply_period(_registers[actual_var].value);
			// Add to result buffer
			*(float*) result_cursor = value;
			result_cursor += sizeof(float);
		}
		break;
		case SEQERR:
		{
			// Add to result buffer
			*(uint32_t*) result_cursor = _registers[actual_var].value;
			result_cursor += sizeof(uint32_t);

			uint32_t reg;

			ade9000_read_register(ADE9000_STATUS1, (uint8_t*)&reg);
			reg |= PHASE_SEQERR;
			ade9000_write_register(ADE9000_STATUS1, (uint8_t*)&reg);
		}
		break;
		default:
			ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTER_TYPE_UNKNOWN);
		break;
    	}
    	actual_var++;
    }
}

static void ade9000_get_data_to_send(void *_data, fast_vars_config_t* _config)
{
	float* p_aux = (float*)_data;

	unsigned int sample_index;
    for (sample_index = 0; sample_index < WAFEFORM_DATA_SAMPLE_SIZE; sample_index++)
    {
    	for(ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
    	{
        	if(_config->var_calculation_cb[phase_index] != NULL)
        	{
        		_config->var_calculation_cb[phase_index](phase_index, sample_index, (int32_t*)p_aux);
        	}
    	}

		samples_index = ABSL_INC_INDEX(samples_index, samples_s_cycle);
        p_aux += NUM_CHANNELS;
    }

    if(_config->calc_p_q)
	{
		ade9000_calc_q_and_p_pwr(_config);
	}
}

static bool ade9000_quickstart(void)
{
	uint32_t reg = 0;
	bool ret;

	/*
	 * The following config steps are defined in the following guide (page 50, Quick Start section):
	 *  - https://www.analog.com/media/en/technical-documentation/user-guides/ADE9000-UG-1098.pdf
	 * Note in the following code that some of them are not necessary for our purposes,
	 * so they are not included.
	 */

	/* Step 0 */
	ret = ade900_change_reg_protection(false);
	if(false == ret)
	{
		return ret;
	}

	/* Stop the measurements */
	reg = STOP_DSP;
	ret = ade9000_write_register(ADE9000_RUN, (uint8_t*)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Step 3 */
	/* We set the gain of all parameters to 1 */
	reg = GAIN_1;
	ret = ade9000_write_register(ADE9000_PGA_GAIN, (uint8_t*)&reg);
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_configuration();
	if (false == ret)
	{
		return ret;
	}

	/* Step 10b, 10c, 10d & 10e */
	/*
	 * 64 samples to evaluate the no-load condition,
	 * 8 KSPS mode, the internal energy register is added to the user
	 * accessible energy register enable the energy register read with
	 * reset feature
	 */
	ret = ade9000_set_ep_cfg();
	if (false == ret)
	{
		return ret;
	}

	reg = (uint32_t)(ENERGY_SAMPLES_PER_SECOND * ENERGY_ACCUMULATION_TIME) - 1;

	/* Step 10c */
	/* Energy accumulation update time configuration */
	ret = ade9000_write_register(ADE9000_EGY_TIME, (uint8_t *)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Step 11 */
	ret = ade9000_set_mask0();
	if (false == ret)
	{
		return ret;
	}

	/* Step 12 */

	/*
	 * Select the three phases to monitor their current and voltage peaks
	 * Set the overcurrent configuration
	 */
	ret = ade9000_set_config3();
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_set_thresholds(ENERGY_OVERCURRENT_THRESHOLD, ENERGY_DIP_LVL_THRESHOLD, ENERGY_SWELL_LVL_THRESHOLD);

	reg = 3000;
	ret = ade9000_write_register(ADE9000_SWELL_CYC, (uint8_t*)&reg);
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_write_register(ADE9000_DIP_CYC, (uint8_t*)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Step 13 */

	/*
	 * Waveform buffer configuration
	 * BURST_CHAN 0000 for all data in WFB_CFG register
	 * Current and voltage channel waveform samples,
	 * processed by the DSP (xI_PCF, xV_PCF) at 8 kSPS
	 */
	ret = ade9000_set_wfb_cfg();
	if (false == ret)
	{
		return ret;
	}


	/* WFB_PG_IRQEN: Enable IRQs for page 7 and 15 */
	ret = ade9000_set_wfb_pg_irq();
	if (false == ret)
	{
		return ret;
	}

	/* Set 1 incorrect cycle to set SEQERR, recommended in ADE9000 ref. manual*/
	reg = 1;
	ret = ade9000_write_register(ADE9000_SEQ_CYC, (uint8_t *)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Step 14 */
	/* RUN: enable DSP by setting RUN register = 1 */
	reg = RUN_DSP;
	ret = ade9000_write_register(ADE9000_RUN, (uint8_t *)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Command to start energy accumulator */
	ret = ade9000_enable_energy_acc();
	if (false == ret)
	{
		return ret;
	}

	/* Step 16 */
	ret = ade900_change_reg_protection(true);
	if (false == ret)
	{
		return ret;
	}

	absl_timer_start(&absl_timer_slow_vars);

	return ret;
}

static bool  ade9000_reconfig(void)
{
	bool ret;
	uint32_t reg = 0;

	/* Disable the register protection */
	ret = ade900_change_reg_protection(false);
	if(false == ret)
	{
		return ret;
	}

	/* Stop the measurements */
	reg = STOP_DSP;
	ret = ade9000_write_register(ADE9000_RUN, (uint8_t*)&reg);
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_configuration();
	if (false == ret)
	{
		return ret;
	}

	/* RUN: enable DSP by setting RUN register = 1 */
	reg = RUN_DSP;
	ret = ade9000_write_register(ADE9000_RUN, (uint8_t *)&reg);
	if (false == ret)
	{
		return ret;
	}

	/* Enable the register protection */
	ret = ade900_change_reg_protection(true);
	
	return ret;
}

static bool ade9000_configuration(void)
{
	bool ret;
	uint32_t adc_redirect[ADC_DIN_MAXVALUE];

	/*
	 * Enable the high-pass filters for all voltages and currents.
	 * Input is current transformer
	 */
	bool high_pass_filter_dis = ade9000_get_high_pass_filter_config();
	ret = ade9000_set_config0(high_pass_filter_dis);
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_set_xpgain(0);
	if (false == ret)
	{
		return ret;
	}

	/* Step 6, 8 & 10a */
	/*
	 * Configure expected fundamental frequency to 50Hz in ACCMODE register.
	 * Program the nominal voltage in VLEVEL WATTACC & VARACC set both as
	 * 00 -> Configured as signed accumulation mode
	 * VCONSEL -> 000 4-wire wye
	 */
	energy_fundamental_freq_t fundamental_frequency = ade9000_get_fund_freq_config();
	energy_hw_config_t hw_config = ade9000_get_hw_config();

	ret = ade9000_set_accmode(fundamental_frequency, hw_config);
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_check_current_invert(ADE9000_AIGAIN, ade9000_get_current_a_invert());
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_check_current_invert(ADE9000_BIGAIN, ade9000_get_current_b_invert());
	if (false == ret)
	{
		return ret;
	}

	ret = ade9000_check_current_invert(ADE9000_CIGAIN, ade9000_get_current_c_invert());
	if (false == ret)
	{
		return ret;
	}

	ade9000_get_adc_redirect(adc_redirect);

	ret = ade9000_config_adc_redirect(adc_redirect);

	return ret;
}

static bool	ade9000_set_config0(bool _hpf_dis)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_CONFIG0, (uint8_t *)&reg);
	if (true == ret)
	{
		if (_hpf_dis)
		{
			reg |= (HPFDIS);
		}
		else
		{
			reg &= ~(HPFDIS);
		}

		reg &= ~(INTEN);
		reg &= ~(DISRPLPF | DISAPLPF);
		reg |= ISUM_CFG; 				/* AI_PCF + BI_PCF + CI_PCF (for approximated neutral current rms calculation) */

		ret = ade9000_write_register(ADE9000_CONFIG0, (uint8_t*)&reg);
	}

	return ret;
}

static bool	ade9000_set_xpgain(uint32_t _x_pgain)
{
	bool ret;

	ret = ade9000_write_register(ADE9000_APGAIN, (uint8_t*)&_x_pgain);
	if (false != ret)
	{
		ret = ade9000_write_register(ADE9000_BPGAIN, (uint8_t*)&_x_pgain);
		if (false != ret)
		{
			ret = ade9000_write_register(ADE9000_CPGAIN, (uint8_t*)&_x_pgain);
		}
	}

	return ret;
}

static bool	ade9000_set_accmode(energy_fundamental_freq_t _fund_freq, energy_hw_config_t _hw_config)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_ACCMODE, (uint8_t *)&reg);
	if (false != ret)
	{
		if(false != ade9000_check_fund_freq(_fund_freq, &reg))
		{
			if(false != ade9000_check_hw_config(_hw_config, &reg))
			{
				reg &= ~(VARACC | WATTACC);

				ret = ade9000_write_register(ADE9000_ACCMODE, (uint8_t*)&reg);
			}
			else
			{
				ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_HW_CONFIG);
				ret = false;
			}
		}
		else
		{
			ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_FUND_FREQ);
			ret = false;
		}
	}

	return ret;
}

static bool	ade9000_config_adc_redirect(uint32_t* _adc_redirect)
{
	bool ret;
	uint32_t reg = 0;

	for(uint32_t adc_index = 0; adc_index < ADC_DIN_MAXVALUE; adc_index++)
	{
		reg |= _adc_redirect[adc_index]  << (3 * adc_index);
	}

	ret = ade9000_write_register(ADE9000_ADC_REDIRECT, (uint8_t*)&reg);

	return ret;
}

static bool	ade9000_set_ep_cfg(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_EP_CFG, (uint8_t *)&reg);
	if (false != ret)
	{
		reg &= ~(EGY_TMR_MODE | EGY_LD_ACCUM);
		reg |= RD_RST_EN | NOLOAD_TMR;

		ret = ade9000_write_register(ADE9000_EP_CFG, (uint8_t*)&reg);
	}

	return ret;
}

static bool	ade9000_set_mask0(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_MASK0, (uint8_t *)&reg);
	if (false != ret)
	{
		reg |= PAGE_FULL;

		ret = ade9000_write_register(ADE9000_MASK0, (uint8_t *)&reg);
	}

	return ret;
}

static bool	ade9000_set_config3(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_CONFIG3, (uint8_t *)&reg);
	if (false != ret)
	{
		reg |= PEAKSEL | OC_EN;

		ret = ade9000_write_register(ADE9000_CONFIG3, (uint8_t *)&reg);
	}

	return ret;
}

static bool	ade9000_set_thresholds(float _oc_threshold, float _dip_threshold, float _swell_threshold)
{
	bool ret;

	uint32_t reg;

	reg = _oc_threshold / (i_factor * FACTOR_COEF);

	/* Set the threshold of the over current */
	ret = ade9000_write_register(ADE9000_OILVL, (uint8_t *)&reg);
	if (false != ret)
	{
		reg = _dip_threshold / (v_factor * FACTOR_COEF);

		/* Set the threshold of the over current */
		ret = ade9000_write_register(ADE9000_DIP_LVL, (uint8_t *)&reg);
		if (false != ret)
		{
			reg = _swell_threshold / (v_factor * FACTOR_COEF);

			/* Set the threshold of the over current */
			ret = ade9000_write_register(ADE9000_SWELL_LVL, (uint8_t *)&reg);
		}
	}

	return ret;
}

static bool	ade9000_check_current_invert(uint16_t _xGAIN, bool _invert_current)
{
	bool ret;

	uint32_t reg;

	if(true == _invert_current)
	{
		reg = CURRENT_INVERTION_GAIN;
		ret = ade9000_write_register(_xGAIN, (uint8_t *)&reg);
	}
	else
	{
		ret = true;
	}

	return ret;
}

static bool	ade9000_set_wfb_cfg(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_WFB_CFG, (uint8_t *)&reg);
	if (false != ret)
	{
		reg &= ~BURST_CHAN;
		reg |= (BIT(9) | BIT(8));
		reg |= (CONT_FILL & CONT_FILL_FIELD);
		reg |= WF_CAP_SEL;
		reg |= WF_CAP_EN;

		ret = ade9000_write_register(ADE9000_WFB_CFG, (uint8_t *)&reg);
	}

	return ret;
}

static bool	ade9000_set_wfb_pg_irq(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_WFB_PG_IRQEN, (uint8_t *)&reg);
	if (false != ret)
	{
		reg |= (PAGE_IRQ_EN(7) | PAGE_IRQ_EN(15));

		ret = ade9000_write_register(ADE9000_WFB_PG_IRQEN, (uint8_t *)&reg);
	}

	return ret;
}

static bool ade9000_enable_energy_acc(void)
{
	bool ret;

	uint32_t reg = 0;

	ret = ade9000_read_register(ADE9000_EP_CFG, (uint8_t *)&reg);
	if (false != ret)
	{
		reg |= EGY_PWR_EN;

		ret = ade9000_write_register(ADE9000_EP_CFG, (uint8_t *)&reg);
	}

	return ret;
}

static bool ade9000_check_fund_freq(energy_fundamental_freq_t _fund_freq, uint32_t* _reg)
{
	bool valid_fund_freq = false;

	if (_fund_freq == FUNDAMENTAL_FREQ_50_HZ)
	{
		// 50Hz
		absl_debug_printf("OPTION: fundamental frequency 50Hz\r\n");
		*_reg &= ~SELFREQ;
		valid_fund_freq = true;
	}
	else if (_fund_freq == FUNDAMENTAL_FREQ_60_HZ)
	{
		// 60Hz
		absl_debug_printf("OPTION: fundamental frequency 60Hz\r\n");
		*_reg |= SELFREQ;
		valid_fund_freq = true;
	}

	return valid_fund_freq;
}

static bool ade9000_check_hw_config(energy_hw_config_t _hw_config, uint32_t* _reg)
{
	bool valid_hw_config = false;

	switch(_hw_config)
	{
	case HW_CONF_3_WIRE_1PH_NEUTRAL:
	case HW_CONF_3_WIRE_NETWORK_NEUTRAL:
	case HW_CONF_4_WIRE_WYE_NEUTRAL:
	case HW_CONF_4_WIRE_WYE_ISOLATED:
	case HW_CONF_4_WIRE_DELTA_NEUTRAL:
	case HW_CONF_MULTIPLE_1PH_NEUTRAL:
		absl_debug_printf("OPTION: 4 wire wye hw configuration\r\n");
		*_reg &= ~VCONSEL_111;
		valid_hw_config = true;
		break;
	case HW_CONF_3_WIRE_DELTA_ISOLATED_VB:
	case HW_CONF_3_WIRE_DELTA_PHASEB:
		*_reg |= VCONSEL_001;
		valid_hw_config = true;
		break;
	case HW_CONF_4_WIRE_WYE_NONBLONDEL_NEUTRAL:
		*_reg |= VCONSEL_010;
		valid_hw_config = true;
		break;
	case HW_CONF_4_WIRE_DELTA_NONBLONDEL_NEUTRAL:
		*_reg |= VCONSEL_011;
		valid_hw_config = true;
		break;
	case HW_CONF_3_WIRE_DELTA_ISOLATED_VA_VB_VC:
		*_reg |= VCONSEL_100;
		valid_hw_config = true;
		break;
	case HW_CONF_UNKNOWN:
	default:
		break;
	}

	return valid_hw_config;
}

static bool ade9000_system_parameters(void)
{
	bool valid_tc_values = false;

	uint32_t tc_primary_side = ade9000_get_tc_primary_config();
	uint32_t tc_secondary_side = ade9000_get_tc_secondary_config();

	if((0 != tc_primary_side) && (0 != tc_secondary_side))
	{
		c_tf = BURDEN_RESISTOR / (tc_primary_side /tc_secondary_side);
		v_tf = R2 / (float)(R1 + R2);
		c_adc = I_NOM * c_tf;
		v_adc = V_NOM * v_tf;
		i_fsp = c_adc / VOLTAGE_ADC_FULL_SCALE_RMS;
		v_fsp = v_adc / VOLTAGE_ADC_FULL_SCALE_RMS;

		valid_tc_values = true;
	}
	else
	{
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_TC_VALUES_ERROR);
	}

	return valid_tc_values;
}

static void ade9000_conversion_factors(void)
{
	i_factor = ((I_NOM) / (i_fsp * FULLSCALE_VI_RMS_CODES));
	v_factor = ((V_NOM) / (v_fsp * FULLSCALE_VI_RMS_CODES));
	p_factor = ((I_NOM * V_NOM) / (i_fsp * v_fsp * FULLSCALE_POWER_CODES));
	e_factor = ((I_NOM * V_NOM) / (i_fsp * v_fsp * FULLSCALE_POWER_CODES *
				ADE9000_SAMPLING_FREQUENCY * ADE9000_ACCUMULATION_TIME));
}

static void ade9000_print_config(void)
{
    absl_debug_printf("R1: %d\n", R1);
    absl_debug_printf("R2: %d\n", R2);

    absl_debug_printf("I_conversion_factor:    %.20f\n", i_factor);
    absl_debug_printf("V_conversion_factor:    %.20f\n", v_factor);
    absl_debug_printf("P_conversion_factor:    %.20f\n", p_factor);
    absl_debug_printf("E_conversion_factor:    %.20f\n", e_factor);
}

static void ade9000_generate_hamming_window(void)
{
    const float alpha = 0.54f;
    const float beta = 1.0f - alpha; // 0.46f
    const float pi_div_Nminus1 = (float)M_PI * 2.0f / (float)(SAMPLES_POWER_CALC - 1);

    for (uint32_t n = 0; n < SAMPLES_POWER_CALC; n++)
    {
        hamming_window[n] = alpha - beta * cosf(pi_div_Nminus1 * (float)n);
    }
}

static void ade9000_calc_cycle_values(int32_t _samples_in_cycle)
{
	uint32_t cycles;
	uint32_t empty_pos_amount;

#ifdef  FLOAT_PWR_CALC
	arm_rfft_fast_init_f32(&fftInstance, SAMPLES_POWER_CALC);
#else
	arm_rfft_fast_init_f64(&fftInstance, SAMPLES_POWER_CALC);
#endif

	cycles = SAMPLES_POWER_CALC / _samples_in_cycle;
	samples_s_cycle = cycles * _samples_in_cycle;
	empty_pos_amount =  SAMPLES_POWER_CALC - samples_s_cycle;
	empty_initial_positons = empty_pos_amount / 2;

#ifdef ADE9000_PWR_HAMMING_WINDOW
	fft_coef = 2.0f / (SAMPLES_POWER_CALC  * 0.54f); /* 0.54 standard hamming coefficient */
#else
	fft_coef = 2.0f / (SAMPLES_POWER_CALC);
#endif

	ade9000_generate_hamming_window();
}

static void ade9000_coeficients(void)
{
	energy_fundamental_freq_t fundamental_frequency = ade9000_get_fund_freq_config();
	if(FUNDAMENTAL_FREQ_50_HZ == fundamental_frequency)
	{
		ade9000_calc_cycle_values(SAMPLES_CYCLE_50HZ);
		angle_factor = ANGLE_50HZ;
	}
	else if(FUNDAMENTAL_FREQ_60_HZ == fundamental_frequency)
	{
		ade9000_calc_cycle_values(SAMPLES_CYCLE_60HZ);
		angle_factor = ANGLE_60HZ;
	}
	else
	{
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_INCORRECT_FUND_FREQ_ERROR);
	}
}

static bool ade900_change_reg_protection(bool _enable)
{
	uint32_t reg = 0;

	/* Disable the register protection */
	if(_enable)
	{
		reg = ENABLE_REG_PROT_CMD;
	}
	else
	{
		reg = DISABLE_REG_PROT_CMD;
	}


	if (false == ade9000_write_register(ADE9000_WR_LOCK, (uint8_t*)&reg))
	{
		return false;
	}

	return true;
}

/** @brief Reads the value of the given register
 *
 * @param _register				Register to be read
 * @param _read_register_data	Buffer with register data
 * @param _length				Bytes length of the register
 * @return bool					true: Data correctly read
 * 								false: Error reading register data
 */
static bool ade9000_read_register(uint16_t _register, uint8_t* _read_register_data)
{
	bool return_value = false;
	uint32_t bytes_len;

	absl_spi_rv_t spi_rv;

	absl_mutex_take(&spi_buff_mutex);

	bytes_len = (_register >= 0x480 && _register <= 0x4FE) ? 2 : 4;

	transmit_buffer[0] = (_register << 4) >> 8;
	transmit_buffer[1] = (_register << 4) | ADE9000_READ_REGISTER;

	spi_rv = absl_spi_transfer(&spi_ade9000, ADE9000_REGISTER_SIZE + bytes_len);

	if (ABSL_SPI_RV_OK == spi_rv)
	{
		for(uint8_t buff_index = 0; buff_index < bytes_len; buff_index++)
		{
			_read_register_data[bytes_len - (1 + buff_index)] = receive_buffer[ADE9000_REGISTER_SIZE + buff_index];
		}

		return_value= true;
	}
	else
	{
		absl_debug_printf("LPSPI master transfer completed with error.\r\n");
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTER_READ);
	}

	absl_mutex_give(&spi_buff_mutex);

	return return_value;
}

/** @brief Reads the value of the given register
 *
 * @param _register				Register to be read
 * @param _read_register_data	Buffer with register data
 * @param _length				Bytes length of the register
 * @return bool					true: Data correctly read
 * 								false: Error reading register data
 */
static bool ade9000_ts_read_register(uint16_t _register, uint8_t* _read_register_data, uint64_t* _timestamp)
{
	bool return_value = false;
	uint32_t bytes_len;

	absl_spi_rv_t spi_rv;

	absl_mutex_take(&spi_buff_mutex);

	bytes_len = (_register >= 0x480 && _register <= 0x4FE) ? 2 : 4;

	transmit_buffer[0] = (_register << 4) >> 8;
	transmit_buffer[1] = (_register << 4) | ADE9000_READ_REGISTER;

	spi_rv = absl_spi_ts_transfer(&spi_ade9000, ADE9000_REGISTER_SIZE + bytes_len, _timestamp);

	if (ABSL_SPI_RV_OK == spi_rv)
	{
		for(uint8_t buff_index = 0; buff_index < bytes_len; buff_index++)
		{
			_read_register_data[bytes_len - (1 + buff_index)] = receive_buffer[ADE9000_REGISTER_SIZE + buff_index];
		}

		return_value = true;
	}
	else
	{
		absl_debug_printf("LPSPI master transfer completed with error.\r\n");
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTER_READ);
	}

	absl_mutex_give(&spi_buff_mutex);

	return return_value;
}

/** @brief Reads the value of the given register
 *
 * @param _register				Register to be read
 * @param _write_register_data	Buffer with register data
 * @return bool					true: Data correctly read
 * 								false: Error reading register data
 */
static bool ade9000_write_register(uint16_t _register, uint8_t* _write_register_data)
{
	bool 		return_value = false;
	uint32_t	read_value = 0;
	uint32_t 	bytes_len;

    absl_spi_rv_t spi_rv;

    absl_mutex_take(&spi_buff_mutex);

	bytes_len = (_register >= 0x480 && _register <= 0x4FE) ? 2 : 4;

	transmit_buffer[0] = (_register << 4) >> 8;
	transmit_buffer[1] = (_register << 4) | ADE9000_WRITE_REGISTER;

	for(uint8_t buff_index = 0; buff_index < bytes_len; buff_index++)
	{
		transmit_buffer[ADE9000_REGISTER_SIZE + buff_index] = _write_register_data[(bytes_len - 1) -  buff_index];
	}

	spi_rv = absl_spi_transfer(&spi_ade9000, ADE9000_REGISTER_SIZE + bytes_len);
	absl_mutex_give(&spi_buff_mutex);
	if (ABSL_SPI_RV_OK == spi_rv)
	{
		if(true == ade9000_read_register(_register, (uint8_t *)&read_value))
		{
			uint32_t written_value = 0;
			memcpy(&written_value, _write_register_data, bytes_len);

			if(true == ade9000_check_written_register(_register, written_value, read_value))
			{	
				return_value = true;
			}
			else
			{
				absl_debug_printf("Read register data differs from the data written, make sure that the ADE9000 is correctly connected.\r\n");
				ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTER_NOT_WRITTEN);
			}
		}
		else
		{
			absl_debug_printf("LPSPI master transfer completed with error.\r\n");
		}
	}
	else
	{
		absl_debug_printf("LPSPI master transfer completed with error.\r\n");
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_REGISTER_WRITE);
	}

	return return_value;
}

static bool ade9000_check_written_register(uint16_t _register, uint32_t _written_value, uint32_t _read_value)
{
	uint32_t value_to_compare = 0;

	switch(_register)
	{
	case ADE9000_OILVL:
	case ADE9000_DIP_LVL:
	case ADE9000_SWELL_LVL:
		/* The last byte reserved, so they are not checked */
		value_to_compare = _written_value & 0x00FFFFFF;
		break;
	case ADE9000_EGY_TIME:
		/* max value is 0x1FFF */
		value_to_compare = _written_value & 0x1FFF;
		break;
	case ADE9000_WR_LOCK:
		/* when write register disabled the read value is 0
		 * and enabled 1 */
		if(DISABLE_REG_PROT_CMD == _written_value)
		{
			value_to_compare = 0;
		}
		else if(ENABLE_REG_PROT_CMD == _written_value)
		{
			value_to_compare = 1;
		}
		break;
	case ADE9000_STATUS0:
	case ADE9000_STATUS1:
		/* bits in register, could have changed already */
		return true;
		break;
	default:
		value_to_compare = _written_value;
		break;
	}

	if(value_to_compare != _read_value)
	{
		return false;
	}

	return true;
}

static bool ade9000_read_burst(uint16_t _page, uint8_t* _buff)
{
	bool 		return_value = false;
	absl_spi_rv_t spi_rv;
	uint16_t 	page_cmd = 0;
	uint16_t 	tmp = 0;

	switch(wvf_half_to_expect)
	{
	case WVF_FIRST_HALF:
		if (_page >= PAGE_HALF_IDX && _page < PAGE_FULL_IDX )
		{
			wvf_half_to_expect = WVF_SECOND_HALF;
			page_cmd = CMD_BURST_1ST_HALF;
		}
		else
		{
			return return_value;
		}
		break;
	case WVF_SECOND_HALF:
		if (_page == PAGE_FULL_IDX || _page < PAGE_HALF_IDX)
		{
			wvf_half_to_expect = WVF_FIRST_HALF;
			page_cmd = CMD_BURST_2ND_HALF;
		}
		else
		{
			return return_value;
		}
		break;
	default:
		return return_value;
		break;
	}

	absl_mutex_take(&spi_buff_mutex);
	tmp = (page_cmd << 4) | ADE9000_READ_REGISTER;

	transmit_buffer[0] = (uint8_t)(tmp >> 8);
	transmit_buffer[1] = (uint8_t)tmp;

	spi_rv = absl_spi_transfer(&spi_ade9000, PAGE_SIZE_BYTES);

	if (ABSL_SPI_RV_OK == spi_rv)
	{
		memcpy(_buff, &receive_buffer[ADE9000_REGISTER_SIZE], WAVEFORM_BUFF_SIZE);
		ade9000_data_rearrange(_buff, WAVEFORM_BUFF_SIZE/4);

		return_value= true;
	}
	else
	{
		absl_debug_printf("LPSPI master transfer completed with error.\r\n");
		ade9000_notify_system_event(ade9000_event_info_array, ADE9000_EVENTS_BURST_READ);
	}

	absl_mutex_give(&spi_buff_mutex);

	return return_value;
}

static void ade9000_data_rearrange(void *_data, unsigned int _bytes)
{
	uint32_t *p_aux = _data;
	uint16_t i;

	for (i=0; i < _bytes; i++)
	{
		p_aux[i] = ((p_aux[i] << 16) | (p_aux[i] >> 16));
	}

	uint16_t *p_aux2 = _data;

	for (i=0; i < _bytes*2; i++)
	{
		p_aux2[i] = ((p_aux2[i] << 8) | (p_aux2[i] >> 8));
	}
}

static float ade9000_apply_intensity(int32_t _data)
{
    return ((float)_data * i_factor);
}

static float ade9000_apply_voltage(int32_t _data)
{
    return ((float)_data * v_factor);
}

static float ade9000_apply_power(int32_t _data)
{
    float ret = (float)_data * POWER_FACTOR_COEF;

    return (ret >= 31) ? (ret - 32) : ret;
}

static float ade9000_calc_s_pwr(float _v_sample, float _i_sample, ade9000_wfb_samples_t _v_sample_type,
							   ade9000_wfb_samples_t _i_sample_type, energy_data_ph_t* _e_data_ph_idx)
{
	energy_pwr_sample_t* _v_pwr_sample = &pwr_calc[_v_sample_type];
	energy_pwr_sample_t* _i_pwr_sample = &pwr_calc[_i_sample_type];

	ade9000_fill_samples_buffers(_v_sample, _v_pwr_sample);
	ade9000_fill_samples_buffers(_i_sample, _i_pwr_sample);

	/* RMS */
	_e_data_ph_idx->i_rms = sqrt(_i_pwr_sample->rms_acc / samples_s_cycle);
	_e_data_ph_idx->v_rms = sqrt(_v_pwr_sample->rms_acc / samples_s_cycle);

	return   _e_data_ph_idx->i_rms * _e_data_ph_idx->v_rms;
}

static void ade9000_fill_samples_buffers(float _sample, energy_pwr_sample_t* _pwr_sample)
{
	_pwr_sample->s_buf[samples_index] = _sample;
	_pwr_sample->rms_acc -= _pwr_sample->rms_buf[samples_index];
	_pwr_sample->rms_buf[samples_index] = _sample * _sample;
	_pwr_sample->rms_acc += _pwr_sample->rms_buf[samples_index];
}

static void ade9000_clear_aux_buff_partial(uint32_t empty_initial_positions, uint32_t samples_in_cycle)
{
    uint32_t empty_final_positions = SAMPLES_POWER_CALC - (empty_initial_positions + samples_in_cycle);

    if (empty_initial_positions > 0)
    {
        memset(aux_buff, 0, empty_initial_positions * SAMPLE_BYTES);
    }

    if (empty_final_positions > 0)
    {
        memset(&aux_buff[empty_initial_positions + samples_in_cycle], 0, empty_final_positions * SAMPLE_BYTES);
    }
}

#ifdef FLOAT_PWR_CALC
static inline void ade9000_apply_hamming_window(float* _buffer, uint32_t _length)
#else
static inline void ade9000_apply_hamming_window(double* _buffer, uint32_t _length)
#endif
{
    for (uint32_t i = 0; i < _length; i++)
    {
#ifdef FLOAT_PWR_CALC
    	_buffer[i] *= (float)hamming_window[i];
#else
        _buffer[i] *= (double)hamming_window[i];
#endif

    }
}

#ifdef FLOAT_PWR_CALC
static void ade9000_calc_fft(float* _samples, float* _complex_buff, uint32_t _first_part, uint32_t _second_part)
#else
static void ade9000_calc_fft(double* _samples, double* _complex_buff, uint32_t _first_part, uint32_t _second_part)
#endif
{
	ade9000_clear_aux_buff_partial(empty_initial_positons, samples_s_cycle);
	memcpy(&aux_buff[empty_initial_positons], &_samples[samples_index], _first_part);
	memcpy(&aux_buff[empty_initial_positons + (samples_s_cycle - samples_index)],
		   _samples, _second_part);

#ifdef ADE9000_PWR_HAMMING_WINDOW
	ade9000_apply_hamming_window(aux_buff, SAMPLES_POWER_CALC);
#endif

#ifdef FLOAT_PWR_CALC
	arm_rfft_fast_f32(&fftInstance, aux_buff, _complex_buff, 0);
#else
	arm_rfft_fast_f64(&fftInstance, aux_buff, _complex_buff, 0);
#endif
}

#define FFT_INTERPOLATION_START (COMPLEX_SAMPLES - WAFEFORM_DATA_SAMPLE_SIZE)

static float p_acc_last[PHASE_MAXNUM] = {0};
static float q_acc_last[PHASE_MAXNUM] = {0};


// Explanation of FFT-based power calculation and interpolation
// ============================================================
// We compute active (P) and reactive (Q) power using FFT over 512-sample windows.
// This provides spectral resolution and allows separating signal components.
//
// However, computing a full FFT for every new sample is computationally very expensive.
// So, the next steps are done:
//  - Compute FFT and spectral power every 128 samples
//  - Store current and previous FFT-derived values of P and Q
//  - Apply linear interpolation across 128 points:
//        interpolated_value = (1 - α) * previous + α * current
//        with α = index / (N - 1), where N is 128 (WAFEFORM_DATA_SAMPLE_SIZE)
// This provides a sample-by-sample estimate of P and Q while maintaining smooth evolution.
//
// Additionally, a Hamming window is applied to the input before FFT to reduce spectral leakage.
//
// FFT sampling window = 512 samples (SAMPLES_POWER_CALC)
// This ensures good frequency resolution (approx. 15.6Hz at 8kHz sampling)
//
// Interpolation between successive P/Q values is physically valid because:
//  - Electrical power signals change slowly relative to sampling (50/60Hz vs 8kHz)
//  - FFT-derived power approximates average power per segment
//  - Linear interpolation maintains continuity without distortion
static void ade9000_calc_q_and_p_pwr(fast_vars_config_t* _vars_config)
{
	uint32_t power_calc_index = 0;

	uint32_t firt_buff_part = (samples_s_cycle - samples_index) * SAMPLE_BYTES;
	uint32_t second_buff_part = samples_index * SAMPLE_BYTES;

#ifdef FLOAT_PWR_CALC
	float p_acc[PHASE_MAXNUM] = {0.0};
	float q_acc[PHASE_MAXNUM] = {0.0};
#else
	double p_acc[PHASE_MAXNUM] = {0.0};
	double q_acc[PHASE_MAXNUM] = {0.0};
#endif

	for(ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
	{
		if(_vars_config->pwr_p_q_calc[phase_index] != CALC_PWR_NONE)
		{
			ade9000_calc_fft(v_samples[phase_index], v_complex[phase_index], firt_buff_part, second_buff_part);
			ade9000_calc_fft(i_samples[phase_index], i_complex[phase_index], firt_buff_part, second_buff_part);
		}
	}

	for (uint32_t i = 0; i < COMPLEX_SAMPLES; i++)
	{
		uint32_t r = 2 * i;
		uint32_t im = (2 * i) + 1;

		for(ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
		{
			if (_vars_config->pwr_p_q_calc[phase_index] == CALC_PWR_NONE)
			{
				/* No calculation needed */
			}
			else
			{
#ifdef FLOAT_PWR_CALC
				float* v = v_complex[phase_index];
				float* i = i_complex[phase_index];
#else
				double* v = v_complex[phase_index];
				double* i = i_complex[phase_index];
#endif
				if (_vars_config->pwr_p_q_calc[phase_index] == CALC_PWR_BOTH)
				{
					p_acc[phase_index] += v[r] * i[r] + v[im] * i[im];
					q_acc[phase_index] += v[im] * i[r] - v[r] * i[im];
				}
				else if (_vars_config->pwr_p_q_calc[phase_index] == CALC_PWR_P)
				{
					p_acc[phase_index] += v[r] * i[r] + v[im] * i[im];
				}
				else if (_vars_config->pwr_p_q_calc[phase_index] == CALC_PWR_Q)
				{
					q_acc[phase_index] += v[im] * i[r] - v[r] * i[im];
				}
			}
		}

		if (i >= FFT_INTERPOLATION_START)
		{
#ifdef FLOAT_PWR_CALC
			float interp_factor = (float)power_calc_index / (WAFEFORM_DATA_SAMPLE_SIZE - 1);
#else
			double interp_factor = (double)power_calc_index / (WAFEFORM_DATA_SAMPLE_SIZE - 1);
#endif

			for (ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
			{
				switch (_vars_config->pwr_p_q_calc[phase_index])
				{
					case CALC_PWR_P:
						wvf_p[phase_index][power_calc_index] = fft_coef * ((1.0 - interp_factor) * p_acc_last[phase_index] +
													           interp_factor * p_acc[phase_index]);
					break;
					case CALC_PWR_Q:
						wvf_q[phase_index][power_calc_index] = fft_coef * ((1.0 - interp_factor) * q_acc_last[phase_index] +
															   interp_factor * q_acc[phase_index]);
						break;
					case CALC_PWR_BOTH:
						wvf_p[phase_index][power_calc_index] = fft_coef * ((1.0 - interp_factor) * p_acc_last[phase_index] +
															   interp_factor * p_acc[phase_index]);
						wvf_q[phase_index][power_calc_index] = fft_coef * ((1.0 - interp_factor) * q_acc_last[phase_index] +
															   interp_factor * q_acc[phase_index]);
						break;
					default:
						break;
				}
			}

			power_calc_index++;
		}
	}



	for (ade9000_phases_t phase_index = 0; phase_index < PHASE_MAXNUM; phase_index++)
	{
		p_acc_last[phase_index] = p_acc[phase_index];
		q_acc_last[phase_index] = q_acc[phase_index];
	}
}

static double ade9000_apply_watvar(uint32_t _high, uint32_t _low)
{
    int64_t aux = ((int64_t)_high << 13) | (_low & 0x00001FFF);

    int64_t aux2 = (aux & 0x0000020000000000) ? (aux |= 0xFFFFFC0000000000) : aux;

    return (double)aux2 * e_factor;
}

static  float ade9000_apply_oi(uint32_t _data)
{
    return (float)_data * i_factor * FACTOR_COEF;
}

static  float ade9000_apply_ipeak(uint32_t _data)
{
    return (float)_data * i_factor * FACTOR_COEF;
}

static  float ade9000_apply_vpeak(uint32_t _data)
{
    return (float)_data * v_factor * FACTOR_COEF;
}

static  float ade9000_apply_dip_swell(uint32_t _data)
{
    return (float)_data * v_factor * FACTOR_COEF;
}

static float ade9000_apply_thd(int32_t _data)
{
    return (float)_data * POWER_FACTOR_COEF * 100;
}

static float ade9000_apply_angle(int32_t _data)
{
    return (float)_data * angle_factor;
}

static float ade9000_apply_period(int32_t _data)
{
    return (float)(_data + 1) / PERIOD_FACTOR * 1000; /* ms */
}

static void ade9000_set_slow_vars_period(void)
{
	absl_time_t slowvars_read_time;

	float slow_vars_period = ade9000_get_slow_vars_period();

	uint64_t sec= (uint64_t)slow_vars_period;
	uint32_t nsec = (slow_vars_period - sec) * 1000000000;

	slowvars_read_time.seconds = sec;
	slowvars_read_time.nseconds = nsec;

	absl_timer_change(&absl_timer_slow_vars, slowvars_read_time, true);
}

void ade9000_calc_i(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
}

void ade9000_calc_v(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
}

void ade9000_calc_v_i(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
}

void ade9000_calc_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_i;
	float aux_v;

	aux_i = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	aux_v = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
	wvf_s[_phase][_sample] = ade9000_calc_s_pwr(aux_i, aux_v, v_phases[_phase], i_phases[_phase], &e_data[_phase]);
}

void ade9000_calc_i_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_v;

	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	aux_v = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
	wvf_s[_phase][_sample] = ade9000_calc_s_pwr(wvf_i[_phase][_sample], aux_v, v_phases[_phase], i_phases[_phase], &e_data[_phase]);
}

void ade9000_calc_v_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_i;

	aux_i = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
	wvf_s[_phase][_sample] = ade9000_calc_s_pwr(aux_i, wvf_v[_phase][_sample], v_phases[_phase], i_phases[_phase], &e_data[_phase]);
}

void ade9000_calc_v_i_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos + v_phases[_phase]));
	wvf_s[_phase][_sample] = ade9000_calc_s_pwr(wvf_i[_phase][_sample], wvf_v[_phase][_sample], v_phases[_phase], i_phases[_phase], &e_data[_phase]);
}


void ade9000_calc_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_i;
	float aux_v;

	ABSL_UNUSED_ARG(_sample);

	aux_i = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	ade9000_fill_samples_buffers(aux_i, &pwr_calc[i_phases[_phase]]);
	aux_v = ade9000_apply_voltage(*(_data_pos  + v_phases[_phase]));
	ade9000_fill_samples_buffers(aux_v, &pwr_calc[v_phases[_phase]]);
}

void ade9000_calc_i_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_v;

	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	ade9000_fill_samples_buffers(wvf_i[_phase][_sample], &pwr_calc[i_phases[_phase]]);
	aux_v = ade9000_apply_voltage(*(_data_pos  + v_phases[_phase]));
	ade9000_fill_samples_buffers(aux_v, &pwr_calc[v_phases[_phase]]);
}

void ade9000_calc_v_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	float aux_i;

	aux_i = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	ade9000_fill_samples_buffers(aux_i, &pwr_calc[i_phases[_phase]]);
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos  + v_phases[_phase]));
	ade9000_fill_samples_buffers(wvf_v[_phase][_sample], &pwr_calc[v_phases[_phase]]);
}

void ade9000_calc_v_i_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos)
{
	wvf_i[_phase][_sample] = ade9000_apply_intensity(*(_data_pos + i_phases[_phase]));
	ade9000_fill_samples_buffers(wvf_i[_phase][_sample], &pwr_calc[i_phases[_phase]]);
	wvf_v[_phase][_sample] = ade9000_apply_voltage(*(_data_pos  + v_phases[_phase]));
	ade9000_fill_samples_buffers(wvf_v[_phase][_sample], &pwr_calc[v_phases[_phase]]);
}

