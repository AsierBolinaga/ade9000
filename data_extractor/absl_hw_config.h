/*
 * absl_hw_config.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_HW_CONFIG_H_
#define ABSL_HW_CONFIG_H_

#include "absl_spi.h"
#include "absl_enet_event.h"
#include "absl_phy.h"
#include "absl_socket.h"
#include "absl_mqtt.h"
#include "absl_nvm.h"
#include "absl_gpio.h"
#include "absl_watchdog.h"
#include "absl_ntp.h"

typedef enum spi
{
    ABSL_SPI_ENERGY 	= 0U,
	ABSL_SPI_MAXVALUE
} spi_t;

typedef enum event
{
    ABSL_IRQ0_ENET_EVENT 	= 0U,
	ABSL_ENET_EVENT_MAXVALUE
} event_t;

typedef enum socket
{
	ABSL_SOCKET_STREAM_FAST = 0U,
	ABSL_SOCKET_STREAM_SLOW,
	ABSL_SOCKET_MAXVALUE
} socket_t;

typedef enum mqtt
{
	ABSL_MQTT_COMMANDS= 0U,
	ABSL_MQTT_MAXVALUE
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
	ABSL_NVM_CONFIG = 0,
	ABSL_NVM_MAXVALUE
}nvm_t;

typedef enum gpio
{
	ABSL_GPIO_ADE9000_RESET = 0,
	ABSL_GPIO_LED_USER,
	ABSL_GPIO_LED_USER_ENABLE,
#ifdef DEBUG_PIN
	ABSL_GPIO_DEBUG,
#endif
	ABSL_GPIO_MAXVALUE
}gpio_t;

typedef enum wdog
{
	ABSL_WDOG1 = 0,
	ABSL_WDOG_MAXVALUE
}wdog_t;

typedef enum ntp
{
	ABSL_NTP_SYNC = 0,
	ABSL_NTP_MAXVALUE
}ntp_t;

absl_spi_config_t*    	absl_config_get_spi_conf(spi_t _spi);

void absl_hw_config_assign_phy_mac(uint8_t* _mac_address);

absl_enet_event_config_t* absl_config_get_enet_event_conf(event_t _enet_event);

absl_socket_config_t* 	absl_config_get_socket_conf(socket_t _socket);

absl_mqtt_config_t*		absl_config_get_mqtt_conf(mqtt_t _mqtt);

absl_nvm_config_t*		absl_config_get_nvm_conf(nvm_t _nmv);

absl_gpio_config_t*		absl_config_get_gpio_conf(gpio_t _gpio);

absl_watchdog_config_t*	absl_config_get_wdog_conf(wdog_t _wdog);

absl_ntp_config_t*		absl_config_get_ntp_conf(ntp_t _ntp);

#endif /* ABSL_HW_CONFIG_H_ */
