//
//    http://www.gnu.org/licenses/lgpl-2.1.html
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA

/*
 *---------------------------------------------------------------------------
 *| VERSION | AUTHOR            | DATE       | NOTE                         |
 *---------------------------------------------------------------------------
 *| 01      | Fan Chunquan      | 2017-05-30 |                              |
 *---------------------------------------------------------------------------
 */

#ifndef FWK_TASK_QUEUE_H_
#define FWK_TASK_QUEUE_H_

#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* queue name maximum size */
#define FWK_QUEUE_NAME_MAX_LEN 12

typedef uint16_t fwk_queueID_t;

typedef enum
{
	FWK_TASK_QUEUE_E_PARAM = -1, /* invalid parameter */
	FWK_TASK_QUEUE_E_INTERNAL = -2, /* internal error */
	FWK_TASK_QUEUE_E_FULL = -3, /* queue is full */
	FWK_TASK_QUEUE_E_NODATA = -4, /* no data in the queue */
	FWK_TASK_QUEUE_E_BUFOVERFLOW = -5, /* buffer size too small */
} fwk_taskQueueErrorCode_t;

/*
 * Create queue with fixed size.
 * Input:
 * 	name: a unique friendly name for the queue
 * 	qID: designated queue ID (0 if leave it for system allocation)
 * 	depth: how many items the queue can hold
 * 	size: size of each item
 * Output:
 * 	qID: allocated queue ID if not specified
 * Return:
 *  queue ID if created successfully, the queue ID shall be a positive integer.
 * 	or error code defined with fwk_taskQueueErrorCode_t
 * 		FWK_TASK_QUEUE_E_PARAM
 * 		FWK_TASK_QUEUE_E_INTERNAL
 */
extern int fwk_createFixsizeQueue(fwk_queueID_t * qID, char * name,
		uint8_t depth, uint16_t size);

/*
 * Create queue for variable sized data.
 * Input:
 * 	name: a unique friendly name for the queue
 * 	qID: designated queue ID (0 if leave it for system allocation)
 * 	maxBufSize: total size of memory to buffer data
 * Output:
 * 	qID: allocated queue ID if not specified
 * Return:
 *  queue ID if created successfully, the queue ID shall be a positive integer.
 * 	or error code defined with fwk_taskQueueErrorCode_t
 * 		FWK_TASK_QUEUE_E_PARAM
 * 		FWK_TASK_QUEUE_E_INTERNAL
 */
extern int fwk_createVarSizeQueue(fwk_queueID_t * qID, char * name,
		uint16_t maxBufSize);

/*
 * Send a data item to queue.
 * Parameters:
 * 	qID: identify the queue
 * 	data: data to send
 * 	size: size of the data to send
 * 	timeout: ticks timeout
 * Return:
 * 	0 if succeed
 * 	or error code defined with fwk_taskQueueErrorCode_t
 * 		FWK_TASK_QUEUE_E_PARAM
 * 		FWK_TASK_QUEUE_E_INTERNAL
 * 		FWK_TASK_QUEUE_E_FULL
 */
extern int fwk_sendToQueue(fwk_queueID_t qID, void * data, uint16_t size,
		uint32_t timeout);

/*
 * Receive data from a queue.
 * Return:
 * 	valid data size if succeed
 * 	or error code defined with fwk_taskQueueErrorCode_t
 * 		FWK_TASK_QUEUE_E_PARAM
 * 		FWK_TASK_QUEUE_E_INTERNAL
 * 		FWK_TASK_QUEUE_E_NODATA
 * 		FWK_TASK_QUEUE_E_BUFOVERFLOW
 */
extern int fwk_receiveFromQueue(fwk_queueID_t qID, void * buffer,
		uint16_t bufsize, uint32_t timeout);

/*
 * Clear data in the queue
 */
extern int fwk_clearQueue(fwk_queueID_t qID);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_TASK_QUEUE_H_ */
