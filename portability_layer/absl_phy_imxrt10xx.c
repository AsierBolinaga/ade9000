/*
 * absl_phy_imxrt10xx.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */
#include "absl_phy.h"
#ifdef ABSL_PHY
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "lwip/prot/dhcp.h"

#include "fsl_phydp83825.h"

#include "absl_thread.h"
#include "absl_debug.h"

#define PHY_AMOUNT 	2

#define DCHP_IP_OBTAINING_MAX_RETRIES 10000

static absl_phy_t phy_imxrt10xx[PHY_AMOUNT];
static uint32_t	initialized_phy = 0;

static absl_phy_rv_t absl_phy_set_default_ip_addresses(absl_phy_t* _absl_phy, ethernetif_config_t* _fsl_enet_config);
static absl_phy_rv_t absl_phy_set_ip_dhcp_ip_addresses(absl_phy_t* _absl_phy, ethernetif_config_t* _fsl_enet_config);
static absl_phy_rv_t absl_phy_get_ip_from_dhcp_master(absl_phy_t* _absl_phy);

static void absl_phy_print_ip_addresses(absl_phy_t* _absl_phy);

void absl_phy_link_changed(struct netif *netif)
{
	absl_phy_t* phy;

	for(uint8_t phy_index = 0; phy_index < PHY_AMOUNT; phy_index++)
	{
		if(netif->ip_addr.addr == phy_imxrt10xx[phy_index].netif.ip_addr.addr)
		{
			phy = &phy_imxrt10xx[phy_index];
		}
	}
	if(!netif_is_link_up(netif))
	{
		if(phy->give_link_event)
		{
			absl_event_set_fromISR(phy->phy_link_event_group, phy->phy_link_down);
		}
		phy->link_up = false;
	}
}

absl_phy_rv_t absl_phy_init_imxrt10xx(uint8_t* _absl_phy_index, absl_phy_config_t* _absl_phy_config)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;

	if(NULL != _absl_phy_config)
	{
		phy_imxrt10xx[initialized_phy].link_up = false;
		if(false == _absl_phy_config->initialized)
		{
			phy_imxrt10xx[initialized_phy].phy_config = _absl_phy_config;

			phy_imxrt10xx[initialized_phy].fsl_enet_config.phyHandle = _absl_phy_config->phy_handle;
			phy_imxrt10xx[initialized_phy].fsl_enet_config.phyAddr = _absl_phy_config->phyAddr;
			phy_imxrt10xx[initialized_phy].fsl_enet_config.phyOps = _absl_phy_config->ops;
			phy_imxrt10xx[initialized_phy].fsl_enet_config.phyResource = _absl_phy_config->phy_resource;
			phy_imxrt10xx[initialized_phy].fsl_enet_config.srcClockHz = CLOCK_GetFreq(_absl_phy_config->clock_name);

			memcpy(&phy_imxrt10xx[initialized_phy].fsl_enet_config.macAddress, &_absl_phy_config->mac_address, 6);

			tcpip_init(NULL, NULL);

			if(_absl_phy_config->ip_obtaining == ABSL_PHY_IP_DEFAULT)
			{
				phy_rv = absl_phy_set_default_ip_addresses(&phy_imxrt10xx[initialized_phy], &phy_imxrt10xx[initialized_phy].fsl_enet_config);
			}
			else if(_absl_phy_config->ip_obtaining == ABSL_PHY_IP_DHCP)
			{
				phy_rv = absl_phy_set_ip_dhcp_ip_addresses(&phy_imxrt10xx[initialized_phy], &phy_imxrt10xx[initialized_phy].fsl_enet_config);
				if((ABSL_PHY_RV_OK != phy_rv) && (ABSL_PHY_RV_LINK_DOWN != phy_rv))
				{
					absl_debug_printf("Default IP addresses will be assigned!\n");
					phy_rv = absl_phy_set_default_ip_addresses(&phy_imxrt10xx[initialized_phy], &phy_imxrt10xx[initialized_phy].fsl_enet_config);
				}
			}
			else
			{

			}

			if(ABSL_PHY_RV_ERROR != phy_rv)
			{
				if(ABSL_PHY_RV_LINK_DOWN == phy_rv)
				{
					phy_imxrt10xx[initialized_phy].link_up = false;
				}
				else
				{
					phy_imxrt10xx[initialized_phy].link_up = true;
				}

				*_absl_phy_index = initialized_phy;
				initialized_phy++;
				_absl_phy_config->initialized = true;
			}
		}
		else
		{
			for(uint8_t phy_index = 0; phy_index < PHY_AMOUNT; phy_index++)
			{
				if(phy_imxrt10xx[phy_index].phy_config == _absl_phy_config)
				{
					*_absl_phy_index = phy_index;
				}
			}
			phy_rv = ABSL_PHY_RV_OK;
		}
	}
	else
	{
		phy_rv = ABSL_PHY_RV_NO_CONF;
	}

	return phy_rv;
}

absl_phy_rv_t absl_phy_assign_ip_imxrt10xx(uint8_t _absl_phy_index)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;

	if(ABSL_PHY_IP_DHCP == phy_imxrt10xx[_absl_phy_index].phy_config->ip_obtaining)
	{
		LOCK_TCPIP_CORE();
		netif_set_link_callback(&phy_imxrt10xx[_absl_phy_index].netif, absl_phy_link_changed);
		UNLOCK_TCPIP_CORE();

		dhcp_set_vendor_class_identifier(strlen(phy_imxrt10xx[_absl_phy_index].phy_config->vendor_class_id),
										phy_imxrt10xx[_absl_phy_index].phy_config->vendor_class_id);

		netifapi_dhcp_start(&phy_imxrt10xx[_absl_phy_index].netif);

		phy_rv = absl_phy_get_ip_from_dhcp_master(&phy_imxrt10xx[_absl_phy_index]);
	}
	else
	{
		absl_phy_print_ip_addresses(&phy_imxrt10xx[_absl_phy_index]);
		phy_rv = ABSL_PHY_RV_OK;
	}

	return phy_rv;
}

absl_phy_rv_t absl_phy_reassign_ip_imxrt10xx(uint8_t _absl_phy_index)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;

	if(ABSL_PHY_IP_DHCP == phy_imxrt10xx[_absl_phy_index].phy_config->ip_obtaining)
	{
		netifapi_dhcp_renew(&phy_imxrt10xx[_absl_phy_index].netif);

		phy_rv = absl_phy_get_ip_from_dhcp_master(&phy_imxrt10xx[_absl_phy_index]);
	}
	else
	{
		absl_phy_print_ip_addresses(&phy_imxrt10xx[_absl_phy_index]);
		phy_rv = ABSL_PHY_RV_OK;
	}

	return phy_rv;
}

absl_phy_rv_t absl_phy_check_link_imxrt10xx(uint8_t _absl_phy_index)
{ 
	if (ethernetif_wait_linkup(&phy_imxrt10xx[_absl_phy_index].netif, 5000) != ERR_OK)
	{
		PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
		phy_imxrt10xx[_absl_phy_index].link_up = false;
		return ABSL_PHY_RV_LINK_DOWN;
	}
	else
	{
		phy_speed_t speed;
		phy_duplex_t duplex;

		phy_imxrt10xx[_absl_phy_index].phy_config->phy_handle->ops->getLinkSpeedDuplex(phy_imxrt10xx[_absl_phy_index].phy_config->phy_handle, &speed, &duplex);

		switch(speed)
		{
		case kPHY_Speed10M:
			absl_debug_printf("Configured to 10M.\r\n\n");
			break;
		case kPHY_Speed100M:
			absl_debug_printf("Configured to 100M.\r\n\n");
			break;
		case kPHY_Speed1000M:
			absl_debug_printf("Configured to 1000M.\r\n\n");
			break;
		default:
			absl_debug_printf("\r\n\n");
			break;
		}
		phy_imxrt10xx[_absl_phy_index].link_up = true;
		return ABSL_PHY_RV_LINK_UP;
	}
}

absl_phy_rv_t absl_phy_check_link_state_imxrt10xx(uint8_t _absl_phy_index)
{ 
	if(true == phy_imxrt10xx[_absl_phy_index].link_up)
	{
		return ABSL_PHY_RV_LINK_UP;
	}
	else
	{
		return ABSL_PHY_RV_LINK_DOWN;
	}
}

absl_phy_t* absl_phy_get_object_imxrt10xx(uint8_t _absl_phy_index)
{
	return &phy_imxrt10xx[_absl_phy_index];
}

uint8_t absl_phy_get_init_phy_amount_imxrt10xx(void)
{
	return initialized_phy;
}

void absl_phy_set_link_down_event_imxrt10xx(uint8_t _absl_phy_index, absl_event_t* _link_event_group, uint32_t _link_down_event)
{
	phy_imxrt10xx[_absl_phy_index].phy_link_event_group = _link_event_group;
	phy_imxrt10xx[_absl_phy_index].phy_link_down 		  = _link_down_event;
	phy_imxrt10xx[_absl_phy_index].give_link_event 	  = true;
}


static absl_phy_rv_t absl_phy_set_default_ip_addresses(absl_phy_t* _absl_phy, ethernetif_config_t* _fsl_enet_config)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;
	err_t err;

	IP4_ADDR(&_absl_phy->netif_local_addr, _absl_phy->phy_config->default_local_ip_address[0],
										 _absl_phy->phy_config->default_local_ip_address[1],
										 _absl_phy->phy_config->default_local_ip_address[2],
										 _absl_phy->phy_config->default_local_ip_address[3]);
	IP4_ADDR(&_absl_phy->netif_net_addr, _absl_phy->phy_config->default_net_ip_address[0],
									   _absl_phy->phy_config->default_net_ip_address[1],
									   _absl_phy->phy_config->default_net_ip_address[2],
									   _absl_phy->phy_config->default_net_ip_address[3]);
	IP4_ADDR(&_absl_phy->netif_gw_addr, _absl_phy->phy_config->default_gateway_ip_address[0],
									  _absl_phy->phy_config->default_gateway_ip_address[1],
									  _absl_phy->phy_config->default_gateway_ip_address[2],
									  _absl_phy->phy_config->default_gateway_ip_address[3]);
	IP4_ADDR(&_absl_phy->netif_remote_addr, _absl_phy->phy_config->default_remote_ip_address[0],
										  _absl_phy->phy_config->default_remote_ip_address[1],
										  _absl_phy->phy_config->default_remote_ip_address[2],
										  _absl_phy->phy_config->default_remote_ip_address[3]);

	err = netifapi_netif_add(&_absl_phy->netif, &_absl_phy->netif_local_addr, &_absl_phy->netif_net_addr, &_absl_phy->netif_gw_addr,
			  _fsl_enet_config, ethernetif0_init, tcpip_input);
	if(!err)
	{
		err = netifapi_netif_set_default(&_absl_phy->netif);
		if(!err)
		{
			err = netifapi_netif_set_up(&_absl_phy->netif);
			if(!err)
			{
				if (ethernetif_wait_linkup(&_absl_phy->netif, 5000) != ERR_OK)
				{
					absl_debug_printf("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");

					phy_rv = ABSL_PHY_RV_LINK_DOWN;
				}
				else
				{
					phy_speed_t speed;
					phy_duplex_t duplex;

					absl_debug_printf("PHY Link UP. ");
					_absl_phy->phy_config->phy_handle->ops->getLinkSpeedDuplex(_absl_phy->phy_config->phy_handle, &speed, &duplex);

					switch(speed)
					{
					case kPHY_Speed10M:
						absl_debug_printf("Configured to 10M.\r\n\n");
						break;
					case kPHY_Speed100M:
						absl_debug_printf("Configured to 100M.\r\n\n");
						break;
					case kPHY_Speed1000M:
						absl_debug_printf("Configured to 1000M.\r\n\n");
						break;
					default:
						absl_debug_printf("\r\n\n");
						break;
					}
					absl_phy_print_ip_addresses(_absl_phy);
					phy_rv = ABSL_PHY_RV_OK;
				}
			}
		}
	}

	return phy_rv;
}

static absl_phy_rv_t absl_phy_set_ip_dhcp_ip_addresses(absl_phy_t* _absl_phy, ethernetif_config_t* _fsl_enet_config)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;
	err_t err;

	err = netifapi_netif_add(&_absl_phy->netif, NULL, NULL, NULL, _fsl_enet_config, ethernetif0_init, tcpip_input);
	if(!err)
	{
		err = netifapi_netif_set_default(&_absl_phy->netif);
		if(!err)
		{
			err = netifapi_netif_set_up(&_absl_phy->netif);
			if(!err)
			{
				if (ethernetif_wait_linkup(&_absl_phy->netif, 5000) != ERR_OK)
				{
					absl_debug_printf("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");

					phy_rv = ABSL_PHY_RV_LINK_DOWN;
				}
				else
				{
					phy_speed_t speed;
					phy_duplex_t duplex;

					absl_debug_printf("PHY Link UP. ");
					PHY_DP83825_GetLinkSpeedDuplex(_absl_phy->phy_config->phy_handle, &speed, &duplex);

					switch(speed)
					{
					case kPHY_Speed10M:
						absl_debug_printf("Configured to 10M.\r\n\n");
						break;
					case kPHY_Speed100M:
						absl_debug_printf("Configured to 100M.\r\n\n");
						break;
					case kPHY_Speed1000M:
						absl_debug_printf("Configured to 1000M.\r\n\n");
						break;
					default:
						absl_debug_printf("\r\n\n");
						break;
					}
					phy_rv= absl_phy_assign_ip(initialized_phy);
				}
			}
		}
	}

	return phy_rv;
}

static absl_phy_rv_t absl_phy_get_ip_from_dhcp_master(absl_phy_t* _absl_phy)
{
	absl_phy_rv_t phy_rv = ABSL_PHY_RV_ERROR;

	struct dhcp*	dhcp;
	u8_t 			dhcp_last_state = DHCP_STATE_OFF;
	bool			process_done = false;

	while (netif_is_up(&_absl_phy->netif) && !process_done)
	{
		dhcp = netif_dhcp_data(&_absl_phy->netif);

		if (dhcp == NULL)
		{
			dhcp_last_state = DHCP_STATE_OFF;
		}
		else if (dhcp_last_state != dhcp->state)
		{
			dhcp_last_state = dhcp->state;

			absl_debug_printf(" DHCP state       : ");
			switch (dhcp_last_state)
			{
				case DHCP_STATE_OFF:
					absl_debug_printf("OFF");
					break;
				case DHCP_STATE_REQUESTING:
					absl_debug_printf("REQUESTING");
					break;
				case DHCP_STATE_INIT:
					absl_debug_printf("INIT");
					break;
				case DHCP_STATE_REBOOTING:
					absl_debug_printf("REBOOTING");
					break;
				case DHCP_STATE_REBINDING:
					absl_debug_printf("REBINDING");
					break;
				case DHCP_STATE_RENEWING:
					absl_debug_printf("RENEWING");
					break;
				case DHCP_STATE_SELECTING:
					absl_debug_printf("SELECTING");
					break;
				case DHCP_STATE_INFORMING:
					absl_debug_printf("INFORMING");
					break;
				case DHCP_STATE_CHECKING:
					absl_debug_printf("CHECKING");
					break;
				case DHCP_STATE_BOUND:
					absl_debug_printf("BOUND");
					break;
				case DHCP_STATE_BACKING_OFF:
					absl_debug_printf("BACKING_OFF");
					break;
				default:
					absl_debug_printf("%u", dhcp_last_state);
					assert(0);
					break;
			}
			absl_debug_printf("\r\n");

			if (dhcp_last_state == DHCP_STATE_BOUND)
			{
				_absl_phy->netif_local_addr = dhcp->offered_ip_addr;
				_absl_phy->netif_net_addr = dhcp->offered_sn_mask;
				_absl_phy->netif_gw_addr = dhcp->offered_gw_addr;

				_absl_phy->netif_remote_addr = dhcp->server_ip_addr;

				absl_phy_print_ip_addresses(_absl_phy);
				process_done = true;
				phy_rv = ABSL_PHY_RV_OK;
			}
		}

		absl_thread_sleep(20U);
	}

	if(phy_rv != ABSL_PHY_RV_OK)
	{
		absl_debug_printf("IP not assigned, no DHCP server!\n");
		netifapi_dhcp_release_and_stop(&_absl_phy->netif);
		netifapi_netif_set_link_down(&_absl_phy->netif);
		netifapi_netif_remove(&_absl_phy->netif);
	}

	return phy_rv;
}

static void absl_phy_print_ip_addresses(absl_phy_t* _absl_phy)
{
	absl_debug_printf("\r\n IPv4 Address     : %s\r\n", ipaddr_ntoa(&_absl_phy->netif_local_addr));
	absl_debug_printf(" IPv4 Subnet mask : %s\r\n", ipaddr_ntoa(&_absl_phy->netif_net_addr));
	absl_debug_printf(" IPv4 Gateway     : %s\r\n\r\n", ipaddr_ntoa(&_absl_phy->netif_gw_addr));
	absl_debug_printf(" IPv4 Server     : %s\r\n\r\n", ipaddr_ntoa(&_absl_phy->netif_remote_addr));
}

#endif
