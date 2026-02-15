/*
 * pl_thread.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_thread.h"
#if defined(PL_THREAD)
#if defined(PL_OS_FREE_RTOS)
#include "task.h"

static void pl_thread_cb(void *_pvParameters)
{
	pl_thread_t* pl_thread = (pl_thread_t*)_pvParameters;

	pl_thread->pl_thread_entry(pl_thread->args);
}

void pl_thread_run_freertos(void)
{
	vTaskStartScheduler();
}

void pl_thread_sleep(uint32_t _ms)
{
	vTaskDelay(_ms / portTICK_PERIOD_MS);
}

pl_thread_rv_t pl_thread_create_freertos(pl_thread_t* _pl_thread, char* _pl_thread_name, pl_thread_entry_t _pl_thread_entry,
		uint32_t _pl_thread_priority, uint32_t _pl_thread_stack_size, void* _args)
{
	pl_thread_rv_t pl_thread_rv = PL_THREAD_RV_ERROR;

	_pl_thread->pl_thread_name = _pl_thread_name;
	_pl_thread->pl_thread_priority = _pl_thread_priority;
	/* stack size to give in freertos is the number of variables, not bytes.
	 * 32bits stack, for each 100bits, 400 will be reserved. */
	_pl_thread->pl_thread_stack_size = _pl_thread_stack_size / sizeof( StackType_t );
	_pl_thread->pl_thread_entry = _pl_thread_entry;
	_pl_thread->args = _args;

	if (xTaskCreate(pl_thread_cb, _pl_thread_name, _pl_thread->pl_thread_stack_size,
					_pl_thread, _pl_thread_priority, &_pl_thread->task_handle) == pdPASS)
	{
		pl_thread_rv = PL_THREAD_RV_OK;
	}

	return pl_thread_rv;
}

void pl_thread_delete_freertos(pl_thread_t* _pl_thread)
{
	vTaskDelete(_pl_thread->task_handle);
}

void pl_thread_actual_delete_freertos(void)
{
	vTaskDelete(NULL);
}

#endif
#endif
