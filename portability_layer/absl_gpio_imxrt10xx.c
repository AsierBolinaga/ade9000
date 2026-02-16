/*
 * absl_gpio_imxrt10xx.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "absl_gpio.h"
#ifdef ABSL_GPIO

#if defined(ABSL_IMX_RT10XX)
#include "pin_mux.h"
#include "board.h"

#define absl_gpio_1_Combined_0_15_IRQHandler 		GPIO1_Combined_0_15_IRQHandler
#define absl_gpio_1_Combined_16_31_IRQHandler 	GPIO1_Combined_16_31_IRQHandler
#define absl_gpio_2_Combined_0_15_IRQHandler 		GPIO2_Combined_0_15_IRQHandler
#define absl_gpio_2_Combined_16_31_IRQHandler 	GPIO2_Combined_16_31_IRQHandler
#define absl_gpio_3_Combined_0_15_IRQHandler 		GPIO3_Combined_0_15_IRQHandler
#define absl_gpio_3_Combined_16_31_IRQHandler 	GPIO3_Combined_16_31_IRQHandler
#define absl_gpio_4_Combined_0_15_IRQHandler 		GPIO4_Combined_0_15_IRQHandler
#define absl_gpio_4_Combined_16_31_IRQHandler 	GPIO4_Combined_16_31_IRQHandler
#define absl_gpio_5_Combined_0_15_IRQHandler 		GPIO5_Combined_0_15_IRQHandler
#define absl_gpio_5_Combined_16_31_IRQHandler 	GPIO5_Combined_16_31_IRQHandler

static absl_gpio_t* absl_gpio_irq_gpio1_0_15;
static absl_gpio_t* absl_gpio_irq_gpio1_16_31;
static absl_gpio_t* absl_gpio_irq_gpio2_0_15;
static absl_gpio_t* absl_gpio_irq_gpio2_16_31;
static absl_gpio_t* absl_gpio_irq_gpio3_0_15;
static absl_gpio_t* absl_gpio_irq_gpio3_16_31;
static absl_gpio_t* absl_gpio_irq_gpio4_0_15;
static absl_gpio_t* absl_gpio_irq_gpio4_16_31;
static absl_gpio_t* absl_gpio_irq_gpio5_0_15;
static absl_gpio_t* absl_gpio_irq_gpio5_16_31;

static absl_gpio_rv_t absl_gpio_init_irq_imxrt10xx(absl_gpio_t* _absl_gpio);

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_1_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio1_0_15->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio1_0_15->gpio_config->gpio_pin);

	absl_event_set_fromISR(&absl_gpio_irq_gpio1_0_15->gpio_event_group, absl_gpio_irq_gpio1_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_1_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio1_16_31->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio1_16_31->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio1_16_31->gpio_event_group, absl_gpio_irq_gpio1_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_2_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio2_0_15->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio2_0_15->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio2_0_15->gpio_event_group, absl_gpio_irq_gpio2_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_2_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio2_16_31->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio2_16_31->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio2_16_31->gpio_event_group, absl_gpio_irq_gpio2_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_3_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio3_0_15->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio3_0_15->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio3_0_15->gpio_event_group, absl_gpio_irq_gpio3_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_3_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio3_16_31->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio3_16_31->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio3_16_31->gpio_event_group, absl_gpio_irq_gpio3_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_4_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio4_0_15->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio4_0_15->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio4_0_15->gpio_event_group, absl_gpio_irq_gpio4_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_4_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio4_16_31->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio4_16_31->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio4_16_31->gpio_event_group, absl_gpio_irq_gpio4_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_5_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio5_0_15->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio5_0_15->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio5_0_15->gpio_event_group, absl_gpio_irq_gpio5_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void absl_gpio_5_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(absl_gpio_irq_gpio5_16_31->gpio_config->gpio,
    							 1U << absl_gpio_irq_gpio5_16_31->gpio_config->gpio_pin);

    absl_event_set_fromISR(&absl_gpio_irq_gpio5_16_31->gpio_event_group, absl_gpio_irq_gpio5_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}


absl_gpio_rv_t absl_gpio_init_imxrt10xx(absl_gpio_t* _absl_gpio, absl_gpio_config_t* _gpio_config, absl_gpio_int_mode_t _int_mode)
{
	absl_gpio_rv_t gpio_rv = ABSL_GPIO_RV_ERROR;

	if(NULL != _gpio_config)
	{
		_absl_gpio->gpio_config = _gpio_config;

		/* Define the init structure for the input switch pin */
		gpio_pin_config_t sw_config;
		sw_config.direction = _gpio_config->direction;
		sw_config.outputLogic = _gpio_config->outputLogic;
		switch(_int_mode)
		{
		case ABSL_GPIO_HIGH_LEVEL:
			sw_config.interruptMode = kGPIO_IntHighLevel;
			break;
		case ABSL_GPIO_LOW_LEVEL:
			sw_config.interruptMode = kGPIO_IntLowLevel;
			break;
		case ABSL_GPIO_RISING_EDGE:
			sw_config.interruptMode = kGPIO_IntRisingEdge;
			break;
		case ABSL_GPIO_FALLING_EDGE:
			sw_config.interruptMode = kGPIO_IntFallingEdge;
			break;
		case ABSL_GPIO_FALLING_RISING_EDGE:
			sw_config.interruptMode = kGPIO_IntRisingOrFallingEdge;
			break;
		default:
			sw_config.interruptMode = kGPIO_NoIntmode;
			break;
		}
		GPIO_PinInit(_gpio_config->gpio, _gpio_config->gpio_pin, &sw_config);

		if(ABSL_GPIO_NO_INT != _int_mode)
		{
			_absl_gpio->gpio_event = _int_mode;
			gpio_rv = absl_gpio_init_irq_imxrt10xx(_absl_gpio);
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_OK;
		}
	}
	else
	{
		/* No configuration */
		gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
	}

	return gpio_rv;
}

void absl_gpio_on_imxrt10xx(absl_gpio_t* _absl_gpio)
{
	GPIO_PinWrite(_absl_gpio->gpio_config->gpio, _absl_gpio->gpio_config->gpio_pin, 1U);
}

void absl_gpio_off_imxrt10xx(absl_gpio_t* _absl_gpio)
{
	GPIO_PinWrite(_absl_gpio->gpio_config->gpio, _absl_gpio->gpio_config->gpio_pin, 0U);
}

void absl_gpio_toggle_imxrt10xx(absl_gpio_t* _absl_gpio)
{
	GPIO_PortToggle(_absl_gpio->gpio_config->gpio, 1u << _absl_gpio->gpio_config->gpio_pin);
}

absl_gpio_pin_state_t absl_gpio_get_imxrt10xx(absl_gpio_t* _absl_gpio)
{
	if(GPIO_PinRead(_absl_gpio->gpio_config->gpio, _absl_gpio->gpio_config->gpio_pin))
	{
		return ABSL_GPIO_PIN_ON;
	}
	else
	{
		return ABSL_GPIO_PIN_OFF;
	}
}

static absl_gpio_rv_t absl_gpio_init_irq_imxrt10xx(absl_gpio_t* _absl_gpio)
{
	absl_gpio_rv_t gpio_rv = ABSL_GPIO_RV_ERROR;

	switch((uint32_t)_absl_gpio->gpio_config->gpio)
	{
	case GPIO1_BASE:
		if((_absl_gpio->gpio_config->gpio_pin) >= 0 && (_absl_gpio->gpio_config->gpio_pin < 15))
		{
			_absl_gpio->gpio_irq = GPIO1_Combined_0_15_IRQn;
			absl_gpio_irq_gpio1_0_15 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else if(_absl_gpio->gpio_config->gpio_pin >= 15 && _absl_gpio->gpio_config->gpio_pin < 31)
		{
			_absl_gpio->gpio_irq = GPIO1_Combined_16_31_IRQn;
			absl_gpio_irq_gpio1_16_31 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO2_BASE:
		if((_absl_gpio->gpio_config->gpio_pin) >= 0 && (_absl_gpio->gpio_config->gpio_pin < 15))
		{
			_absl_gpio->gpio_irq = GPIO2_Combined_0_15_IRQn;
			absl_gpio_irq_gpio2_0_15 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else if(_absl_gpio->gpio_config->gpio_pin >= 15 && _absl_gpio->gpio_config->gpio_pin < 31)
		{
			_absl_gpio->gpio_irq = GPIO2_Combined_16_31_IRQn;
			absl_gpio_irq_gpio2_16_31 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO3_BASE:
		if((_absl_gpio->gpio_config->gpio_pin) >= 0 && (_absl_gpio->gpio_config->gpio_pin < 15))
		{
			_absl_gpio->gpio_irq = GPIO3_Combined_0_15_IRQn;
			absl_gpio_irq_gpio3_0_15 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else if(_absl_gpio->gpio_config->gpio_pin >= 15 && _absl_gpio->gpio_config->gpio_pin < 31)
		{
			_absl_gpio->gpio_irq = GPIO3_Combined_16_31_IRQn;
			absl_gpio_irq_gpio3_16_31 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO4_BASE:
		if((_absl_gpio->gpio_config->gpio_pin) >= 0 && (_absl_gpio->gpio_config->gpio_pin < 15))
		{
			_absl_gpio->gpio_irq = GPIO4_Combined_0_15_IRQn;
			absl_gpio_irq_gpio4_0_15 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else if(_absl_gpio->gpio_config->gpio_pin >= 15 && _absl_gpio->gpio_config->gpio_pin < 31)
		{
			_absl_gpio->gpio_irq = GPIO4_Combined_16_31_IRQn;
			absl_gpio_irq_gpio4_16_31 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO5_BASE:
		if((_absl_gpio->gpio_config->gpio_pin) >= 0 && (_absl_gpio->gpio_config->gpio_pin < 15))
		{
			_absl_gpio->gpio_irq = GPIO5_Combined_0_15_IRQn;
			absl_gpio_irq_gpio5_0_15 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else if(_absl_gpio->gpio_config->gpio_pin >= 15 && _absl_gpio->gpio_config->gpio_pin < 31)
		{
			_absl_gpio->gpio_irq = GPIO5_Combined_16_31_IRQn;
			absl_gpio_irq_gpio5_16_31 = _absl_gpio;
			gpio_rv = ABSL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	default:
		gpio_rv = ABSL_GPIO_RV_INCORRECT_CONFIG;
		break;
	}

	if(ABSL_GPIO_RV_OK == gpio_rv)
	{
        absl_event_create(&_absl_gpio->gpio_event_group);
		/* Enable GPIO pin interrupt */
		NVIC_SetPriority(_absl_gpio->gpio_irq, (configMAX_SYSCALL_INTERRUPT_PRIORITY + 1) * 2);
		EnableIRQ(_absl_gpio->gpio_irq);
		GPIO_PortEnableInterrupts(_absl_gpio->gpio_config->gpio, 1U << _absl_gpio->gpio_config->gpio_pin);
	}

	return gpio_rv;
}

#endif /* ABSL_GPIO */
#endif
