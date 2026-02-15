/*
 * pl_config.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef PL_CONFIG_H_
#define PL_CONFIG_H_

/* portablility layer modules activation defines */
// #define PL_I2C
#define PL_THREAD
#define PL_SOCKET
#define PL_SPI
#define PL_DEBUG
#define PL_EVENT
#define PL_GPIO
#define PL_QUEUE
#define PL_TIME
#define PL_TIMER
#define PL_NVM
#define PL_ENET_EVENT
#define PL_PHY
#define PL_WATCHDOG
#define PL_NTP
#define PL_MUTEX
#define PL_MQTT
#define PL_TEMPERATURE
#define PL_SYSTEM
#define PL_JSON

/* used OS define */
 #define PL_OS_FREE_RTOS
//#define PL_LINUX
//#define PL_WINDOWS

/* Platform defines*/
 #define PL_IMX_RT10XX
// #define PL_PC
//#define PL_BEAGLEBONE

//#define PL_PTPD_TIME

/* JSON parser used */
#define PL_CJSON

#endif /* PL_CONFIG_H_ */
