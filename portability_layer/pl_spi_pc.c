/*
 * pl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_spi.h"
#ifdef PL_SPI

#if defined(PL_PC)

static bool pl_spi_init_master_freertos_pc(pl_spi_t * _spi);


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
pl_spi_rv_t pl_spi_init_pc(pl_spi_t * _spi, pl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff)
{
	pl_spi_rv_t	pl_spi_rv = PL_SPI_RV_ERROR;

	
	pl_spi_rv = PL_SPI_RV_OK;
		

	return pl_spi_rv;
}


pl_spi_rv_t pl_spi_transfer_pc(pl_spi_t * _spi, uint32_t _length)
{
	pl_spi_rv_t pl_spi_rv = PL_SPI_RV_ERROR;

	pl_spi_rv = PL_SPI_RV_OK;

	return pl_spi_rv;
}

void pl_spi_reset_pc(pl_spi_t * _spi)
{
	pl_spi_init_master_freertos_pc( _spi);
}

static pl_spi_rv_t pl_spi_transfer_master_freertos_pc(pl_spi_t * _spi, uint32_t _length)
{
	pl_spi_rv_t pl_spi_rv = PL_SPI_RV_ERROR;

	pl_spi_rv = PL_SPI_RV_OK;

	return pl_spi_rv;
}

static bool pl_spi_init_master_freertos_pc(pl_spi_t * _spi)
{
	bool return_value = false;

	return_value = true;

	return return_value;
}

#endif
#endif /* PL_SPI */
