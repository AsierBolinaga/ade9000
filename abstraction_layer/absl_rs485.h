/*
 * absl_rs485.h
 *
 *  Created on: May 6, 2025
 *      Author: abolinaga
 */

#ifndef ABSL_RS485_H_
#define ABSL_RS485_H_

#include "absl_config.h"
#ifdef ABSL_RS485

#if defined(ABSL_OS_FREE_RTOS)
#include "FreeRTOS.h"
#include "fsl_lpuart_edma.h"
#endif

#include "absl_mutex.h"
#include "absl_event.h"

typedef enum absl_rs485_rv
{
    ABSL_RS485_RV_OK = 0x0U,
    ABSL_RS485_RV_ERROR,
	ABSL_RS485_RV_NO_CONF
} absl_rs485_rv_t;

typedef enum absl_rs485_mode
{
    ABSL_RS485_MODE_MASTER = 0x0U,
    ABSL_RS485_MODE_SLAVE
} absl_rs485_mode_t;


typedef enum absl_rs485_state
{
    ABSL_RS485_TX = 0x0U,
    ABSL_RS485_RX,
	ABSL_RS485_IDLE
} absl_rs485_state_t;

typedef enum absl_rs485_parity
{
    ABSL_RS485_PARITY_DISABLED = 0x0U,
	ABSL_RS485_PARITY_EVEN,
	ABSL_RS485_PARITY_ODD,
	ABSL_RS485_PARITY_UNKNOWN
}absl_rs485_parity_t;

typedef enum absl_rs485_data_bits
{
	ABSL_RS485_DATA_BITS_EIGHT = 0x0U,
	ABSL_RS485_DATA_BITS_SEVEN,
	ABSL_RS485_DATA_BITS_UNKNOWN,
} absl_rs485_data_bits_t;

typedef enum absl_rs485_stop_bit
{
	ABSL_RS485_STOP_BIT_ONE = 0U,
	ABSL_RS485_STOP_BIT_TWO,
	ABSL_RS485_STOP_BIT_UNKNOWN
} absl_rs485_stop_bit_t;

typedef struct absl_rs485_config
{
	LPUART_Type *			uart_base;
	IRQn_Type				uart_irq_type;
	GPIO_Type *				gpio;
	int32_t					gpio_pin;
	uint32_t 				baud_rate;
	absl_rs485_parity_t		parity;
	absl_rs485_data_bits_t	data_bits;
	absl_rs485_stop_bit_t		stop_bit;
	DMA_Type *				dma_base;
	IRQn_Type				irq_type_tx;
	IRQn_Type				irq_type_rx;
	uint32_t				dma_tx_ch;
	uint32_t				dma_rx_ch;
	dma_request_source_t	dma_request_tx_src;
	dma_request_source_t	dma_request_rx_src;
	uint32_t				transfer_timeout_ms;
	bool					is_initialized;
} absl_rs485_config_t;

typedef struct absl_rs485
{
	absl_mutex_t				rs485_mutex;
#if defined(ABSL_OS_FREE_RTOS)
	lpuart_transfer_t 		rs485_tx;
	lpuart_transfer_t 		rs485_rx;
	lpuart_edma_handle_t 	lpuartEdmaHandle;
	edma_handle_t 			txEdmaHandle;
	edma_handle_t 			rxEdmaHandle;
#endif
	absl_rs485_config_t*		rs485_config;
	absl_rs485_state_t		rs485_state;
	absl_event_t				transmision_event;
	uint32_t				received_size;
}absl_rs485_t;

#if defined(ABSL_OS_FREE_RTOS)
#define absl_rs485_init					absl_rs485_init_freertos
#define absl_rs485_transfer				absl_rs485_transfer_freertos
#define absl_rs485_receive				absl_rs485_receive_freertos
#else
#error Platform not defined
#endif

absl_rs485_rv_t absl_rs485_init(absl_rs485_t * _rs485, absl_rs485_config_t* _rs485_config);

absl_rs485_rv_t absl_rs485_transfer(absl_rs485_t * _rs485, uint8_t* _buff, uint32_t _size, uint32_t _timeout);

absl_rs485_rv_t absl_rs485_receive(absl_rs485_t * _rs485, uint8_t* _buff, uint32_t _size, uint32_t _timeout);

#endif /* ABSL_RS485 */
#endif /* ABSL_RS485_H_ */
