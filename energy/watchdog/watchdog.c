
#include "watchdog.h"

#include "pl_system.h"
#include "pl_hw_config.h"
#include "pl_watchdog.h"
#include "pl_debug.h"
#include "pl_thread.h"

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

static pl_watchdog_t			pl_wdog;
static pl_watchdog_config_t * 	pl_wdog_config;

/*******************************************************************************
 * Code
 ******************************************************************************/
bool watchdog_init(watchdog_config_t* _wdog_config)
{
	bool return_value = false;

	if(NULL != _wdog_config)
	{
		wdog_config = _wdog_config;

		pl_wdog_config = pl_config_get_wdog_conf(wdog_config->wdog_index);

		if(PL_WATCHDOG_OK == pl_watchdog_init(&pl_wdog, pl_wdog_config))
		{
			wdog_config->initialized = true;
			return_value = true;
			pl_debug_printf("--- wdog Init done---\r\n");
		}
	}

	return return_value;
}

void watchdog_run(void)
{
    if(!wdog_config->initialized)
    {
    	pl_debug_printf("watchdog module not initialized!\n");
		pl_hardfault_handler(WDOG_NOT_INIT_ERROR);
    }

	pl_watchdog_run(&pl_wdog);
}

void watchdog_disable(void)
{
	WDOG_Disable(WDOG1);
}

void watchdog_reset(void)
{
	pl_debug_printf("watchdog software reset triggered!\n");
	pl_thread_sleep(500);
	pl_watchdog_sw_reset(&pl_wdog);
}

