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

#ifndef FWK_TASK_EVENT_H_
#define FWK_TASK_EVENT_H_

#include <fwk/task/task.h>

typedef uint8_t fwk_eventType_t;

typedef struct
{
	uint16_t datalen; /* size of the data field only, not including other source, eventType & other fields*/
	fwk_eventType_t eventType; /* event type */
	fwk_sysID_t sourceSys; /* source system ID */
	fwk_taskID_t sourceTask; /* source task ID */
	/* data sent by copy, i.e. data are encapsulated instead of just pass the pointer.
	 * obviously, event by copy can also be used to implement data by reference, but that's specific task design choice (not recommended)
	 */
	uint8_t data[0]; /* data attached to the event */
} fwk_event_t;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Send an event to queue.
 * Input:
 * 	dest: the destination task
 * 	eventType: event type to send
 * 	data: data to send
 * 	size: size of the data to send
 * 	timeout: ticks timeout
 * Return:
 * 	0 if succeed
 * 	error code defined in fwk_taskEventErrorCode_t if failed
 */
extern int fwk_task_sendEvent(fwk_taskID_t dest, fwk_eventType_t eventType,
		void * data, int datalen, int timeout);

/*
 * Send an urgent event, this event will be on top of all other events not yet handled by the destine task.
 * Often the destine task may also clear the queue after receiving an overruling event.
 * Input:
 * 	dest: the destination task
 * 	eventType: event type to send
 * 	data: data to send
 * 	size: size of the data to send
 * 	timeout: ticks timeout, set to 0 if no wait
 * Return:
 * 	0 if succeed
 * 	error code defined in fwk_taskEventErrorCode_t if failed
 */
extern int fwk_task_sendOverullingEvent(fwk_taskID_t dest,
		fwk_eventType_t eventType, void * data, int datalen, int timeout);

/*
 * Receive data from a queue.
 * Input:
 * 	timeout: timeout in ticks, set to 0 if no wait
 * Output:
 * 	event: holds the event if received.
 * Return:
 * 	0 if succeed
 * 	error code defined in fwk_taskEventErrorCode_t if failed
 */
extern int fwk_task_receiveEvent(fwk_event_t * event, int timeout);

/*
 * Clear not yet received events in the queue
 */
extern int fwk_task_clearEventQueue(fwk_taskID_t tid);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_TASK_EVENT_H_ */
