/*
 * pl_thread.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_thread.h"
#if defined(PL_THREAD)

#if defined(PL_LINUX)
#include <time.h>

static void *pl_thread_cb(void *_pvParameters)
{
	pl_thread_t* pl_thread = (pl_thread_t*)_pvParameters;

	pl_thread->pl_thread_entry(pl_thread->args);
}

void pl_thread_run_posix(void)
{
	pthread_exit(0);
}

void pl_thread_sleep(uint32_t _ms)
{
	usleep(_ms * 1000);
}

pl_thread_rv_t pl_thread_create_posix(pl_thread_t* _pl_thread, char* _pl_thread_name, pl_thread_entry_t _pl_thread_entry,
								      uint32_t _pl_thread_priority, uint32_t _pl_thread_stack_size, void* _args)
{
	pl_thread_rv_t pl_thread_rv = PL_THREAD_RV_ERROR;

	pthread_attr_t 	tattr;
	int 			ret;

	if(NULL != _pl_thread)
	{
		_pl_thread->pl_thread_entry = _pl_thread_entry;
		_pl_thread->pl_thread_name = _pl_thread_name;
		_pl_thread->pl_thread_priority = _pl_thread_priority;
		_pl_thread->pl_thread_stack_size = _pl_thread_stack_size;
		_pl_thread->args = _args;

		/* initialized with default attributes */
		pthread_attr_init (&tattr);
		
		pthread_attr_setstacksize(&tattr, _pl_thread->pl_thread_stack_size);

		if(!pthread_create(&_pl_thread->pthread, &tattr, pl_thread_cb, _pl_thread))
		{
			pthread_setschedprio(_pl_thread->pthread,_pl_thread->pl_thread_priority);
			pl_thread_rv = PL_THREAD_RV_OK;
		}
	}

	return pl_thread_rv;
}

#endif /* PL_THREAD */
#endif
