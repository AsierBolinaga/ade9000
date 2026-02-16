/*
 * absl_config.h
 *
 *  Created on: Mar 22, 2023
 *      Author: abolinaga
 */

#ifndef ABSL_CONFIG_H_
#define ABSL_CONFIG_H_

/* portablility layer modules activation defines */
// #define ABSL_I2C
#define ABSL_THREAD
#define ABSL_SOCKET
#define ABSL_SPI
#define ABSL_DEBUG
#define ABSL_EVENT
#define ABSL_GPIO
#define ABSL_QUEUE
#define ABSL_TIME
#define ABSL_TIMER
#define ABSL_NVM
#define ABSL_ENET_EVENT
#define ABSL_PHY
#define ABSL_WATCHDOG
#define ABSL_NTP
#define ABSL_MUTEX
#define ABSL_MQTT
#define ABSL_TEMPERATURE
#define ABSL_SYSTEM
#define ABSL_JSON

/* used OS define */
 #define ABSL_OS_FREE_RTOS
//#define ABSL_LINUX
//#define ABSL_WINDOWS

/* Platform defines*/
 #define ABSL_IMX_RT10XX
// #define ABSL_PC
//#define ABSL_BEAGLEBONE

//#define ABSL_PTPD_TIME

/* JSON parser used */
#define ABSL_CJSON

#endif /* ABSL_CONFIG_H_ */
