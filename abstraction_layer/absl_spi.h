/*
 * absl_spi.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_SPI_H_
#define ABSL_SPI_H_

#include "absl_config.h"
#ifdef ABSL_SPI

#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#elif defined(ABSL_LINUX)
#include <linux/spi/spidev.h>
#endif

#include "absl_mutex.h"
#include "absl_event.h"

typedef enum absl_spi_rv
{
    ABSL_SPI_RV_OK = 0x0U,
    ABSL_SPI_RV_ERROR,
	ABSL_SPI_RV_NO_CONF
} absl_spi_rv_t;

typedef enum absl_spi_mode
{
    ABSL_SPI_MODE_MASTER = 0x0U,
    ABSL_SPI_MODE_SLAVE
} absl_spi_mode_t;


typedef struct absl_spi_config
{
	LPSPI_Type *			spi_base;
	DMA_Type *				dma_base;
	absl_spi_mode_t			spi_mode;
	uint32_t 				baud_rate;
	uint32_t				transfer_timeout_ms;
} absl_spi_config_t;

typedef struct absl_spi
{
	absl_mutex_t				spi_mutex;
#if defined(ABSL_OS_FREE_RTOS)
	lpspi_rtos_handle_t  	spi_handle;
	lpspi_transfer_t 	 	spi_transfer;
#elif defined(ABSL_LINUX)
    int 					spi_handle;
	struct spi_ioc_transfer spi_transfer;
#endif
	absl_spi_config_t*		spi_config;
	absl_event_t				transmision_event;
	uint64_t				last_tx_ts;
}absl_spi_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_spi_init						absl_spi_init_freertos
#define absl_spi_transfer					absl_spi_transfer_freertos
#define absl_spi_ts_transfer				absl_spi_ts_transfer_freertos
#define absl_spi_reset					absl_spi_reset_freertos
#define absl_spi_get_last_transfer_ts		absl_spi_get_last_transfer_ts_freertos
#elif defined(ABSL_PC) //Todo - should be windows?
#define absl_spi_init			absl_spi_init_pc
#define absl_spi_transfer		absl_spi_transfer_pc
#define absl_spi_reset		absl_spi_reset_pc
#elif defined(ABSL_LINUX)
#define absl_spi_init			absl_spi_init_linux
#define absl_spi_transfer		absl_spi_transfer_linux
#define absl_spi_reset		absl_spi_reset_linux
#else
#error Platform not defined
#endif

absl_spi_rv_t absl_spi_init(absl_spi_t * _spi, absl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff);

absl_spi_rv_t absl_spi_transfer(absl_spi_t * _spi, uint32_t _length);

absl_spi_rv_t absl_spi_ts_transfer(absl_spi_t * _spi, uint32_t _length, uint64_t* _tx_timestamp);

void absl_spi_reset(absl_spi_t * _spi);

uint64_t absl_spi_get_last_transfer_ts(absl_spi_t* _spi);

#endif /* ABSL_SPI */
#endif /* ABSL_SPI_H_ */
