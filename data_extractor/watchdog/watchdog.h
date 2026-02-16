/*
 * watchdog.h
 *
 *  Created on: Jan 19, 2023
 *      Author: abolinaga
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "absl_types.h"

#include "absl_event.h"
#include "absl_queue.h"


#define WATCHDOG_THREAD_WDG		0x00000001

typedef struct watchdog_thread_vars
{
	absl_event_t*  	watchdog_event_group;
	uint32_t		watchdog_event;
	absl_queue_t*		watchdog_queue;
	uint32_t		prev_consistency_counter;
	uint32_t		thread_no_answer_count;
	uint32_t		reset_magic_number;
}watchdog_thread_vars_t;

typedef struct watchdog_config_t
{
	uint8_t						system_wdog_index;
	absl_event_t*					watchdog_thread_event;
	uint32_t					thread_amount;
	watchdog_thread_vars_t*		watchdog_thread_vars;
	absl_queue_t*					watchdog_queue;
	bool						initialized;
}watchdog_config_t;

bool 			watchdog_init(watchdog_config_t* _wdog_config);
void 			watchdog_run(void);
void			watchdog_disable(void);
void 			watchdog_reset(void);

void watchdog_task(void* arg);

#endif /* WATCHDOG_H_ */
