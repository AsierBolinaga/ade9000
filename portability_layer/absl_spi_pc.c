/*
 * absl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_spi.h"
#ifdef ABSL_SPI

#if defined(ABSL_PC)

static bool absl_spi_init_master_freertos_pc(absl_spi_t * _spi);


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
absl_spi_rv_t absl_spi_init_pc(absl_spi_t * _spi, absl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff)
{
	absl_spi_rv_t	absl_spi_rv = ABSL_SPI_RV_ERROR;

	
	absl_spi_rv = ABSL_SPI_RV_OK;
		

	return absl_spi_rv;
}


absl_spi_rv_t absl_spi_transfer_pc(absl_spi_t * _spi, uint32_t _length)
{
	absl_spi_rv_t absl_spi_rv = ABSL_SPI_RV_ERROR;

	absl_spi_rv = ABSL_SPI_RV_OK;

	return absl_spi_rv;
}

void absl_spi_reset_pc(absl_spi_t * _spi)
{
	absl_spi_init_master_freertos_pc( _spi);
}

static absl_spi_rv_t absl_spi_transfer_master_freertos_pc(absl_spi_t * _spi, uint32_t _length)
{
	absl_spi_rv_t absl_spi_rv = ABSL_SPI_RV_ERROR;

	absl_spi_rv = ABSL_SPI_RV_OK;

	return absl_spi_rv;
}

static bool absl_spi_init_master_freertos_pc(absl_spi_t * _spi)
{
	bool return_value = false;

	return_value = true;

	return return_value;
}

#endif
#endif /* ABSL_SPI */
