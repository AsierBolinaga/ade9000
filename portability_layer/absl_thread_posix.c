/*
 * absl_thread.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_thread.h"
#if defined(ABSL_THREAD)

#if defined(ABSL_LINUX)
#include <time.h>

static void *absl_thread_cb(void *_pvParameters)
{
	absl_thread_t* absl_thread = (absl_thread_t*)_pvParameters;

	absl_thread->absl_thread_entry(absl_thread->args);
}

void absl_thread_run_posix(void)
{
	pthread_exit(0);
}

void absl_thread_sleep(uint32_t _ms)
{
	usleep(_ms * 1000);
}

absl_thread_rv_t absl_thread_create_posix(absl_thread_t* _absl_thread, char* _absl_thread_name, absl_thread_entry_t _absl_thread_entry,
								      uint32_t _absl_thread_priority, uint32_t _absl_thread_stack_size, void* _args)
{
	absl_thread_rv_t absl_thread_rv = ABSL_THREAD_RV_ERROR;

	pthread_attr_t 	tattr;
	int 			ret;

	if(NULL != _absl_thread)
	{
		_absl_thread->absl_thread_entry = _absl_thread_entry;
		_absl_thread->absl_thread_name = _absl_thread_name;
		_absl_thread->absl_thread_priority = _absl_thread_priority;
		_absl_thread->absl_thread_stack_size = _absl_thread_stack_size;
		_absl_thread->args = _args;

		/* initialized with default attributes */
		pthread_attr_init (&tattr);
		
		pthread_attr_setstacksize(&tattr, _absl_thread->absl_thread_stack_size);

		if(!pthread_create(&_absl_thread->pthread, &tattr, absl_thread_cb, _absl_thread))
		{
			pthread_setschedprio(_absl_thread->pthread,_absl_thread->absl_thread_priority);
			absl_thread_rv = ABSL_THREAD_RV_OK;
		}
	}

	return absl_thread_rv;
}

#endif /* ABSL_THREAD */
#endif
