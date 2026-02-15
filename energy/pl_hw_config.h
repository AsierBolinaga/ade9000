/*
 * pl_hw_config.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef PL_HW_CONFIG_H_
#define PL_HW_CONFIG_H_

#include "pl_spi.h"
#include "pl_enet_event.h"
#include "pl_phy.h"
#include "pl_socket.h"
#include "pl_mqtt.h"
#include "pl_nvm.h"
#include "pl_gpio.h"
#include "pl_watchdog.h"
#include "pl_ntp.h"

typedef enum spi
{
    PL_SPI_ENERGY 	= 0U,
	PL_SPI_MAXVALUE
} spi_t;

typedef enum event
{
    PL_IRQ0_ENET_EVENT 	= 0U,
	PL_ENET_EVENT_MAXVALUE
} event_t;

typedef enum socket
{
	PL_SOCKET_STREAM_FAST = 0U,
	PL_SOCKET_STREAM_SLOW,
	PL_SOCKET_MAXVALUE
} socket_t;

typedef enum mqtt
{
	PL_MQTT_COMMANDS= 0U,
	PL_MQTT_MAXVALUE
} mqtt_t;

typedef enum nvm_sections
{
	NVM_SECTION_PRIMARY = 0,
	NVM_SECTION_SECONDARY,
	NVM_SECTION_FACTORY_CONF,
	NVM_SECTION_MAXVALUE
}nvm_sections_t;

typedef enum nvm
{
	PL_NVM_CONFIG = 0,
	PL_NVM_MAXVALUE
}nvm_t;

typedef enum gpio
{
	PL_GPIO_ADE9000_RESET = 0,
	PL_GPIO_LED_USER,
	PL_GPIO_LED_USER_ENABLE,
#ifdef DEBUG_PIN
	PL_GPIO_DEBUG,
#endif
	PL_GPIO_MAXVALUE
}gpio_t;

typedef enum wdog
{
	PL_WDOG1 = 0,
	PL_WDOG_MAXVALUE
}wdog_t;

typedef enum ntp
{
	PL_NTP_SYNC = 0,
	PL_NTP_MAXVALUE
}ntp_t;

pl_spi_config_t*    	pl_config_get_spi_conf(spi_t _spi);

void pl_hw_config_assign_phy_mac(uint8_t* _mac_address);

pl_enet_event_config_t* pl_config_get_enet_event_conf(event_t _enet_event);

pl_socket_config_t* 	pl_config_get_socket_conf(socket_t _socket);

pl_mqtt_config_t*		pl_config_get_mqtt_conf(mqtt_t _mqtt);

pl_nvm_config_t*		pl_config_get_nvm_conf(nvm_t _nmv);

pl_gpio_config_t*		pl_config_get_gpio_conf(gpio_t _gpio);

pl_watchdog_config_t*	pl_config_get_wdog_conf(wdog_t _wdog);

pl_ntp_config_t*		pl_config_get_ntp_conf(ntp_t _ntp);

#endif /* PL_HW_CONFIG_H_ */
