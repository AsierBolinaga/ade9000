/**
 * @file led_handler.h
 * @brief LED handler interface for managing LED blinking patterns based on system state.
 *
 * This module defines the interface for controlling LED blinking patterns based on system states.
 * It provides functionality to initialize the LED handler and execute tasks related to blinking patterns.
 *
 * @date Created on: Sep 12, 2023
 * @author abolinaga
 */

#ifndef LED_HANDLER_H_
#define LED_HANDLER_H_

#include "absl_types.h"

#include "absl_gpio.h"
#include "absl_timer.h"
#include "absl_event.h"

/******************************************************************************
 * Type definitions
 ******************************************************************************/


/**
 * @enum led_state_t
 * @brief Represents the ON/OFF state of the LED.
 *
 * This enum defines the two possible states of the LED: ON and OFF.
 */
typedef enum led_state
{
    LED_ON = 0,  ///< LED is turned on
    LED_OFF      ///< LED is turned off
} led_state_t;


/**
 * @struct toggle_state_t
 * @brief Represents a single state in a LED blinking pattern.
 *
 * This structure defines a single LED state, which includes the desired state (ON or OFF)
 * and the duration for which the LED remains in that state.
 */
typedef struct toggle_state
{
    led_state_t led_state;     ///< Desired LED state
    uint32_t    state_time_ms; ///< Duration of this state in milliseconds
} toggle_state_t;

/**
 * @struct led_pattern_t
 * @brief Represents a full LED blinking pattern consisting of multiple toggle states.
 *
 * This structure defines an entire LED blinking pattern, which consists of multiple toggle states.
 * Each toggle state describes the LED state (ON or OFF) and the duration for which the LED stays in that state.
 */
typedef struct led_pattern
{
    toggle_state_t* toggle_state_table; ///< Pointer to array of toggle states
    uint32_t        toggle_amount;      ///< Number of toggle states in the pattern
    uint32_t        pattern_position;   ///< Current position in the toggle state sequence
} led_pattern_t;


/**
 * @struct led_thread_data_t
 * @brief LED data structure passed to the LED handler during initialization.
 *
 * This structure contains the data necessary for the LED handler to function, such as the GPIO object,
 * the timer for toggling the LED, the event for triggering toggles, and the currently active LED pattern.
 */
typedef struct led_thread_data
{
	absl_gpio_t		status_led_user_gpio;				///< GPIO object for the status LED
	absl_timer_t      led_toggle_timer;			 		///< Timer used to toggle the LED
	absl_event_t      led_toggle_event;			 		///< Event used to trigger LED toggle
	led_pattern_t*  led_pattern;				 		///< Currently active LED blinking pattern
}led_thread_data_t;

/**
 * @struct led_thread_config_t
 * @brief Configuration structure passed to the LED handler during initialization.
 *
 * This structure holds configuration data for the LED handler, such as GPIO indices and the pattern table.
 * The pattern table contains a list of LED blinking patterns for different system states.
 */
typedef struct led_thread_config
{
    uint8_t         led_gpio_user_index;         ///< Index for user LED GPIO
    uint8_t         led_gpio_user_enable_index;  ///< Index for enable GPIO
    uint8_t* 		system_state;				 ///< Pointer to  system state
    led_pattern_t*  pattern_table;               ///< Pointer to array of patterns (one per system state)
    bool            led_handler_initialized;     ///< Flag indicating if handler has been initialized
} led_thread_config_t;

/**
 * @struct led_thread
 * @brief Structure that combines both configuration and data for the LED thread.
 *
 * This structure combines the configuration data and thread data for a single LED handler instance.
 * It is passed as an argument to initialize and manage the LED handler.
 */
typedef struct led_thread
{
	led_thread_data_t*		led_thread_data;
	led_thread_config_t* 	led_thread_config;
}led_thread_t;


/**
 * @brief Initializes the LED handler module.
 *
 * This function initializes the LED handler module by setting up GPIO, events, and timers.
 * It configures the necessary hardware resources to control the LED according to the system's state.
 *
 * @param _led_thread Pointer to the LED thread structure containing configuration and data.
 * @return true if initialization was successful, false otherwise.
 */
bool led_handler_initialize(led_thread_t* _led_thread);

/**
 * @brief LED handler task.
 *
 * This task continuously updates the LED state based on the system's current state pattern.
 * It waits for toggle events and updates the LED accordingly, following the defined blinking pattern.
 *
 * @param arg Pointer to a uint8_t indicating the current system state.
 */
void led_handler_task(void* arg);

#endif /* LED_HANDLER_H_ */
