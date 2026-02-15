/*
 * watchdog.h
 *
 *  Created on: Jan 19, 2023
 *      Author: abolinaga
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "pl_types.h"

typedef struct watchdog_config_t
{
	uint8_t		wdog_index;
	bool		initialized;
}watchdog_config_t;

bool 			watchdog_init(watchdog_config_t* _wdog_config);
void 			watchdog_run(void);
void			watchdog_disable(void);
void 			watchdog_reset(void);

#endif /* WATCHDOG_H_ */
