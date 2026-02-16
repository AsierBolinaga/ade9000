/*
 * absl_input_output_event.h
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_ENET_EVENT_H_
#define ABSL_ENET_EVENT_H_

#include "absl_config.h"
#ifdef ABSL_ENET_EVENT
#include "absl_time.h"
#include "absl_event.h"
#include "absl_phy.h"
#include "fsl_enet.h"

typedef struct absl_enet_event_config
{
	absl_phy_config_t*				event_phy;
	enet_ptp_timer_channel_mode_t	channel_mode;
	enet_ptp_timer_channel_t		channel;
}absl_enet_event_config_t;

typedef struct absl_enet_event
{
	absl_enet_event_config_t*		event_config;
	absl_time_t					event_time;
	absl_event_t*					event_group;
	uint32_t					signal_to_give;
	uint8_t 					enet_phy;
	uint64_t 					actual_time_us;
	uint64_t 					previous_time_us;
}absl_enet_event_t;


#if defined(ABSL_IMX_RT10XX)
#define absl_enet_event_init								absl_enet_event_init_imxrt10xx
#define absl_enet_event_enable							absl_enet_event_enable_imxrt10xx
#define absl_enet_event_disable							absl_enet_event_disable_imxrt10xx
#define absl_enet_event_get_event_time					absl_enet_event_get_event_time_imxrt10xx
#define absl_enet_event_get_event_time_ns					absl_enet_event_get_event_time_ns_imxrt10xx
#define absl_enet_event_get_event_time_us					absl_enet_event_get_event_time_us_imxrt10xx
#define absl_enet_event_get_pointer_to_event_time_us		absl_enet_event_get_pointer_to_event_time_us_imxrt10xx
#else
#error Platform not defined
#endif

void absl_enet_event_init(absl_enet_event_t * _absl_enet_event, absl_enet_event_config_t* _io_event_config,
						absl_event_t* _event_group, uint32_t _event_mask);

void absl_enet_event_enable(absl_enet_event_t * _absl_enet_event);

void absl_enet_event_disable(absl_enet_event_t * _absl_enet_event);

void absl_enet_event_get_event_time(absl_enet_event_t * _absl_enet_event, absl_time_t* _event_time);

uint64_t absl_enet_event_get_event_time_ns(absl_enet_event_t* _io_event);

uint64_t absl_enet_event_get_event_time_us(absl_enet_event_t* _io_event);

uint64_t* absl_enet_event_get_pointer_to_event_time_us(absl_enet_event_t* _io_event);

#endif
#endif /* ABSL_ENET_EVENT_H_ */
