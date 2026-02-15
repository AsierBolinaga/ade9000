/*
 * pl_hw_config.c
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */
#include "pl_hw_config.h"
#include "pl_macros.h"

#include "board.h"

#include "fsl_phy.h"
#if defined(ADSN4)
#include "fsl_phydp83825.h"
#elif defined(MIMXRT1060_EVKB)
#include "fsl_phyksz8081.h"
#else
#error ERROR! Valid platform not defined
#endif

#include "mflash_drv.h"
#include "flash_info.h"
#include "fw_update.h"

/*************************************************************************/
/*							SPI CONFIGURATION       					 */
/*************************************************************************/

#define TRANSFER_BAUDRATE (10000000U)

static ALLOC_DATA_SEC pl_spi_config_t  spi_ade9000_config =
{
#if defined(ADSN4)
		LPSPI4,
#elif defined(MIMXRT1060_EVKB)
		LPSPI1,
#else
#error ERROR! Valid platform not defined
#endif
		DMA0,
		PL_SPI_MODE_MASTER,
		TRANSFER_BAUDRATE,
		500
};

static ALLOC_DATA_SEC pl_spi_config_t* 	spi_configs[PL_SPI_MAXVALUE] =
{
	&spi_ade9000_config
};

pl_spi_config_t* pl_config_get_spi_conf(spi_t _spi)
{
	return spi_configs[_spi];
}

/*************************************************************************/
/*							PHY CONFIGURATION       					 */
/*************************************************************************/
#if defined(ADSN4)
#define PHY_ADDR	0

extern  phy_dp83825_resource_t g_phy_resource;
static  ALLOC_DATA_SEC phy_handle_t phyHandle;

static  ALLOC_DATA_SEC pl_phy_config_t energy_phy=
{
		false,
		PL_PHY_IP_DHCP,
		"aingura",
		{192, 168, 2, 102},
		{255, 255, 255, 0},
		{192, 168, 2, 72},
		{192, 168, 2, 15},
		{0, 0, 0, 0, 0, 0},
		&phyHandle,
		PHY_ADDR,
		&phydp83825_ops,
		&g_phy_resource,
		kCLOCK_IpgClk,
};
#elif defined(MIMXRT1060_EVKB)
extern phy_ksz8081_resource_t g_phy_resource;
static ALLOC_DATA_SEC phy_handle_t phyHandle;

static pl_phy_config_t energy_phy =
{
		false,
		PL_PHY_IP_DHCP,
		"aingura",
		{192, 168, 2, 101},
		{255, 255, 255, 0},
		{192, 168, 2, 72},
		{192, 168, 2, 1},
		{0, 0, 0, 0, 0, 0},
		&phyHandle,
		BOARD_ENET0_PHY_ADDRESS,
		&phyksz8081_ops,
		&g_phy_resource,
		kCLOCK_IpgClk,
};
#else
#error ERROR! Valid platform not defined
#endif

void pl_hw_config_assign_phy_mac(uint8_t* _mac_address)
{
	memcpy(&energy_phy.mac_address, _mac_address, NETIF_MAX_HWADDR_LEN);
}

/*************************************************************************/
/*					ENET EVENTS CONFIGURATION       					 */
/*************************************************************************/

static  ALLOC_DATA_SEC pl_enet_event_config_t ade9000_irq_event =
{
		&energy_phy,
		kENET_PtpChannelFallingCapture,
#if defined(ADSN4)
		kENET_PtpTimerChannel3
#elif defined(MIMXRT1060_EVKB)
		kENET_PtpTimerChannel1
#else
#error ERROR! Valid platform not defined
#endif
};


static pl_enet_event_config_t* 	enet_event_configs[PL_ENET_EVENT_MAXVALUE] =
{
	&ade9000_irq_event
};


pl_enet_event_config_t* pl_config_get_enet_event_conf(event_t _enet_event)
{
	return enet_event_configs[_enet_event];
}

/*************************************************************************/
/*						SOCKET CONFIGURATION       						 */
/*************************************************************************/
#define	RECEPTION_MAX_LENGTH 	1000

static ALLOC_DATA_SEC uint8_t comm_fast_rx_buffer[RECEPTION_MAX_LENGTH];
static ALLOC_DATA_SEC uint8_t comm_slow_rx_buffer[RECEPTION_MAX_LENGTH];

static ALLOC_DATA_SEC pl_socket_config_t comm_socket_fast_config =
{
		&energy_phy,
		PL_SOCKET_PROTOCOL_UDP_TCP,
		PL_SOCKET_MODE_CLIENT,
		44983,
		comm_fast_rx_buffer
};

static ALLOC_DATA_SEC pl_socket_config_t comm_socket_slow_config =
{
		&energy_phy,
		PL_SOCKET_PROTOCOL_UDP_TCP,
		PL_SOCKET_MODE_CLIENT,
		44984,
		comm_slow_rx_buffer
};

static pl_socket_config_t* 	socket_configs[PL_SOCKET_MAXVALUE] =
{
	&comm_socket_fast_config,
	&comm_socket_slow_config
};

pl_socket_config_t* pl_config_get_socket_conf(socket_t _socket)
{
	return socket_configs[_socket];
}


/*************************************************************************/
/*						MQTT CONFIGURATION       						 */
/*************************************************************************/
#define	MQTT_RECEPTION_MAX_LENGTH 	2000

static ALLOC_DATA_SEC uint8_t mqtt_rx_buffer[MQTT_RECEPTION_MAX_LENGTH];

static ALLOC_DATA_SEC pl_mqtt_config_t cmd_mqtt_config =
{
	&energy_phy,
	1883,
	MQTT_RECEPTION_MAX_LENGTH,
	mqtt_rx_buffer,
	"aidsapi",
	"Ja6HNecn55UnskOmd3oH",
	10
};

static pl_mqtt_config_t* 	mqtt_configs[PL_MQTT_MAXVALUE] =
{
	&cmd_mqtt_config
};

pl_mqtt_config_t* pl_config_get_mqtt_conf(mqtt_t _mqtt)
{
	return mqtt_configs[_mqtt];
}

/*************************************************************************/
/*			NON VOLATILE MEMORY CONFIGURATION       					 */
/*************************************************************************/

pl_nvm_sector_t nvm_sectors[NVM_SECTION_MAXVALUE] =
{
	{FLASH_AREA_IMAGE_1_OFFSET, 		FLASH_AREA_IMAGE_1_SIZE},
	{FLASH_AREA_IMAGE_2_OFFSET, 		FLASH_AREA_IMAGE_2_SIZE},
	{FLASH_AREA_FACTORY_CONF_OFFSET, 	FLASH_AREA_FACTORY_CONF_SIZE}
};

static pl_nvm_config_t nvm_qspi_config =
{
	FLASH_DEVICE_BASE_ADDR,
	FLASH_AREA_IMAGE_SECTOR_SIZE,
	MFLASH_PAGE_SIZE,
	NVM_SECTION_MAXVALUE,
	nvm_sectors,
	false
};

static pl_nvm_config_t* nvm_configs[PL_NVM_MAXVALUE] =
{
	&nvm_qspi_config
};

pl_nvm_config_t*	pl_config_get_nvm_conf(nvm_t _nmv)
{
	return nvm_configs[_nmv];
}

/*************************************************************************/
/*			               GPIO CONFIGURATION       					 */
/*************************************************************************/

static pl_gpio_config_t gpio_ade9000_reset_config =
{
	GPIO1,
#if defined(ADSN4)
	25,
#elif defined(MIMXRT1060_EVKB)
	3,
#else
#error ERROR! Valid platform not defined
#endif
	kGPIO_DigitalOutput,
	0
};

static pl_gpio_config_t gpio_status_led_user_config =
{
	GPIO1,
	21,
	kGPIO_DigitalOutput,
	0
};


static pl_gpio_config_t gpio_status_led_user_enable_config =
{
	GPIO1,
	20,
	kGPIO_DigitalOutput,
	0
};

#ifdef DEBUG_PIN
static pl_gpio_config_t gpio_debug_pin_config =
{
	GPIO1,
	28,
	kGPIO_DigitalOutput,
	0
};
#endif

static pl_gpio_config_t* gpio_configs[PL_GPIO_MAXVALUE] =
{
	&gpio_ade9000_reset_config,
	&gpio_status_led_user_config,
	&gpio_status_led_user_enable_config,
#ifdef DEBUG_PIN
	&gpio_debug_pin_config
#endif
};

pl_gpio_config_t*	pl_config_get_gpio_conf(gpio_t _gpio)
{
	return gpio_configs[_gpio];
}

/*************************************************************************/
/*			               WDOG CONFIGURATION       					 */
/*************************************************************************/
static pl_watchdog_config_t wdog_config =
{
	WDOG1,
	20,
	8
};

static pl_watchdog_config_t* wdog_configs[PL_WDOG_MAXVALUE] =
{
	&wdog_config
};

pl_watchdog_config_t*	pl_config_get_wdog_conf(wdog_t _wdog)
{
	return wdog_configs[_wdog];
}

/*************************************************************************/
/*			               NTP CONFIGURATION       					 */
/*************************************************************************/

extern pl_event_t enet_time_events;

static ALLOC_DATA_SEC pl_ntp_config_t ntp_syn_config =
{
		&energy_phy,
		&enet_time_events,
		ETHERNETIF_TIME_UPDATED
};

static pl_ntp_config_t* ntp_configs[PL_NTP_MAXVALUE] =
{
	&ntp_syn_config
};


pl_ntp_config_t*	pl_config_get_ntp_conf(ntp_t _ntp)
{
	return ntp_configs[_ntp];
}
