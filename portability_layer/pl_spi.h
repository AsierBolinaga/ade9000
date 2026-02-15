/*
 * pl_spi.h
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_SPI_H_
#define PL_SPI_H_

#include "pl_config.h"
#ifdef PL_SPI

#if defined(PL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "fsl_lpspi.h"
#include "fsl_lpspi_freertos.h"
#elif defined(PL_LINUX)
#include <linux/spi/spidev.h>
#endif

#include "pl_mutex.h"
#include "pl_event.h"

typedef enum pl_spi_rv
{
    PL_SPI_RV_OK = 0x0U,
    PL_SPI_RV_ERROR,
	PL_SPI_RV_NO_CONF
} pl_spi_rv_t;

typedef enum pl_spi_mode
{
    PL_SPI_MODE_MASTER = 0x0U,
    PL_SPI_MODE_SLAVE
} pl_spi_mode_t;


typedef struct pl_spi_config
{
	LPSPI_Type *			spi_base;
	DMA_Type *				dma_base;
	pl_spi_mode_t			spi_mode;
	uint32_t 				baud_rate;
	uint32_t				transfer_timeout_ms;
} pl_spi_config_t;

typedef struct pl_spi
{
	pl_mutex_t				spi_mutex;
#if defined(PL_OS_FREE_RTOS)
	lpspi_rtos_handle_t  	spi_handle;
	lpspi_transfer_t 	 	spi_transfer;
#elif defined(PL_LINUX)
    int 					spi_handle;
	struct spi_ioc_transfer spi_transfer;
#endif
	pl_spi_config_t*		spi_config;
	pl_event_t				transmision_event;
	uint64_t				last_tx_ts;
}pl_spi_t;

#if defined(PL_OS_FREE_RTOS)
#define pl_spi_init						pl_spi_init_freertos
#define pl_spi_transfer					pl_spi_transfer_freertos
#define pl_spi_ts_transfer				pl_spi_ts_transfer_freertos
#define pl_spi_reset					pl_spi_reset_freertos
#define pl_spi_get_last_transfer_ts		pl_spi_get_last_transfer_ts_freertos
#elif defined(PL_PC) //Todo - should be windows?
#define pl_spi_init			pl_spi_init_pc
#define pl_spi_transfer		pl_spi_transfer_pc
#define pl_spi_reset		pl_spi_reset_pc
#elif defined(PL_LINUX)
#define pl_spi_init			pl_spi_init_linux
#define pl_spi_transfer		pl_spi_transfer_linux
#define pl_spi_reset		pl_spi_reset_linux
#else
#error Platform not defined
#endif

pl_spi_rv_t pl_spi_init(pl_spi_t * _spi, pl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff);

pl_spi_rv_t pl_spi_transfer(pl_spi_t * _spi, uint32_t _length);

pl_spi_rv_t pl_spi_ts_transfer(pl_spi_t * _spi, uint32_t _length, uint64_t* _tx_timestamp);

void pl_spi_reset(pl_spi_t * _spi);

uint64_t pl_spi_get_last_transfer_ts(pl_spi_t* _spi);

#endif /* PL_SPI */
#endif /* PL_SPI_H_ */
