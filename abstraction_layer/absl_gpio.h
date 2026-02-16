/*
 * absl_gpio.h
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#ifndef ABSL_GPIO_H_
#define ABSL_GPIO_H_

#include "absl_config.h"
#ifdef ABSL_GPIO

#ifndef ABSL_EVENT
#error ERROR! absl_gpio module needs event module!
#else
#include "absl_event.h"
#endif

#if defined(ABSL_IMX_RT10XX)
#include "fsl_gpio.h"
#endif

typedef enum absl_gpio_rv
{
	ABSL_GPIO_RV_OK = 0,
	ABSL_GPIO_RV_INCORRECT_CONFIG,
	ABSL_GPIO_RV_ERROR
}absl_gpio_rv_t;

typedef enum absl_gpio_pin_state
{
	ABSL_GPIO_PIN_OFF = 0,
	ABSL_GPIO_PIN_ON
}absl_gpio_pin_state_t;

/*! @brief GPIO interrupt mode definition. */
typedef enum absl_gpio_int_mode
{
	ABSL_GPIO_NO_INT 				= 0x00000000,
	ABSL_GPIO_HIGH_LEVEL          = 0x00000001,
	ABSL_GPIO_LOW_LEVEL           = 0x00000002,
	ABSL_GPIO_RISING_EDGE         = 0x00000004,
	ABSL_GPIO_FALLING_EDGE        = 0x00000008,
	ABSL_GPIO_FALLING_RISING_EDGE = 0x00000010
} absl_gpio_int_mode_t;

typedef struct absl_gpio_config
{
#if defined(ABSL_IMX_RT10XX)
	GPIO_Type *				gpio;
	int32_t					gpio_pin;
	gpio_pin_direction_t 	direction;
	uint8_t 				outputLogic;
#endif
}absl_gpio_config_t;

typedef struct absl_gpio
{
	absl_gpio_config_t* 	gpio_config; 			/* absl_gpio configuration pointer */
#if defined(ABSL_IMX_RT10XX)
	uint32_t			gpio_irq;				/* iMXRT10XX gpio interrupt */
#endif
	absl_event_t 			gpio_event_group;		/* Event group to give gpio related events */
	absl_gpio_int_mode_t	gpio_event;				/* Event flag to give */
}absl_gpio_t;

#if defined(ABSL_IMX_RT10XX)
#define absl_gpio_init		absl_gpio_init_imxrt10xx
#define absl_gpio_on			absl_gpio_on_imxrt10xx
#define absl_gpio_off			absl_gpio_off_imxrt10xx
#define absl_gpio_toggle		absl_gpio_toggle_imxrt10xx
#define absl_gpio_get			absl_gpio_get_imxrt10xx
#else
#error Platform not defined
#endif

/*!
 * @brief Initialized the specified pin with the given configuration
 *
 * @param *_absl_gpio  		Pointer to absl_gpio_t instance
 * @param *_gpio_config		Pointer to gpio configuration
 * @param int_mode			Specifies the type of interruption to generate if needed,
 * 							if not specify  ABSL_GPIO_NO_INT
 * @param
 *
 * @return absl_event_rv_t 	 ABSL_EVENT_RV_OK 	if event group was correctly created
 * 							 ABSL_EVENT_RV_ERROR 	if there was an error on creation
 */
absl_gpio_rv_t absl_gpio_init(absl_gpio_t* _absl_gpio, absl_gpio_config_t* _gpio_config, absl_gpio_int_mode_t _int_mode);

void absl_gpio_on(absl_gpio_t* _absl_gpio);

void absl_gpio_off(absl_gpio_t* _absl_gpio);

void absl_gpio_toggle(absl_gpio_t* _absl_gpio);

absl_gpio_pin_state_t absl_gpio_get(absl_gpio_t* _absl_gpio);

#endif /* ABSL_GPIO */
#endif /* ABSL_GPIO_H_ */
