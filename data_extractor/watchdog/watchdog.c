
#include "watchdog.h"

#include "absl_system.h"
#include "absl_hw_config.h"
#include "absl_watchdog.h"
#include "absl_debug.h"
#include "absl_thread.h"
#include "absl_macros.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define WATCHDOG_THREAD_NO_ANSWER_MAX_AMOUNT	5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static watchdog_config_t* wdog_config;

static absl_watchdog_t			absl_wdog;
static absl_watchdog_config_t * 	absl_wdog_config;

static uint32_t consistency_counter;

static absl_timer_t	watchdog_timer;
static uint32_t		thread_index;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void watchdog_thread_check_cb(void* arg);
static void watchdog_possible_blocked_thread(watchdog_thread_vars_t* _watchdog_thread_vars);

bool watchdog_init(watchdog_config_t* _wdog_config)
{
	bool return_value = false;

	absl_time_t watchdog_time;

	if(NULL != _wdog_config)
	{
		wdog_config = _wdog_config;

		absl_wdog_config = absl_config_get_wdog_conf(wdog_config->system_wdog_index);

		if((ABSL_WATCHDOG_OK == absl_watchdog_init(&absl_wdog, absl_wdog_config)) &&
		   (ABSL_EVENT_RV_OK == absl_event_create(wdog_config->watchdog_thread_event)) &&
		   (ABSL_QUEUE_RV_OK == absl_queue_create(wdog_config->watchdog_queue, sizeof(consistency_counter), 1)))
		{
			watchdog_time.seconds  = 1;
			watchdog_time.nseconds = 0;
			absl_timer_create(&watchdog_timer, &watchdog_thread_check_cb, _wdog_config, watchdog_time, true, false);

			thread_index = 0;

			consistency_counter = 0;

			wdog_config->initialized = true;
			return_value = true;
		}
	}

	return return_value;
}

void watchdog_run(void)
{
    if(!wdog_config->initialized)
    {
		absl_hardfault_handler(WDOG_NOT_INIT_ERROR);
    }

	absl_watchdog_run(&absl_wdog);
}

void watchdog_disable(void)
{
	WDOG_Disable(WDOG1);
}

void watchdog_reset(void)
{
	absl_debug_printf("watchdog software reset triggered!\n");
	absl_thread_sleep(500);
	absl_watchdog_sw_reset(&absl_wdog);
}

void watchdog_task(void* arg)
{
	ABSL_UNUSED_ARG(arg);

	uint32_t	recieved_events;

	if(!wdog_config->initialized)
	{
		absl_debug_printf("ERROR! watchdog thread has not been initialized!\n");
		absl_hardfault_handler(THREAD_NOT_INIT_ERROR);
	}

	absl_timer_start(&watchdog_timer);

	while (1)
	{
		if(ABSL_EVENT_RV_OK == absl_event_wait(wdog_config->watchdog_thread_event, WATCHDOG_THREAD_WDG, &recieved_events))
		{
			if(WATCHDOG_THREAD_WDG == (recieved_events & WATCHDOG_THREAD_WDG))
			{
				consistency_counter++;
				absl_queue_send(wdog_config->watchdog_queue, (void*)&consistency_counter, 1);
			}
		}
	}
}

static void watchdog_thread_check_cb(void* arg)
{
	watchdog_config_t* wdog_config = (watchdog_config_t*)arg;
	watchdog_thread_vars_t* watchdog_thread_vars = &wdog_config->watchdog_thread_vars[thread_index];

	uint32_t new_consistency_counter;

	if(ABSL_QUEUE_RV_OK == absl_queue_receive_fromISR(watchdog_thread_vars->watchdog_queue, (void*)&new_consistency_counter))
	{
		if(new_consistency_counter != watchdog_thread_vars->prev_consistency_counter)
		{
			watchdog_thread_vars->prev_consistency_counter = new_consistency_counter;
		}
		else
		{
			watchdog_possible_blocked_thread(watchdog_thread_vars);
		}
	}
	else
	{
		watchdog_possible_blocked_thread(watchdog_thread_vars);
	}

	absl_event_set_fromISR(watchdog_thread_vars->watchdog_event_group, watchdog_thread_vars->watchdog_event);

	thread_index = ABSL_INC_INDEX(thread_index, wdog_config->thread_amount);
}

static void watchdog_possible_blocked_thread(watchdog_thread_vars_t* _watchdog_thread_vars)
{
	_watchdog_thread_vars->thread_no_answer_count++;
	if(_watchdog_thread_vars->thread_no_answer_count >= WATCHDOG_THREAD_NO_ANSWER_MAX_AMOUNT)
	{
		absl_hardfault_handler(_watchdog_thread_vars->reset_magic_number);
	}
}


