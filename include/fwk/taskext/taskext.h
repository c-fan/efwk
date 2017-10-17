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

#ifndef FWK_TASKEXT_TASKEXT_H_
#define FWK_TASKEXT_TASKEXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef int (*fwk_taskDelete_proto_t)(void * nativeTaskInfo);
typedef int (*fwk_taskSuspend_proto_t)(void * nativeTaskInfo);
typedef int (*fwk_taskResume_proto_t)(void * nativeTaskInfo);
typedef int (*fwk_taskSendEvent_proto_t)(void * nativeTaskInfo,
		fwk_eventType_t eventType, void * data, int datalen, int timeout);

typedef struct
{
	fwk_taskDelete_proto_t deleteFunc;
	fwk_taskSuspend_proto_t suspendFunc;
	fwk_taskResume_proto_t resumeFunc;
	fwk_taskSendEvent_proto_t sendEventToMe;
} fwk_taskMgr_t;

/*
 * put a task otherwise created into management.
 * the task maybe local or remote, identified with sid (system ID), if sid=0 it's a local task.
 */
int fwk_registerTask(fwk_sysID_t sid, fwk_taskID_t tid, const char* name,
		void * nativeTaskInfo, fwk_taskMgr_t nativeTaskMgr);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_TASKEXT_TASKEXT_H_ */
