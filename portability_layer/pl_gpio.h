/*
 * pl_gpio.h
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef PL_GPIO_H_
#define PL_GPIO_H_

#include "pl_config.h"
#ifdef PL_GPIO

#ifndef PL_EVENT
#error ERROR! pl_gpio module needs event module!
#else
#include "pl_event.h"
#endif

#if defined(PL_IMX_RT10XX)
#include "fsl_gpio.h"
#endif

typedef enum pl_gpio_rv
{
	PL_GPIO_RV_OK = 0,
	PL_GPIO_RV_INCORRECT_CONFIG,
	PL_GPIO_RV_ERROR
}pl_gpio_rv_t;

typedef enum pl_gpio_pin_state
{
	PL_GPIO_PIN_OFF = 0,
	PL_GPIO_PIN_ON
}pl_gpio_pin_state_t;

/*! @brief GPIO interrupt mode definition. */
typedef enum pl_gpio_int_mode
{
	PL_GPIO_NO_INT 				= 0x00000000,
	PL_GPIO_HIGH_LEVEL          = 0x00000001,
	PL_GPIO_LOW_LEVEL           = 0x00000002,
	PL_GPIO_RISING_EDGE         = 0x00000004,
	PL_GPIO_FALLING_EDGE        = 0x00000008,
	PL_GPIO_FALLING_RISING_EDGE = 0x00000010
} pl_gpio_int_mode_t;

typedef struct pl_gpio_config
{
#if defined(PL_IMX_RT10XX)
	GPIO_Type *				gpio;
	int32_t					gpio_pin;
	gpio_pin_direction_t 	direction;
	uint8_t 				outputLogic;
#endif
}pl_gpio_config_t;

typedef struct pl_gpio
{
	pl_gpio_config_t* 	gpio_config; 			/* pl_gpio configuration pointer */
#if defined(PL_IMX_RT10XX)
	uint32_t			gpio_irq;				/* iMXRT10XX gpio interrupt */
#endif
	pl_event_t 			gpio_event_group;		/* Event group to give gpio related events */
	pl_gpio_int_mode_t	gpio_event;				/* Event flag to give */
}pl_gpio_t;

#if defined(PL_IMX_RT10XX)
#define pl_gpio_init		pl_gpio_init_imxrt10xx
#define pl_gpio_on			pl_gpio_on_imxrt10xx
#define pl_gpio_off			pl_gpio_off_imxrt10xx
#define pl_gpio_toggle		pl_gpio_toggle_imxrt10xx
#define pl_gpio_get			pl_gpio_get_imxrt10xx
#else
#error Platform not defined
#endif

/*!
 * @brief Initialized the specified pin with the given configuration
 *
 * @param *_pl_gpio  		Pointer to pl_gpio_t instance
 * @param *_gpio_config		Pointer to gpio configuration
 * @param int_mode			Specifies the type of interruption to generate if needed,
 * 							if not specify  PL_GPIO_NO_INT
 * @param
 *
 * @return pl_event_rv_t 	 PL_EVENT_RV_OK 	if event group was correctly created
 * 							 PL_EVENT_RV_ERROR 	if there was an error on creation
 */
pl_gpio_rv_t pl_gpio_init(pl_gpio_t* _pl_gpio, pl_gpio_config_t* _gpio_config, pl_gpio_int_mode_t _int_mode);

void pl_gpio_on(pl_gpio_t* _pl_gpio);

void pl_gpio_off(pl_gpio_t* _pl_gpio);

void pl_gpio_toggle(pl_gpio_t* _pl_gpio);

pl_gpio_pin_state_t pl_gpio_get(pl_gpio_t* _pl_gpio);

#endif /* PL_GPIO */
#endif /* PL_GPIO_H_ */
