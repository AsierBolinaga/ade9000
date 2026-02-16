/*
 * absl_ntp.h
 *
 *  Created on: Jun 29, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_NTP_H_
#define ABSL_NTP_H_

#include "absl_config.h"
#ifdef ABSL_NTP

#include "absl_types.h"

#include "absl_phy.h"
#include "absl_event.h"

typedef enum absl_ntp_rv
{
	ABSL_NTP_RV_OK = 0,
	ABSL_NTP_RV_NO_CONF,
	ABSL_NTP_RV_NO_TIME_SYNC,
	ABSL_NTP_RV_ERROR
}absl_ntp_rv_t;

typedef struct absl_ntp_config
{
	absl_phy_config_t*	ntp_phy;
	absl_event_t*			ntp_events;
	uint32_t			time_updated_event;
}absl_ntp_config_t;

typedef struct absl_ntp
{
	absl_ntp_config_t*	ntp_config;
	uint8_t 			enet_phy;
}absl_ntp_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_ntp_init				absl_ntp_init_freertos
#define absl_ntp_sync_time		absl_ntp_sync_time_freertos
#else
#error Platform not defined
#endif

absl_ntp_rv_t absl_ntp_init(absl_ntp_t * _ntp, absl_ntp_config_t* _ntp_config);

absl_ntp_rv_t absl_ntp_sync_time(absl_ntp_t * _ntp);

#endif
#endif /* ABSL_NTP_H_ */
