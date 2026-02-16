/*
 * absl_phy.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_PHY_H_
#define ABSL_PHY_H_

#include "absl_config.h"
#include "absl_timer.h"
#include "absl_event.h"
#ifdef ABSL_PHY
#include "ethernetif.h"
#include "fsl_phy.h"
#include "lwip/netif.h"

typedef enum absl_phy_rv
{
    ABSL_PHY_RV_OK = 0x0U,
    ABSL_PHY_RV_ERROR,
	ABSL_PHY_RV_NO_CONF,
	ABSL_PHY_RV_LINK_UP,
	ABSL_PHY_RV_RECONNECTED,
	ABSL_PHY_RV_LINK_DOWN
} absl_phy_rv_t;

typedef enum absl_phy_ip
{
    ABSL_PHY_IP_DEFAULT = 0x0U,
    ABSL_PHY_IP_DHCP
} absl_phy_ip_t;

typedef struct absl_phy_config
{
	bool						initialized;
	absl_phy_ip_t					ip_obtaining;
	char*						vendor_class_id;
	uint32_t					default_local_ip_address[4];
	uint32_t					default_subnet_mask[4];
	uint32_t					default_gateway_ip_address[4];
	uint32_t					default_remote_ip_address[4];
	uint8_t						mac_address[NETIF_MAX_HWADDR_LEN];
	phy_handle_t* 				phy_handle;
	uint8_t 				    phyAddr;
    const phy_operations_t *    ops;
    void *						phy_resource;
	uint8_t						clock_name;
} absl_phy_config_t;

typedef struct absl_phy
{
	absl_phy_config_t*	phy_config;
	struct netif 		netif;
	ethernetif_config_t fsl_enet_config;
	ip4_addr_t 			netif_local_addr;
	ip4_addr_t 			netif_subnet_mask;
	ip4_addr_t 			netif_gw_addr;
	ip4_addr_t 			netif_remote_addr;
	uint32_t			dhcp_tries;
	bool				link_up;
	absl_event_t*			phy_link_event_group;
	uint32_t			phy_link_down;
	bool				give_link_event;
} absl_phy_t;

#if defined(ABSL_IMX_RT10XX)
#define absl_phy_init							absl_phy_init_imxrt10xx
#define absl_phy_assign_ip					absl_phy_assign_ip_imxrt10xx
#define absl_phy_reassign_ip					absl_phy_reassign_ip_imxrt10xx
#define absl_phy_check_link					absl_phy_check_link_imxrt10xx
#define absl_phy_check_link_state				absl_phy_check_link_state_imxrt10xx
#define absl_phy_get_object					absl_phy_get_object_imxrt10xx
#define absl_phy_get_init_phy_amount			absl_phy_get_init_phy_amount_imxrt10xx
#define absl_phy_set_link_down_event			absl_phy_set_link_down_event_imxrt10xx
#else
#error Platform not defined
#endif

absl_phy_rv_t absl_phy_init(uint8_t* _absl_phy_index, absl_phy_config_t* _absl_phy_config);

absl_phy_rv_t absl_phy_assign_ip(uint8_t _absl_phy_index);

absl_phy_rv_t absl_phy_reassign_ip(uint8_t _absl_phy_index);

absl_phy_rv_t absl_phy_check_state(uint8_t _absl_phy_index);

absl_phy_rv_t absl_phy_check_link(uint8_t _absl_phy_index);

absl_phy_rv_t absl_phy_check_link_state(uint8_t _absl_phy_index);

absl_phy_t* absl_phy_get_object(uint8_t _absl_phy_index);

uint8_t absl_phy_get_init_phy_amount(void);

void absl_phy_set_link_down_event(uint8_t _absl_phy_index, absl_event_t* _link_event_group, uint32_t _link_down_event);

#endif
#endif /* ABSL_PHY_H_ */
