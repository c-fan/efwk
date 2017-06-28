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

#ifndef FWK_TASK_TASK_H_
#define FWK_TASK_TASK_H_

#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* system ID */
typedef uint8_t fwk_sysID_t;
/* task ID */
typedef ubase_t fwk_taskID_t;
/* task handler function prototype */
typedef void *(*fwk_taskFunc_proto_t)(void *);
/* task name maximum size */
#define FWK_TASK_NAME_MAX_LEN 8
/*
 * Error code
 */
typedef enum
{
	FWK_TASK_E_INTERNAL = 1, /* internal error */
	FWK_TASK_E_PARAM = 2, /* invalid parameter */
	FWK_TASK_E_RESOURCE = 3, /* lack resource */
} fwk_taskErrCode_t;

/*
 * Task resource reservation
 */
typedef struct
{
	/* stack size */
	uint16_t stackSize;
	/* mem pool size */
	ubase_t memPoolSize;
	/* default event queue */
	uint8_t queueSize; /* max size for each event in the queue */
	uint8_t queueDepth; /* max number of events */
} fwk_taskRes_t;
/*
 * Create preemptive task
 * These tasks that can preempt according to priority.
 * Input parameters:
 * 	tid: designated unique task ID if > 0; if input *tid == 0, then let system to allocate a TID
 * 	name: task name, unique within a system, as friendly name for debug
 * 	func: task function
 * 	args: arguments to pass to the start function
 * 	resource: task resource need
 * 	priority: task priority, higher priority > lower priority
 * 	policy: scheduling policy
 * 	independent: decides if manages resource independently from its parent task where it's created (in Linux, this means process v.s. thread).
 * Output:
 * 	tid: holds the allocated task ID if input *tid == 0.
 * Return:
 * 	0 if succeed
 * 	or error code as defined with fwk_taskErrCode_t
 */
extern int fwk_createPreemptiveTask(const char* const name, fwk_taskID_t * tid,
		fwk_taskFunc_proto_t func, void * args, uint8_t priority,
		uint8_t policy, fwk_taskRes_t resource, bool_t independent);

/*
 * Create normal task.
 * Normal task cannot preempt others and can be executed only if others cooperatively give up CPU, e.g. those tasks of CFS class in Linux.
 * In task scheduling, normal tasks shall always have lower priority than all real time tasks.
 * In a platform that supports only real-time tasks, this maybe implemented with all normal tasks at a low priority level.
 * In Linux platform, a priority can still be assigned, however, it may impact only the CPU time allocation.
 */
extern int fwk_createNormalTask(const char* const name, fwk_taskID_t * tid,
		fwk_taskFunc_proto_t func, void * args, uint8_t priority,
		fwk_taskRes_t resource, bool_t independent);

/*
 * delete a task
 * Input:
 * 	tid: task ID of the task to delete
 * Return:
 * 	0 if succeed
 * 	or error code as defined with fwk_taskErrCode_t
 */
extern int fwk_deleteTask(fwk_taskID_t tid);

/*
 * voluntarily yield CPU
 */
extern int fwk_yieldTask();

/*
 * suspend & resume a specific task
 */
extern int fwk_suspendTask(fwk_taskID_t tid);

extern int fwk_resumeTask(fwk_taskID_t tid);

/*
 * Get ID of the task that calling this API
 */
extern fwk_taskID_t fwk_myTaskId();

/*
 * task ID, name identification mapping
 */
extern fwk_taskID_t fwk_taskId(const char* const name);

extern const char* const fwk_taskName(fwk_taskID_t tid);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_TASK_TASK_H_ */
