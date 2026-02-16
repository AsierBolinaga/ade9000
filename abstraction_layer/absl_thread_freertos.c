/*
 * absl_thread.c
 *
 *  Created on: 28 ene. 2022
 *      Author: Asier Bolinaga
 */
#include "absl_thread.h"
#if defined(ABSL_THREAD)
#if defined(ABSL_OS_FREE_RTOS)
#include "task.h"

static void absl_thread_cb(void *_pvParameters)
{
	absl_thread_t* absl_thread = (absl_thread_t*)_pvParameters;

	absl_thread->absl_thread_entry(absl_thread->args);
}

void absl_thread_run_freertos(void)
{
	vTaskStartScheduler();
}

void absl_thread_sleep(uint32_t _ms)
{
	vTaskDelay(_ms / portTICK_PERIOD_MS);
}

absl_thread_rv_t absl_thread_create_freertos(absl_thread_t* _absl_thread, char* _absl_thread_name, absl_thread_entry_t _absl_thread_entry,
		uint32_t _absl_thread_priority, uint32_t _absl_thread_stack_size, void* _args)
{
	absl_thread_rv_t absl_thread_rv = ABSL_THREAD_RV_ERROR;

	_absl_thread->absl_thread_name = _absl_thread_name;
	_absl_thread->absl_thread_priority = _absl_thread_priority;
	/* stack size to give in freertos is the number of variables, not bytes.
	 * 32bits stack, for each 100bits, 400 will be reserved. */
	_absl_thread->absl_thread_stack_size = _absl_thread_stack_size / sizeof( StackType_t );
	_absl_thread->absl_thread_entry = _absl_thread_entry;
	_absl_thread->args = _args;

	if (xTaskCreate(absl_thread_cb, _absl_thread_name, _absl_thread->absl_thread_stack_size,
					_absl_thread, _absl_thread_priority, &_absl_thread->task_handle) == pdPASS)
	{
		absl_thread_rv = ABSL_THREAD_RV_OK;
	}

	return absl_thread_rv;
}

void absl_thread_delete_freertos(absl_thread_t* _absl_thread)
{
	vTaskDelete(_absl_thread->task_handle);
}

void absl_thread_actual_delete_freertos(void)
{
	vTaskDelete(NULL);
}

#endif
#endif
