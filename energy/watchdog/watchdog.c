
#include "watchdog.h"

#include "absl_system.h"
#include "absl_hw_config.h"
#include "absl_watchdog.h"
#include "absl_debug.h"
#include "absl_thread.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static watchdog_config_t* wdog_config;

static absl_watchdog_t			absl_wdog;
static absl_watchdog_config_t * 	absl_wdog_config;

/*******************************************************************************
 * Code
 ******************************************************************************/
bool watchdog_init(watchdog_config_t* _wdog_config)
{
	bool return_value = false;

	if(NULL != _wdog_config)
	{
		wdog_config = _wdog_config;

		absl_wdog_config = absl_config_get_wdog_conf(wdog_config->wdog_index);

		if(ABSL_WATCHDOG_OK == absl_watchdog_init(&absl_wdog, absl_wdog_config))
		{
			wdog_config->initialized = true;
			return_value = true;
			absl_debug_printf("--- wdog Init done---\r\n");
		}
	}

	return return_value;
}

void watchdog_run(void)
{
    if(!wdog_config->initialized)
    {
    	absl_debug_printf("watchdog module not initialized!\n");
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

