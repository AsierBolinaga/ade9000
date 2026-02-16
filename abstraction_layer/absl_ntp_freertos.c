/*
 * absl_ntp.c
 *
 *  Created on: Jun 29, 2023
 *      Author: abolinaga
 */

#include "absl_ntp.h"

#if defined(ABSL_NTP)
#include "absl_macros.h"

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"

static void absl_ntp_sync_start_freertos(void *arg);
static void absl_ntp_sync_stop_freertos(void *arg);

absl_ntp_rv_t absl_ntp_init_freertos(absl_ntp_t * _ntp, absl_ntp_config_t* _ntp_config)
{
	absl_ntp_rv_t sntp_rv = ABSL_NTP_RV_ERROR;

	if(NULL!= _ntp_config)
	{
		_ntp->ntp_config = _ntp_config;

		if(ABSL_PHY_RV_OK == absl_phy_init(&_ntp->enet_phy, _ntp_config->ntp_phy))
		{
			absl_phy_t* phy = absl_phy_get_object(_ntp->enet_phy);

			LOCK_TCPIP_CORE();
			sntp_setoperatingmode(SNTP_OPMODE_POLL);
			sntp_setserver(0, &phy->netif_remote_addr);
			UNLOCK_TCPIP_CORE();
			sntp_rv = ABSL_NTP_RV_OK;
		}
	}
	else
	{
		sntp_rv = ABSL_NTP_RV_NO_CONF;
	}

	return sntp_rv;
}

absl_ntp_rv_t absl_ntp_sync_time_freertos(absl_ntp_t * _ntp)
{
	absl_ntp_rv_t sntp_rv = ABSL_NTP_RV_ERROR;

	uint32_t events;

	tcpip_callback(absl_ntp_sync_start_freertos, NULL);

	if(ABSL_EVENT_RV_OK == absl_event_timed_wait(_ntp->ntp_config->ntp_events, _ntp->ntp_config->time_updated_event, &events, SNTP_RECV_TIMEOUT))
	{
		if(_ntp->ntp_config->time_updated_event == (events & _ntp->ntp_config->time_updated_event))
		{
			sntp_rv = ABSL_NTP_RV_OK;
		}
		else
		{
			sntp_rv = ABSL_NTP_RV_NO_TIME_SYNC;
		}
	}
	else
	{
		sntp_rv = ABSL_NTP_RV_NO_TIME_SYNC;
	}

	tcpip_callback(absl_ntp_sync_stop_freertos, NULL);

	return sntp_rv;
}

static void absl_ntp_sync_start_freertos(void *arg)
{
    ABSL_UNUSED_ARG(arg);
    sntp_init();
}

static void absl_ntp_sync_stop_freertos(void *arg)
{
	ABSL_UNUSED_ARG(arg);
	sntp_stop();
}

#endif
