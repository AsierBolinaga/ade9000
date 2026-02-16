/*
 * absl_spi.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_spi.h"
#ifdef ABSL_SPI

#if defined(ABSL_OS_FREE_RTOS)
#include "fsl_dmamux.h"
#include "fsl_lpspi_edma.h"
#include "fsl_common.h"

#include "absl_thread.h"
#include "absl_time.h"
#include "absl_debug.h"

#include "absl_macros.h"

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define LPSPI_CLOCK_SOURCE_DIVIDER (0U)

#define LPSPI_CLOCK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (LPSPI_CLOCK_SOURCE_DIVIDER + 1U))

#define TRANSMISSION_COMPLETED 	0x00000001

AT_NONCACHEABLE_SECTION_INIT(lpspi_master_edma_handle_t g_m_edma_handle) = {0};
edma_handle_t lpspiEdmaMasterRxRegToRxDataHandle;
edma_handle_t lpspiEdmaMasterTxDataToTxRegHandle;

static bool absl_spi_init_master_freertos(absl_spi_t * _spi);

static bool absl_spi_dmux_init_freertos(absl_spi_config_t * _spi_config);
static absl_spi_rv_t absl_spi_transfer_master_freertos(absl_spi_t * _spi, uint32_t _length, uint64_t* _tx_timestamp);


void LPSPI_MasterUserCallback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
	absl_spi_t* spi = (absl_spi_t*)userData;

	ABSL_UNUSED_ARG(base);
	ABSL_UNUSED_ARG(handle);

    if (status == kStatus_Success)
    {
    	spi->last_tx_ts = absl_time_to_us(absl_time_gettime());
    	absl_event_set_fromISR(&spi->transmision_event, TRANSMISSION_COMPLETED);
    }
}


/** @brief Initialize the spi bus for the ade9000
 *
 * @return bool		TRUE: 	init OK
 * 					FALSE:	init NOK
 */
absl_spi_rv_t absl_spi_init_freertos(absl_spi_t * _spi, absl_spi_config_t* _spi_config, uint8_t* tx_buff, uint8_t* rx_buff)
{
	absl_spi_rv_t	absl_spi_rv = ABSL_SPI_RV_ERROR;

	if(NULL != _spi_config)
	{
		_spi->spi_config = _spi_config;

		_spi->spi_transfer.txData = tx_buff;
		_spi->spi_transfer.rxData = rx_buff;

		/*Set clock source for LPSPI*/
		CLOCK_SetMux(kCLOCK_LpspiMux, LPSPI_CLOCK_SOURCE_SELECT);
		CLOCK_SetDiv(kCLOCK_LpspiDiv, LPSPI_CLOCK_SOURCE_DIVIDER);

		if(ABSL_SPI_MODE_MASTER == _spi_config->spi_mode)
		{
			if(absl_spi_init_master_freertos(_spi))
			{
				absl_mutex_create(&_spi->spi_mutex);
				absl_event_create(&_spi->transmision_event);
				absl_spi_rv = ABSL_SPI_RV_OK;
			}
		}
		else
		{
			/* Todo - implement slave initialization */
		}
	}
	else
	{
		absl_spi_rv = ABSL_SPI_RV_NO_CONF;
	}

	return absl_spi_rv;
}


absl_spi_rv_t absl_spi_transfer_freertos(absl_spi_t * _spi, uint32_t _length)
{
	absl_spi_rv_t absl_spi_rv = ABSL_SPI_RV_ERROR;

	absl_mutex_take(&_spi->spi_mutex);
	if(ABSL_SPI_MODE_MASTER == _spi->spi_config->spi_mode)
	{
		absl_spi_rv = absl_spi_transfer_master_freertos(_spi, _length, NULL);
	}
	else
	{
		/* Todo - slave transmit has to be implemented */
	}

	return absl_spi_rv;
}

absl_spi_rv_t absl_spi_ts_transfer_freertos(absl_spi_t * _spi, uint32_t _length, uint64_t* _tx_timestamp)
{
	absl_spi_rv_t absl_spi_rv = ABSL_SPI_RV_ERROR;

	absl_mutex_take(&_spi->spi_mutex);
	if(ABSL_SPI_MODE_MASTER == _spi->spi_config->spi_mode)
	{
		absl_spi_rv = absl_spi_transfer_master_freertos(_spi, _length, _tx_timestamp);
	}
	else
	{
		/* Todo - slave transmit has to be implemented */
	}

	return absl_spi_rv;
}

void absl_spi_reset(absl_spi_t * _spi)
{
	LPSPI_RTOS_Deinit(&_spi->spi_handle);
	absl_spi_init_master_freertos( _spi);
}

uint64_t absl_spi_get_last_transfer_ts_freertos(absl_spi_t* _spi)
{
	return _spi->last_tx_ts;
}

static absl_spi_rv_t absl_spi_transfer_master_freertos(absl_spi_t * _spi, uint32_t _length, uint64_t* _tx_timestamp)
{
	absl_spi_rv_t absl_spi_rv = ABSL_SPI_RV_ERROR;
	status_t status;
	uint32_t spi_event = 0;

	_spi->spi_transfer.dataSize    = _length + 1;
	_spi->spi_transfer.configFlags = kLPSPI_MasterPcs0 | kLPSPI_MasterByteSwap | kLPSPI_MasterPcsContinuous;

	status = LPSPI_MasterTransferEDMA(_spi->spi_config->spi_base, &g_m_edma_handle, &_spi->spi_transfer);

	if (status == kStatus_Success)
	{
		if(ABSL_EVENT_RV_OK == absl_event_timed_wait(&_spi->transmision_event, TRANSMISSION_COMPLETED,
												 &spi_event, _spi->spi_config->transfer_timeout_ms))
		{
			if(_tx_timestamp != NULL)
			{
				*_tx_timestamp = _spi->last_tx_ts;
			}
			absl_mutex_give(&_spi->spi_mutex);
			absl_spi_rv = ABSL_SPI_RV_OK;
		}
	}
	else
	{
	/* transfer error */
	}

	return absl_spi_rv;
}

static bool absl_spi_init_master_freertos(absl_spi_t * _spi)
{
	bool return_value = false;

	lpspi_master_config_t spi_config;
    edma_config_t 		  dma_config;

	NVIC_SetPriority(DMA0_DMA16_IRQn, (configMAX_SYSCALL_INTERRUPT_PRIORITY + 1) * 2);
	EnableIRQ(DMA0_DMA16_IRQn);

	if(true == absl_spi_dmux_init_freertos(_spi->spi_config))
	{
		/* EDMA init*/
		/*
		 * userConfig.enableRoundRobinArbitration = false;
		 * userConfig.enableHaltOnError = true;
		 * userConfig.enableContinuousLinkMode = false;
		 * userConfig.enableDebugMode = false;
		 */
		EDMA_GetDefaultConfig(&dma_config);
		EDMA_Init(_spi->spi_config->dma_base, &dma_config);

		/* Master config */
		LPSPI_MasterGetDefaultConfig(&spi_config);

		spi_config.baudRate 	= _spi->spi_config->baud_rate;
		spi_config.bitsPerFrame = 8;
		spi_config.cpol         = kLPSPI_ClockPolarityActiveHigh;
		spi_config.cpha         = kLPSPI_ClockPhaseFirstEdge;
		spi_config.direction    = kLPSPI_MsbFirst;

		spi_config.pcsToSckDelayInNanoSec        = 1000000000U / spi_config.baudRate * 2U;
		spi_config.lastSckToPcsDelayInNanoSec    = 1000000000U / spi_config.baudRate * 2U;
		spi_config.betweenTransferDelayInNanoSec = 1000000000U / spi_config.baudRate * 2U;

		spi_config.whichPcs           = kLPSPI_Pcs0;
		spi_config.pcsActiveHighOrLow = kLPSPI_PcsActiveLow;

		spi_config.pinCfg        = kLPSPI_SdiInSdoOut;
		spi_config.dataOutConfig = kLpspiDataOutRetained;

		LPSPI_MasterInit(_spi->spi_config->spi_base, &spi_config, LPSPI_CLOCK_FREQ);

		/*Set up lpspi master*/
		memset(&(lpspiEdmaMasterRxRegToRxDataHandle), 0, sizeof(lpspiEdmaMasterRxRegToRxDataHandle));
		memset(&(lpspiEdmaMasterTxDataToTxRegHandle), 0, sizeof(lpspiEdmaMasterTxDataToTxRegHandle));

		EDMA_CreateHandle(&(lpspiEdmaMasterRxRegToRxDataHandle), _spi->spi_config->dma_base, 0U);
		EDMA_CreateHandle(&(lpspiEdmaMasterTxDataToTxRegHandle), _spi->spi_config->dma_base, 1U);
	#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
		EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL,
						   DEMO_LPSPI_TRANSMIT_EDMA_CHANNEL);
		EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL,
						   DEMO_LPSPI_RECEIVE_EDMA_CHANNEL);
	#endif
		LPSPI_MasterTransferCreateHandleEDMA(_spi->spi_config->spi_base, &g_m_edma_handle, LPSPI_MasterUserCallback,
				(void*)_spi, &lpspiEdmaMasterRxRegToRxDataHandle, &lpspiEdmaMasterTxDataToTxRegHandle);

		return_value = true;
	}

	return return_value;
}

static bool absl_spi_dmux_init_freertos(absl_spi_config_t * _spi_config)
{
	bool return_value = false;
	/* DMA MUX init*/
	DMAMUX_Init(DMAMUX);

	if(LPSPI1 == _spi_config->spi_base)
	{
		DMAMUX_SetSource(DMAMUX, 0U, kDmaRequestMuxLPSPI1Rx);
		DMAMUX_EnableChannel(DMAMUX, 0U);
		DMAMUX_SetSource(DMAMUX, 1U, kDmaRequestMuxLPSPI1Tx);
		DMAMUX_EnableChannel(DMAMUX, 1U);
		return_value = true;
	}
	else if (LPSPI2 == _spi_config->spi_base)
	{
		DMAMUX_SetSource(DMAMUX, 0U, kDmaRequestMuxLPSPI2Rx);
		DMAMUX_EnableChannel(DMAMUX, 0U);
		DMAMUX_SetSource(DMAMUX, 1U, kDmaRequestMuxLPSPI2Tx);
		DMAMUX_EnableChannel(DMAMUX, 1U);
		return_value = true;
	}
	else if (LPSPI3 == _spi_config->spi_base)
	{
		DMAMUX_SetSource(DMAMUX, 0U, kDmaRequestMuxLPSPI3Rx);
		DMAMUX_EnableChannel(DMAMUX, 0U);
		DMAMUX_SetSource(DMAMUX, 1U, kDmaRequestMuxLPSPI3Tx);
		DMAMUX_EnableChannel(DMAMUX, 1U);
		return_value = true;
	}
	else if (LPSPI4 == _spi_config->spi_base)
	{
		DMAMUX_SetSource(DMAMUX, 0U, kDmaRequestMuxLPSPI4Rx);
		DMAMUX_EnableChannel(DMAMUX, 0U);
		DMAMUX_SetSource(DMAMUX, 1U, kDmaRequestMuxLPSPI4Tx);
		DMAMUX_EnableChannel(DMAMUX, 1U);
		return_value = true;
	}

	return return_value;
}

#endif
#endif /* ABSL_SPI */
