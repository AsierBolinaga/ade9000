/*
 * pl_ntp.h
 *
 *  Created on: Jun 29, 2023
 *      Author: abolinaga
 */

#ifndef PL_NTP_H_
#define PL_NTP_H_

#include "pl_config.h"
#ifdef PL_NTP

#include "pl_types.h"

#include "pl_phy.h"
#include "pl_event.h"

typedef enum pl_ntp_rv
{
	PL_NTP_RV_OK = 0,
	PL_NTP_RV_NO_CONF,
	PL_NTP_RV_NO_TIME_SYNC,
	PL_NTP_RV_ERROR
}pl_ntp_rv_t;

typedef struct pl_ntp_config
{
	pl_phy_config_t*	ntp_phy;
	pl_event_t*			ntp_events;
	uint32_t			time_updated_event;
}pl_ntp_config_t;

typedef struct pl_ntp
{
	pl_ntp_config_t*	ntp_config;
	uint8_t 			enet_phy;
}pl_ntp_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_ntp_init				pl_ntp_init_freertos
#define pl_ntp_sync_time		pl_ntp_sync_time_freertos
#else
#error Platform not defined
#endif

pl_ntp_rv_t pl_ntp_init(pl_ntp_t * _ntp, pl_ntp_config_t* _ntp_config);

pl_ntp_rv_t pl_ntp_sync_time(pl_ntp_t * _ntp);

#endif
#endif /* PL_NTP_H_ */
