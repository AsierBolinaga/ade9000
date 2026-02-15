/*
 * pl_gpio_imxrt10xx.c
 *
 *  Created on: 3 may. 2022
 *      Author: Asier Bolinaga
 */

#include "pl_gpio.h"
#ifdef PL_GPIO

#if defined(PL_IMX_RT10XX)
#include "pin_mux.h"
#include "board.h"

#define pl_gpio_1_Combined_0_15_IRQHandler 		GPIO1_Combined_0_15_IRQHandler
#define pl_gpio_1_Combined_16_31_IRQHandler 	GPIO1_Combined_16_31_IRQHandler
#define pl_gpio_2_Combined_0_15_IRQHandler 		GPIO2_Combined_0_15_IRQHandler
#define pl_gpio_2_Combined_16_31_IRQHandler 	GPIO2_Combined_16_31_IRQHandler
#define pl_gpio_3_Combined_0_15_IRQHandler 		GPIO3_Combined_0_15_IRQHandler
#define pl_gpio_3_Combined_16_31_IRQHandler 	GPIO3_Combined_16_31_IRQHandler
#define pl_gpio_4_Combined_0_15_IRQHandler 		GPIO4_Combined_0_15_IRQHandler
#define pl_gpio_4_Combined_16_31_IRQHandler 	GPIO4_Combined_16_31_IRQHandler
#define pl_gpio_5_Combined_0_15_IRQHandler 		GPIO5_Combined_0_15_IRQHandler
#define pl_gpio_5_Combined_16_31_IRQHandler 	GPIO5_Combined_16_31_IRQHandler

static pl_gpio_t* pl_gpio_irq_gpio1_0_15;
static pl_gpio_t* pl_gpio_irq_gpio1_16_31;
static pl_gpio_t* pl_gpio_irq_gpio2_0_15;
static pl_gpio_t* pl_gpio_irq_gpio2_16_31;
static pl_gpio_t* pl_gpio_irq_gpio3_0_15;
static pl_gpio_t* pl_gpio_irq_gpio3_16_31;
static pl_gpio_t* pl_gpio_irq_gpio4_0_15;
static pl_gpio_t* pl_gpio_irq_gpio4_16_31;
static pl_gpio_t* pl_gpio_irq_gpio5_0_15;
static pl_gpio_t* pl_gpio_irq_gpio5_16_31;

static pl_gpio_rv_t pl_gpio_init_irq_imxrt10xx(pl_gpio_t* _pl_gpio);

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_1_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio1_0_15->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio1_0_15->gpio_config->gpio_pin);

	pl_event_set_fromISR(&pl_gpio_irq_gpio1_0_15->gpio_event_group, pl_gpio_irq_gpio1_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_1_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio1_16_31->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio1_16_31->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio1_16_31->gpio_event_group, pl_gpio_irq_gpio1_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_2_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio2_0_15->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio2_0_15->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio2_0_15->gpio_event_group, pl_gpio_irq_gpio2_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_2_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio2_16_31->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio2_16_31->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio2_16_31->gpio_event_group, pl_gpio_irq_gpio2_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_3_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio3_0_15->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio3_0_15->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio3_0_15->gpio_event_group, pl_gpio_irq_gpio3_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_3_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio3_16_31->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio3_16_31->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio3_16_31->gpio_event_group, pl_gpio_irq_gpio3_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_4_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio4_0_15->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio4_0_15->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio4_0_15->gpio_event_group, pl_gpio_irq_gpio4_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_4_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio4_16_31->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio4_16_31->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio4_16_31->gpio_event_group, pl_gpio_irq_gpio4_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_5_Combined_0_15_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio5_0_15->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio5_0_15->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio5_0_15->gpio_event_group, pl_gpio_irq_gpio5_0_15->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Interrupt service function of switch.
 */
void pl_gpio_5_Combined_16_31_IRQHandler(void)
{
	/* clear the interrupt status */
    GPIO_PortClearInterruptFlags(pl_gpio_irq_gpio5_16_31->gpio_config->gpio,
    							 1U << pl_gpio_irq_gpio5_16_31->gpio_config->gpio_pin);

    pl_event_set_fromISR(&pl_gpio_irq_gpio5_16_31->gpio_event_group, pl_gpio_irq_gpio5_16_31->gpio_event);
    SDK_ISR_EXIT_BARRIER;
}


pl_gpio_rv_t pl_gpio_init_imxrt10xx(pl_gpio_t* _pl_gpio, pl_gpio_config_t* _gpio_config, pl_gpio_int_mode_t _int_mode)
{
	pl_gpio_rv_t gpio_rv = PL_GPIO_RV_ERROR;

	if(NULL != _gpio_config)
	{
		_pl_gpio->gpio_config = _gpio_config;

		/* Define the init structure for the input switch pin */
		gpio_pin_config_t sw_config;
		sw_config.direction = _gpio_config->direction;
		sw_config.outputLogic = _gpio_config->outputLogic;
		switch(_int_mode)
		{
		case PL_GPIO_HIGH_LEVEL:
			sw_config.interruptMode = kGPIO_IntHighLevel;
			break;
		case PL_GPIO_LOW_LEVEL:
			sw_config.interruptMode = kGPIO_IntLowLevel;
			break;
		case PL_GPIO_RISING_EDGE:
			sw_config.interruptMode = kGPIO_IntRisingEdge;
			break;
		case PL_GPIO_FALLING_EDGE:
			sw_config.interruptMode = kGPIO_IntFallingEdge;
			break;
		case PL_GPIO_FALLING_RISING_EDGE:
			sw_config.interruptMode = kGPIO_IntRisingOrFallingEdge;
			break;
		default:
			sw_config.interruptMode = kGPIO_NoIntmode;
			break;
		}
		GPIO_PinInit(_gpio_config->gpio, _gpio_config->gpio_pin, &sw_config);

		if(PL_GPIO_NO_INT != _int_mode)
		{
			_pl_gpio->gpio_event = _int_mode;
			gpio_rv = pl_gpio_init_irq_imxrt10xx(_pl_gpio);
		}
		else
		{
			gpio_rv = PL_GPIO_RV_OK;
		}
	}
	else
	{
		/* No configuration */
		gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
	}

	return gpio_rv;
}

void pl_gpio_on_imxrt10xx(pl_gpio_t* _pl_gpio)
{
	GPIO_PinWrite(_pl_gpio->gpio_config->gpio, _pl_gpio->gpio_config->gpio_pin, 1U);
}

void pl_gpio_off_imxrt10xx(pl_gpio_t* _pl_gpio)
{
	GPIO_PinWrite(_pl_gpio->gpio_config->gpio, _pl_gpio->gpio_config->gpio_pin, 0U);
}

void pl_gpio_toggle_imxrt10xx(pl_gpio_t* _pl_gpio)
{
	GPIO_PortToggle(_pl_gpio->gpio_config->gpio, 1u << _pl_gpio->gpio_config->gpio_pin);
}

pl_gpio_pin_state_t pl_gpio_get_imxrt10xx(pl_gpio_t* _pl_gpio)
{
	if(GPIO_PinRead(_pl_gpio->gpio_config->gpio, _pl_gpio->gpio_config->gpio_pin))
	{
		return PL_GPIO_PIN_ON;
	}
	else
	{
		return PL_GPIO_PIN_OFF;
	}
}

static pl_gpio_rv_t pl_gpio_init_irq_imxrt10xx(pl_gpio_t* _pl_gpio)
{
	pl_gpio_rv_t gpio_rv = PL_GPIO_RV_ERROR;

	switch((uint32_t)_pl_gpio->gpio_config->gpio)
	{
	case GPIO1_BASE:
		if((_pl_gpio->gpio_config->gpio_pin) >= 0 && (_pl_gpio->gpio_config->gpio_pin < 15))
		{
			_pl_gpio->gpio_irq = GPIO1_Combined_0_15_IRQn;
			pl_gpio_irq_gpio1_0_15 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else if(_pl_gpio->gpio_config->gpio_pin >= 15 && _pl_gpio->gpio_config->gpio_pin < 31)
		{
			_pl_gpio->gpio_irq = GPIO1_Combined_16_31_IRQn;
			pl_gpio_irq_gpio1_16_31 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO2_BASE:
		if((_pl_gpio->gpio_config->gpio_pin) >= 0 && (_pl_gpio->gpio_config->gpio_pin < 15))
		{
			_pl_gpio->gpio_irq = GPIO2_Combined_0_15_IRQn;
			pl_gpio_irq_gpio2_0_15 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else if(_pl_gpio->gpio_config->gpio_pin >= 15 && _pl_gpio->gpio_config->gpio_pin < 31)
		{
			_pl_gpio->gpio_irq = GPIO2_Combined_16_31_IRQn;
			pl_gpio_irq_gpio2_16_31 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO3_BASE:
		if((_pl_gpio->gpio_config->gpio_pin) >= 0 && (_pl_gpio->gpio_config->gpio_pin < 15))
		{
			_pl_gpio->gpio_irq = GPIO3_Combined_0_15_IRQn;
			pl_gpio_irq_gpio3_0_15 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else if(_pl_gpio->gpio_config->gpio_pin >= 15 && _pl_gpio->gpio_config->gpio_pin < 31)
		{
			_pl_gpio->gpio_irq = GPIO3_Combined_16_31_IRQn;
			pl_gpio_irq_gpio3_16_31 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO4_BASE:
		if((_pl_gpio->gpio_config->gpio_pin) >= 0 && (_pl_gpio->gpio_config->gpio_pin < 15))
		{
			_pl_gpio->gpio_irq = GPIO4_Combined_0_15_IRQn;
			pl_gpio_irq_gpio4_0_15 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else if(_pl_gpio->gpio_config->gpio_pin >= 15 && _pl_gpio->gpio_config->gpio_pin < 31)
		{
			_pl_gpio->gpio_irq = GPIO4_Combined_16_31_IRQn;
			pl_gpio_irq_gpio4_16_31 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	case GPIO5_BASE:
		if((_pl_gpio->gpio_config->gpio_pin) >= 0 && (_pl_gpio->gpio_config->gpio_pin < 15))
		{
			_pl_gpio->gpio_irq = GPIO5_Combined_0_15_IRQn;
			pl_gpio_irq_gpio5_0_15 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else if(_pl_gpio->gpio_config->gpio_pin >= 15 && _pl_gpio->gpio_config->gpio_pin < 31)
		{
			_pl_gpio->gpio_irq = GPIO5_Combined_16_31_IRQn;
			pl_gpio_irq_gpio5_16_31 = _pl_gpio;
			gpio_rv = PL_GPIO_RV_OK;
		}
		else
		{
			gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		}
		break;
	default:
		gpio_rv = PL_GPIO_RV_INCORRECT_CONFIG;
		break;
	}

	if(PL_GPIO_RV_OK == gpio_rv)
	{
        pl_event_create(&_pl_gpio->gpio_event_group);
		/* Enable GPIO pin interrupt */
		NVIC_SetPriority(_pl_gpio->gpio_irq, (configMAX_SYSCALL_INTERRUPT_PRIORITY + 1) * 2);
		EnableIRQ(_pl_gpio->gpio_irq);
		GPIO_PortEnableInterrupts(_pl_gpio->gpio_config->gpio, 1U << _pl_gpio->gpio_config->gpio_pin);
	}

	return gpio_rv;
}

#endif /* PL_GPIO */
#endif
