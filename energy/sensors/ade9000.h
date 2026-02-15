/*
 * ade9000.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ADE9000_H_
#define ADE9000_H_

#include "sensors_types.h"

#include "pl_time.h"
#include "pl_event.h"

typedef enum ade9000_events
{
	 ADE9000_EVENTS_RECONFIG = 0,
	 ADE9000_EVENTS_REGISTER_READ,
	 ADE9000_EVENTS_REGISTER_WRITE,
	 ADE9000_EVENTS_BURST_READ,
	 ADE9000_EVENTS_WAVEFORM_DATA_LOST,
	 ADE9000_EVENTS_REGISTERS_DATA_LOST,
	 ADE9000_EVENTS_INCORRECT_FUND_FREQ,
	 ADE9000_EVENTS_REGISTER_NOT_WRITTEN,
	 ADE9000_EVENTS_INCORRECT_HW_CONFIG,
	 ADE9000_EVENTS_INCORRECT_TC_VALUES,
	 ADE9000_EVENTS_REGISTER_TYPE_UNKNOWN,
	 ADE9000_EVENTS_INCORRECT_FUND_FREQ_ERROR,
	 ADE9000_EVENTS_INCORRECT_TC_VALUES_ERROR,
	 ADE9000_REGISTER_NOT_FOUND,
	 ADE9000_REGISTER_NOT_READ,
	 ADE9000_EVENTS_REGISTERS_DATA_READ_LONG_TIME,
	 ADE9000_EVENTS_MAXVALUE
}ade9000_events_t;

bool ade9000_init(energy_sensor_init_conf_t* _ade9000_conf, pl_event_t* _event_group, uint32_t _fast_event_mask, uint32_t _slow_evet_mask);
void ade9000_set_i_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos);
void ade9000_set_v_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos);
void ade9000_set_s_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos);
void ade9000_set_p_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos);
void ade9000_set_q_buff_position(ade9000_phases_t _phase, uint32_t _buf_pos);
bool ade9000_startup(void);
bool ade9000_configure(void);
void ade9000_reboot(void);
void ade9000_continue_obtaining_fast(void);
void ade9000_continue_obtaining_slow(void);
bool ade9000_read_waveform(void* _fast_vars_config, void* _waveform_data);
void ade9000_clear_waveform(void);
bool ade9000_read_slow_variables(void* _slow_vars_config, void* _slow_vars_data);
void ade9000_clear_slow_vars(void);
pl_time_t ade9000_get_event_time(void);
uint64_t ade9000_get_event_time_ns(void);

void ade9000_calc_i(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v_i(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_i_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v_i_s(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_i_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);
void ade9000_calc_v_i_powers(ade9000_phases_t _phase, int _sample, int32_t* _data_pos);

#endif /* ADE9000_H_ */
