/*
 * pl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_spi.h"
#ifdef PL_SPI

#if defined(PL_LINUX)

#include "pl_thread.h"
#include "pl_debug.h"

#include <fcntl.h>

//todo - make configurable
#define DEV_SPI "/dev/spidev1.0"
static unsigned int mode, speed; 

// /* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
// #define LPSPI_CLOCK_SOURCE_SELECT (1U)
// /* Clock divider for master lpspi clock source */
// #define LPSPI_CLOCK_SOURCE_DIVIDER (0U)

// #define LPSPI_CLOCK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (LPSPI_CLOCK_SOURCE_DIVIDER + 1U))

// #define TRANSMISSION_COMPLETED 	0x00000001

// AT_NONCACHEABLE_SECTION_INIT(lpspi_master_edma_handle_t g_m_edma_handle) = {0};
// edma_handle_t lpspiEdmaMasterRxRegToRxDataHandle;
// edma_handle_t lpspiEdmaMasterTxDataToTxRegHandle;

static bool pl_spi_init_master_linux(pl_spi_t * _spi);
static pl_spi_rv_t pl_spi_transfer_master_linux(pl_spi_t * _spi, uint32_t _length);

// void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
// {
// 	pl_spi_t* spi = (pl_spi_t*)userData;

//     if (status == kStatus_Success)
//     {
//     	pl_event_set_fromISR(&spi->transmision_event, TRANSMISSION_COMPLETED);
//     }
// }


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
pl_spi_rv_t pl_spi_init_linux(pl_spi_t * _spi, pl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff)
{
	pl_spi_rv_t	pl_spi_rv = PL_SPI_RV_ERROR;

	if(NULL != _spi_config)
	{
		_spi->spi_config = _spi_config;

		_spi->spi_transfer.tx_buf = (unsigned long)tx_buff;
		_spi->spi_transfer.rx_buf = (unsigned long)rx_buff;

		if(PL_SPI_MODE_MASTER == _spi_config->spi_mode)
		{
			if(pl_spi_init_master_linux(_spi))
			{
				pl_spi_rv = PL_SPI_RV_OK;
			}
		}
		else
		{
			/* Todo - implement slave initialization */
		}
	}
	else
	{
		pl_spi_rv = PL_SPI_RV_NO_CONF;
	}

	return pl_spi_rv;
}


pl_spi_rv_t pl_spi_transfer_linux(pl_spi_t * _spi, uint32_t _length)
{
	pl_spi_rv_t pl_spi_rv = PL_SPI_RV_ERROR;

	if(PL_SPI_MODE_MASTER == _spi->spi_config->spi_mode)
	{
		pl_spi_rv = pl_spi_transfer_master_linux(_spi, _length);
	}
	else
	{
		/* Todo - slave transmit has to be implemented */
	}

	return pl_spi_rv;
}

void pl_spi_reset(pl_spi_t * _spi)
{
	pl_spi_init_master_linux( _spi);
}

static pl_spi_rv_t pl_spi_transfer_master_linux(pl_spi_t * _spi, uint32_t _length)
{
	pl_spi_rv_t pl_spi_rv = PL_SPI_RV_ERROR;
	uint32_t length;

	if(4 < _length)
	{
		length = 4;
	}
	else
	{
		length = _length;
	}

	_spi->spi_transfer.len = length;
	_spi->spi_transfer.speed_hz = speed;
	_spi->spi_transfer.bits_per_word = (length * 8);

	if (ioctl(_spi->spi_handle, SPI_IOC_MESSAGE(1), &_spi->spi_transfer) < 0)
	{
		perror("SPI_IOC_MESSAGE");
	}
	else
	{
		pl_spi_rv = PL_SPI_RV_OK;
	}

	return pl_spi_rv;
}

static bool pl_spi_init_master_linux(pl_spi_t * _spi)
{
	bool return_value = false;
    int ret;

	// open device node
    _spi->spi_handle = open(DEV_SPI, O_RDWR);
    if (_spi->spi_handle < 0) 
        return return_value;

    // set spi mode and Set the clock polarity to active-high
    mode = SPI_MODE_0;
    if (ioctl(_spi->spi_handle, SPI_IOC_WR_MODE, &mode) < 0)
        return return_value;

    // set spi speed
    speed = 1000000U;
    if (ioctl(_spi->spi_handle, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) 
        return return_value;

    if (ioctl(_spi->spi_handle, SPI_IOC_RD_MAX_SPEED_HZ, &ret) < 0) 
        return return_value;

	return_value = true;

	return return_value;
}

#endif /* PL_LINUX */
#endif /* PL_SPI */
