/*
 * pl_phy.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef PL_PHY_H_
#define PL_PHY_H_

#include "pl_config.h"
#include "pl_timer.h"
#include "pl_event.h"
#ifdef PL_PHY
#include "../platform/OS/lwip/port/ethernetif.h"
#include "fsl_phy.h"
#include "lwip/netif.h"

typedef enum pl_phy_rv
{
    PL_PHY_RV_OK = 0x0U,
    PL_PHY_RV_ERROR,
	PL_PHY_RV_NO_CONF,
	PL_PHY_RV_LINK_UP,
	PL_PHY_RV_RECONNECTED,
	PL_PHY_RV_LINK_DOWN
} pl_phy_rv_t;

typedef enum pl_phy_ip
{
    PL_PHY_IP_DEFAULT = 0x0U,
    PL_PHY_IP_DHCP
} pl_phy_ip_t;

typedef struct pl_phy_config
{
	bool						initialized;
	pl_phy_ip_t					ip_obtaining;
	char*						vendor_class_id;
	uint32_t					default_local_ip_address[4];
	uint32_t					default_net_ip_address[4];
	uint32_t					default_gateway_ip_address[4];
	uint32_t					default_remote_ip_address[4];
	uint8_t						mac_address[NETIF_MAX_HWADDR_LEN];
	phy_handle_t* 				phy_handle;
	uint8_t 				    phyAddr;
    const phy_operations_t *    ops;
    void *						phy_resource;
	uint8_t						clock_name;
} pl_phy_config_t;

typedef struct pl_phy
{
	pl_phy_config_t*	phy_config;
	struct netif 		netif;
	ethernetif_config_t fsl_enet_config;
	ip4_addr_t 			netif_local_addr;
	ip4_addr_t 			netif_net_addr;
	ip4_addr_t 			netif_gw_addr;
	ip4_addr_t 			netif_remote_addr;
	uint32_t			dhcp_tries;
	bool				link_up;
	pl_event_t*			phy_link_event_group;
	uint32_t			phy_link_down;
	bool				give_link_event;
} pl_phy_t;

#if defined(PL_IMX_RT10XX)
#define pl_phy_init							pl_phy_init_imxrt10xx
#define pl_phy_assign_ip					pl_phy_assign_ip_imxrt10xx
#define pl_phy_reassign_ip					pl_phy_reassign_ip_imxrt10xx
#define pl_phy_check_link					pl_phy_check_link_imxrt10xx
#define pl_phy_check_link_state				pl_phy_check_link_state_imxrt10xx
#define pl_phy_get_object					pl_phy_get_object_imxrt10xx
#define pl_phy_get_init_phy_amount			pl_phy_get_init_phy_amount_imxrt10xx
#define pl_phy_set_link_down_event			pl_phy_set_link_down_event_imxrt10xx
#else
#error Platform not defined
#endif

pl_phy_rv_t pl_phy_init(uint8_t* _pl_phy_index, pl_phy_config_t* _pl_phy_config);

pl_phy_rv_t pl_phy_assign_ip(uint8_t _pl_phy_index);

pl_phy_rv_t pl_phy_reassign_ip(uint8_t _pl_phy_index);

pl_phy_rv_t pl_phy_check_state(uint8_t _pl_phy_index);

pl_phy_rv_t pl_phy_check_link(uint8_t _pl_phy_index);

pl_phy_rv_t pl_phy_check_link_state(uint8_t _pl_phy_index);

pl_phy_t* pl_phy_get_object(uint8_t _pl_phy_index);

uint8_t pl_phy_get_init_phy_amount(void);

void pl_phy_set_link_down_event(uint8_t _pl_phy_index, pl_event_t* _link_event_group, uint32_t _link_down_event);

#endif
#endif /* PL_PHY_H_ */
