/*
 * pl_ntp.c
 *
 *  Created on: Jun 29, 2023
 *      Author: abolinaga
 */

#include "pl_ntp.h"

#if defined(PL_NTP)
#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "lwip/netif.h"

pl_ntp_rv_t pl_ntp_init_freertos(pl_ntp_t * _ntp, pl_ntp_config_t* _ntp_config)
{
	pl_ntp_rv_t sntp_rv = PL_NTP_RV_ERROR;

	if(NULL!= _ntp_config)
	{
		_ntp->ntp_config = _ntp_config;

		if(PL_PHY_RV_OK == pl_phy_init(&_ntp->enet_phy, _ntp_config->ntp_phy))
		{
			pl_phy_t* phy = pl_phy_get_object(_ntp->enet_phy);

			LOCK_TCPIP_CORE();
			sntp_setoperatingmode(SNTP_OPMODE_POLL);
			sntp_setserver(0, &phy->netif_remote_addr);
			UNLOCK_TCPIP_CORE();
			sntp_rv = PL_NTP_RV_OK;
		}
	}
	else
	{
		sntp_rv = PL_NTP_RV_NO_CONF;
	}

	return sntp_rv;
}

pl_ntp_rv_t pl_ntp_sync_time_freertos(pl_ntp_t * _ntp)
{
	pl_ntp_rv_t sntp_rv = PL_NTP_RV_ERROR;

	uint32_t events;

	LOCK_TCPIP_CORE();
	sntp_init();
	UNLOCK_TCPIP_CORE();

	if(PL_EVENT_RV_OK == pl_event_timed_wait(_ntp->ntp_config->ntp_events, _ntp->ntp_config->time_updated_event, &events, SNTP_RECV_TIMEOUT))
	{
		if(_ntp->ntp_config->time_updated_event == (events & _ntp->ntp_config->time_updated_event))
		{
			sntp_rv = PL_NTP_RV_OK;
		}
		else
		{
			sntp_rv = PL_NTP_RV_NO_TIME_SYNC;
		}
	}
	else
	{
		sntp_rv = PL_NTP_RV_NO_TIME_SYNC;
	}

	LOCK_TCPIP_CORE();
	sntp_stop();
	UNLOCK_TCPIP_CORE();

	return sntp_rv;
}

#endif
