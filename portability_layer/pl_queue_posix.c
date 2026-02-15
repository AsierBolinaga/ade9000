/*
 * pl_queue_freertos.c
 *
 *  Created on: 9 may. 2022
 *      Author: Asier Bolinaga
 */
#include "pl_queue.h"
#if  defined(PL_QUEUE) && defined(PL_LINUX)        

static uint32_t queue_count = 0;

#define QUEUE_PERMISSIONS 0666

pl_queue_rv_t pl_queue_create_posix(pl_queue_t* _pl_queue, uint32_t _item_size, uint32_t _max_item_length)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_ERROR;
	/* Initialize the queue attributes */
	struct mq_attr attr;
	char q_name[10];
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    sprintf(q_name, "/queue_%d", queue_count);

	attr.mq_flags = 0;
	attr.mq_maxmsg = _max_item_length;
	attr.mq_msgsize = _item_size;
	attr.mq_curmsgs = 0;

	_pl_queue->queue = mq_open(q_name, O_RDWR | O_CREAT, mode, &attr);
	if(-1 != _pl_queue->queue)
	{
		queue_count++;
		_pl_queue->item_size = _item_size;
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_send_posix(pl_queue_t* _pl_queue, void* _queue_item, uint32_t _try_ms)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_FULL;
	/* Todo. implemente retryes */
	if(-1 != mq_send (_pl_queue->queue, _queue_item, _pl_queue->item_size, 0))
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_send_fromISR_posix(pl_queue_t* _pl_queue, queue_item_t* _queue_item)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_ERROR;

	if(-1 != mq_send (_pl_queue->queue, _queue_item, _pl_queue->item_size, 0))
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

pl_queue_rv_t pl_queue_receive_posix(pl_queue_t* _pl_queue, void* _received_item, uint32_t _delay_ms)
{
	pl_queue_rv_t queue_rv = PL_QUEUE_RV_NO_ITEM;
	/* Todo. implemente retryes */
	if(-1 != mq_receive (_pl_queue->queue, _received_item, _pl_queue->item_size, NULL))
	{
		queue_rv = PL_QUEUE_RV_OK;
	}

	return queue_rv;
}

#endif /* PL_QUEUE && PL_INUX */
