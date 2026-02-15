/*
 * pl_input_output_event.h
 *
 *  Created on: 25 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_ENET_EVENT_H_
#define PL_ENET_EVENT_H_

#include "pl_config.h"
#ifdef PL_ENET_EVENT
#include "pl_time.h"
#include "pl_event.h"
#include "pl_phy.h"
#include "fsl_enet.h"

typedef struct pl_enet_event_config
{
	pl_phy_config_t*				event_phy;
	enet_ptp_timer_channel_mode_t	channel_mode;
	enet_ptp_timer_channel_t		channel;
}pl_enet_event_config_t;

typedef struct pl_enet_event
{
	pl_enet_event_config_t*		event_config;
	pl_time_t					event_time;
	pl_event_t*					event_group;
	uint32_t					signal_to_give;
	uint8_t 					enet_phy;
	uint64_t 					actual_time_us;
	uint64_t 					previous_time_us;
}pl_enet_event_t;


#if defined(PL_IMX_RT10XX)
#define pl_enet_event_init								pl_enet_event_init_imxrt10xx
#define pl_enet_event_enable							pl_enet_event_enable_imxrt10xx
#define pl_enet_event_disable							pl_enet_event_disable_imxrt10xx
#define pl_enet_event_get_event_time					pl_enet_event_get_event_time_imxrt10xx
#define pl_enet_event_get_event_time_ns					pl_enet_event_get_event_time_ns_imxrt10xx
#define pl_enet_event_get_event_time_us					pl_enet_event_get_event_time_us_imxrt10xx
#define pl_enet_event_get_pointer_to_event_time_us		pl_enet_event_get_pointer_to_event_time_us_imxrt10xx
#else
#error Platform not defined
#endif

void pl_enet_event_init(pl_enet_event_t * _pl_enet_event, pl_enet_event_config_t* _io_event_config,
						pl_event_t* _event_group, uint32_t _event_mask);

void pl_enet_event_enable(pl_enet_event_t * _pl_enet_event);

void pl_enet_event_disable(pl_enet_event_t * _pl_enet_event);

void pl_enet_event_get_event_time(pl_enet_event_t * _pl_enet_event, pl_time_t* _event_time);

uint64_t pl_enet_event_get_event_time_ns(pl_enet_event_t* _io_event);

uint64_t pl_enet_event_get_event_time_us(pl_enet_event_t* _io_event);

uint64_t* pl_enet_event_get_pointer_to_event_time_us(pl_enet_event_t* _io_event);

#endif
#endif /* PL_ENET_EVENT_H_ */
