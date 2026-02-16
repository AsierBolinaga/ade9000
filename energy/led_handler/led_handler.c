/**
 * @file led_handler.c
 * @brief Module responsible for managing the system's status LED through blinking patterns.
 *
 * This module is responsible for controlling the status LED of the system based on predefined blinking patterns.
 * It provides functionality to toggle the LED state, manage patterns, and initialize hardware components necessary
 * for LED control, including GPIO, timers, and events.
 *
 * @date Created on: Sep 12, 2023
 * @author abolinaga
 */

#include "led_handler.h"

#include "absl_system.h"
#include "absl_debug.h"
#include "absl_macros.h"

#include "absl_hw_config.h"

/// Event flag used to indicate LED toggle
#define LED_HANDLER_TOGGLE_EVENT    0x00000001

/**
 * @brief Toggles the LED state based on input.
 *
 * This function sets the LED to the desired state (ON or OFF) by controlling the GPIO.
 *
 * @param _gpio Pointer to the GPIO object for the LED.
 * @param _led_state Desired state of the LED (LED_ON or LED_OFF).
 */
static void led_handler_toggle(absl_gpio_t* _gpio, led_state_t _led_state);

/**
 * @brief Converts a time in milliseconds to a absl_time_t structure.
 *
 * This function takes a time duration in milliseconds and converts it into the corresponding
 * `absl_time_t` structure, which separates seconds and nanoseconds.
 *
 * @param _toggle_time_ms Time duration in milliseconds.
 * @return Equivalent `absl_time_t` structure.
 */
static absl_time_t led_handler_get_next_toggle_time(uint32_t _toggle_time_ms);

/**
 * @brief Event callback that sets the LED toggle event flag.
 *
 * This function is used as a callback to set the LED toggle event, which is triggered
 * when the LED needs to change its state.
 *
 * @param arg Unused argument.
 */
void led_handler_toggle_event(void* _arg)
{
	absl_event_t* toggle_event = (absl_event_t* )_arg;

	absl_event_set(toggle_event, LED_HANDLER_TOGGLE_EVENT);
}

/**
 * @brief Initializes the LED handler module.
 *
 * This function initializes the necessary components for controlling the LED, including
 * GPIO, event flags, and timers. If the initialization is successful, the LED handler is
 * ready to use for toggling the LED based on patterns.
 *
 * @param _led_thread Pointer to the LED thread structure containing configuration and data.
 * @return true if initialization was successful, false otherwise.
 */
bool led_handler_initialize(led_thread_t* _led_thread)
{
	bool return_value = false;

	absl_time_t toggle_time;

    if(NULL != _led_thread)
    {
    	led_thread_config_t* 	led_thread_config = _led_thread->led_thread_config;
		led_thread_data_t*		led_thread_data	  = _led_thread->led_thread_data;
		if(NULL != (led_thread_config) && (NULL != led_thread_data))
		{
			absl_gpio_config_t* status_led_user_config = absl_config_get_gpio_conf(_led_thread->led_thread_config->led_gpio_user_index);

			if((ABSL_GPIO_RV_OK == absl_gpio_init(&_led_thread->led_thread_data->status_led_user_gpio, status_led_user_config, ABSL_GPIO_NO_INT)) &&
			   (ABSL_EVENT_RV_OK == absl_event_create(&_led_thread->led_thread_data->led_toggle_event)))
			{
				absl_gpio_on(&_led_thread->led_thread_data->status_led_user_gpio);

				led_thread_data->led_pattern = &led_thread_config->pattern_table[*led_thread_config->system_state];

				led_handler_toggle(&led_thread_data->status_led_user_gpio,
								   led_thread_data->led_pattern->toggle_state_table[led_thread_data->led_pattern->pattern_position].led_state);

				toggle_time = led_handler_get_next_toggle_time(led_thread_data->led_pattern->toggle_state_table[led_thread_data->led_pattern->pattern_position].state_time_ms);

				if(ABSL_TIMER_RV_ERROR != absl_timer_create(&_led_thread->led_thread_data->led_toggle_timer, &led_handler_toggle_event,
														&_led_thread->led_thread_data->led_toggle_event, toggle_time, false, true))
				{
					_led_thread->led_thread_config->led_handler_initialized = true;
					return_value = true;
				}
			}
		}
    }

    return return_value;
}


/**
 * @brief Main task that handles the LED blinking pattern.
 *
 * This task continuously manages the LED state based on the system's current state pattern.
 * It listens for events and toggles the LED according to the pattern, updating the state at the
 * specified time intervals.
 *
 * @param arg Pointer to a uint8_t indicating the current system state.
 */
void led_handler_task(void* arg)
{
	led_thread_t* led_thread = (led_thread_t*)arg;

	led_thread_config_t* 	led_thread_config = led_thread->led_thread_config;
	led_thread_data_t*		led_thread_data	  = led_thread->led_thread_data;

    uint32_t    event_flags;
    absl_time_t   toggle_time;

	uint8_t* system_state = led_thread_config->system_state;

    if(!led_thread_config->led_handler_initialized)
	{
		absl_debug_printf("LED handler task was not initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

    // Main loop to wait for events and continue toggling the LED based on the pattern
    while(1)
    {
        if(ABSL_EVENT_RV_OK == absl_event_wait(&led_thread_data->led_toggle_event, LED_HANDLER_TOGGLE_EVENT, &event_flags))
        {
        	led_thread_data->led_pattern = &led_thread_config->pattern_table[*system_state];

        	// Update the pattern position and toggle the LED
        	led_thread_data->led_pattern->pattern_position = ABSL_INC_INDEX(led_thread_data->led_pattern->pattern_position,
        																  led_thread_data->led_pattern->toggle_amount);

            led_handler_toggle(&led_thread_data->status_led_user_gpio,
            				   led_thread_data->led_pattern->toggle_state_table[led_thread_data->led_pattern->pattern_position].led_state);

            // Reset the timer for the next toggle
            toggle_time = led_handler_get_next_toggle_time(led_thread_data->led_pattern->toggle_state_table[led_thread_data->led_pattern->pattern_position].state_time_ms);

            absl_timer_change(&led_thread_data->led_toggle_timer, toggle_time, true);
        }
    }
}


/**
 * @brief Updates the LED GPIO based on the given state.
 *
 * This function updates the GPIO state of the LED according to the desired LED state
 * (ON or OFF).
 *
 * @param _gpio Pointer to the GPIO object for the LED.
 * @param _led_state Desired state of the LED (ON or OFF).
 */
static void led_handler_toggle(absl_gpio_t* _gpio, led_state_t _led_state)
{
    if(LED_ON == _led_state)
    {
        absl_gpio_on(_gpio);
    }
    else
    {
        absl_gpio_off(_gpio);
    }
}

/**
 * @brief Converts a time in milliseconds to a absl_time_t structure.
 *
 * This function takes a time duration in milliseconds and converts it into a structure
 * that separates the time into seconds and nanoseconds, suitable for the timer functions.
 *
 * @param _toggle_time_ms Time duration in milliseconds.
 * @return A `absl_time_t` structure with equivalent time.
 */
static absl_time_t led_handler_get_next_toggle_time(uint32_t _toggle_time_ms)
{
    absl_time_t next_time;

    uint32_t actual_toggle_time = _toggle_time_ms;

    next_time.seconds = 0;
    while(actual_toggle_time >= 1000)
    {
        next_time.seconds++;
        actual_toggle_time -= 1000;
    }
    next_time.nseconds =  actual_toggle_time * 1000000;

    return next_time;
}
