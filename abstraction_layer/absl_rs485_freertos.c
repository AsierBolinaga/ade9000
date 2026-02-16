/*
 * absl_rs485.c - Versión mejorada para recepción DMA + interrupción IdleLine
 *
 * Creado para Asier Bolinaga "Boli" por B.A.I
 */

#include "absl_rs485.h"
#ifdef ABSL_RS485

#if defined(ABSL_OS_FREE_RTOS)
#include "fsl_gpio.h"
#include "fsl_dmamux.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_lpuart_edma.h"

#include "absl_thread.h"
#include "absl_time.h"
#include "absl_debug.h"
#include "absl_macros.h"

#define TRANSMISSION_COMPLETED 0x00000001
#define RECEPTION_COMPLETED    0x00000002

static void absl_rs485_freertos_set_direction_tx(absl_rs485_config_t* _rs485_config, bool enable);

static absl_rs485_rv_t absl_rs485_get_parity(absl_rs485_config_t * _rs485_config, lpuart_parity_mode_t* _parity);
static absl_rs485_rv_t absl_rs485_get_data_bits(absl_rs485_config_t * _rs485_config, lpuart_data_bits_t* _data_bits);
static absl_rs485_rv_t absl_rs485_get_stop_bit(absl_rs485_config_t * _rs485_config, lpuart_stop_bit_count_t* _stop_bit);

void absl_rs485_callback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    absl_rs485_t* rs485 = (absl_rs485_t*)userData;

    ABSL_UNUSED_ARG(handle);
    ABSL_UNUSED_ARG(base);

	if (ABSL_RS485_TX == rs485->rs485_state)
	{
		absl_rs485_freertos_set_direction_tx(rs485->rs485_config, false);
		rs485->rs485_state = ABSL_RS485_RX;
		absl_event_set_fromISR(&rs485->transmision_event, TRANSMISSION_COMPLETED);
	}
	if (ABSL_RS485_RX == rs485->rs485_state)
	{
		rs485->rs485_state = ABSL_RS485_IDLE;
		absl_event_set_fromISR(&rs485->transmision_event, RECEPTION_COMPLETED);
	}
	else
	{
		rs485->rs485_state = ABSL_RS485_IDLE;
	}
}

uint32_t BOARD_SrcFreq(void)
{
    return (CLOCK_GetPllFreq(kCLOCK_PllUsb1) / 6U) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1U);
}

absl_rs485_rv_t absl_rs485_init_freertos(absl_rs485_t * _rs485, absl_rs485_config_t* _rs485_config)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;
    lpuart_config_t config;
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    edma_config_t dmaConfig;

    if(NULL != _rs485_config)
    {
    	if(false == _rs485_config->is_initialized)
    	{
			_rs485->rs485_config = _rs485_config;

			GPIO_PinInit(_rs485_config->gpio, _rs485_config->gpio_pin, &gpio_config);
			absl_rs485_freertos_set_direction_tx(_rs485_config, false);

			LPUART_GetDefaultConfig(&config);

			if((ABSL_RS485_RV_OK == absl_rs485_get_parity(_rs485_config, &config.parityMode)) &&
			   (ABSL_RS485_RV_OK == absl_rs485_get_data_bits(_rs485_config, &config.dataBitsCount)) &&
			   (ABSL_RS485_RV_OK == absl_rs485_get_stop_bit(_rs485_config, &config.stopBitCount)))
			{
				config.baudRate_Bps = _rs485_config->baud_rate;
				config.enableTx = true;
				config.enableRx = true;

				NVIC_SetPriority(_rs485_config->uart_irq_type, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);

				LPUART_Init(_rs485_config->uart_base, &config, BOARD_SrcFreq());

				LPUART_EnableInterrupts(_rs485_config->uart_base, kLPUART_IdleLineFlag);

				NVIC_SetPriority(_rs485_config->irq_type_tx, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
				EnableIRQ(_rs485_config->irq_type_tx);

				NVIC_SetPriority(_rs485_config->irq_type_rx, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
				EnableIRQ(_rs485_config->irq_type_rx);

				DMAMUX_Init(DMAMUX);
				DMAMUX_SetSource(DMAMUX, _rs485_config->dma_tx_ch, _rs485_config->dma_request_tx_src);
				DMAMUX_SetSource(DMAMUX, _rs485_config->dma_rx_ch, _rs485_config->dma_request_rx_src);
				DMAMUX_EnableChannel(DMAMUX, _rs485_config->dma_tx_ch);
				DMAMUX_EnableChannel(DMAMUX, _rs485_config->dma_rx_ch);

				EDMA_GetDefaultConfig(&dmaConfig);
				EDMA_Init(_rs485_config->dma_base, &dmaConfig);
				EDMA_CreateHandle(&_rs485->txEdmaHandle, _rs485_config->dma_base, _rs485_config->dma_tx_ch);
				EDMA_CreateHandle(&_rs485->rxEdmaHandle, _rs485_config->dma_base, _rs485_config->dma_rx_ch);

				LPUART_TransferCreateHandleEDMA(_rs485_config->uart_base, &_rs485->lpuartEdmaHandle, absl_rs485_callback,
												_rs485, &_rs485->txEdmaHandle, &_rs485->rxEdmaHandle);

				_rs485->rs485_state = ABSL_RS485_IDLE;

				absl_mutex_create(&_rs485->rs485_mutex);
				absl_event_create(&_rs485->transmision_event);

				_rs485_config->is_initialized = true;

				absl_rs485_rv = ABSL_RS485_RV_OK;
			}
    	}
    	else
    	{
    		absl_rs485_rv = ABSL_RS485_RV_OK;
    	}
    }

    return absl_rs485_rv;
}

absl_rs485_rv_t absl_rs485_transfer_freertos(absl_rs485_t * _rs485, uint8_t* _buff, uint32_t _size, uint32_t _timeout)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;
    uint32_t rs485_event;

    uint8_t buff[_size];

    absl_mutex_take(&_rs485->rs485_mutex);

    _rs485->rs485_tx.data = _buff;
    _rs485->rs485_tx.dataSize = _size;
    _rs485->rs485_rx.data = buff;
	_rs485->rs485_rx.dataSize = _size;

    absl_rs485_freertos_set_direction_tx(_rs485->rs485_config, true);
    _rs485->rs485_state = ABSL_RS485_TX;
    LPUART_SendEDMA(_rs485->rs485_config->uart_base, &_rs485->lpuartEdmaHandle, &_rs485->rs485_tx);
	LPUART_ReceiveEDMA(_rs485->rs485_config->uart_base, &_rs485->lpuartEdmaHandle, &_rs485->rs485_rx);

	if(ABSL_EVENT_RV_OK == absl_event_timed_wait(&_rs485->transmision_event, TRANSMISSION_COMPLETED,
											 &rs485_event, _timeout))
	{
		if(ABSL_EVENT_RV_OK == absl_event_timed_wait(&_rs485->transmision_event, RECEPTION_COMPLETED,
												 &rs485_event, _timeout))
		{
			absl_rs485_rv = ABSL_RS485_RV_OK;
		}
	}

    absl_mutex_give(&_rs485->rs485_mutex);
    return absl_rs485_rv;
}

absl_rs485_rv_t absl_rs485_receive_freertos(absl_rs485_t * _rs485, uint8_t* _buff, uint32_t _size, uint32_t _timeout)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;
    uint32_t rs485_event;

    absl_mutex_take(&_rs485->rs485_mutex);

    _rs485->rs485_rx.data = _buff;
    _rs485->rs485_rx.dataSize = _size;

    _rs485->rs485_state = ABSL_RS485_RX;
    absl_event_clear_events(&_rs485->transmision_event, RECEPTION_COMPLETED);
    LPUART_ReceiveEDMA(_rs485->rs485_config->uart_base, &_rs485->lpuartEdmaHandle, &_rs485->rs485_rx);

    if (ABSL_EVENT_RV_OK == absl_event_timed_wait(&_rs485->transmision_event, RECEPTION_COMPLETED, &rs485_event, _timeout))
    {
        absl_rs485_rv = ABSL_RS485_RV_OK;
    }
    else
    {
    	LPUART_TransferAbortReceiveEDMA(_rs485->rs485_config->uart_base, &_rs485->lpuartEdmaHandle);
    }

    absl_mutex_give(&_rs485->rs485_mutex);
    return absl_rs485_rv;
}

static absl_rs485_rv_t absl_rs485_get_parity(absl_rs485_config_t * _rs485_config, lpuart_parity_mode_t* _parity)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;

	switch(_rs485_config->parity)
	{
	case ABSL_RS485_PARITY_DISABLED:
		*_parity = kLPUART_ParityDisabled;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	case ABSL_RS485_PARITY_EVEN:
		*_parity = kLPUART_ParityEven;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	case ABSL_RS485_PARITY_ODD:
		*_parity = kLPUART_ParityOdd;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	default:
		break;
	}

	return absl_rs485_rv;
}

static absl_rs485_rv_t absl_rs485_get_data_bits(absl_rs485_config_t * _rs485_config, lpuart_data_bits_t* _data_bits)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;

	switch(_rs485_config->data_bits)
	{
	case ABSL_RS485_DATA_BITS_EIGHT:
		*_data_bits = kLPUART_EightDataBits;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	case ABSL_RS485_DATA_BITS_SEVEN:
		*_data_bits = kLPUART_SevenDataBits;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	default:
		break;
	}

	return absl_rs485_rv;
}

static absl_rs485_rv_t absl_rs485_get_stop_bit(absl_rs485_config_t * _rs485_config, lpuart_stop_bit_count_t* _stop_bit)
{
    absl_rs485_rv_t absl_rs485_rv = ABSL_RS485_RV_ERROR;

	switch(_rs485_config->stop_bit)
	{
	case ABSL_RS485_STOP_BIT_ONE:
		*_stop_bit = kLPUART_OneStopBit;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	case ABSL_RS485_STOP_BIT_TWO:
		*_stop_bit = kLPUART_TwoStopBit;
		absl_rs485_rv = ABSL_RS485_RV_OK;
		break;
	default:
		break;
	}

	return absl_rs485_rv;
}

static void absl_rs485_freertos_set_direction_tx(absl_rs485_config_t* _rs485_config, bool enable)
{
    GPIO_PinWrite(_rs485_config->gpio, _rs485_config->gpio_pin, enable ? 1 : 0);
    __NOP(); __NOP();
}

#endif
#endif /* ABSL_RS485 */
