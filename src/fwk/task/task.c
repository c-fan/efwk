//
//    Copyright (C) 2017 LGPL
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

#include <fwk/task/task.h>
#include <fwk/basic/basictrace.h>
#include <fwk/memmgmt/memmgmt.h>
#include <fwk/task/mutex.h>
#include <fwk/task/queue.h>
#include <fwk/task/event.h>

#include <errno.h>
#include <string.h>
#include <limits.h>

static fwk_taskList_t gFwkTaskList[FWK_TASK_MAX_LIMIT];
static int gCurTaskCount = 0;

int fwk_threadInit(fwk_taskList_t* pTask)
{
	int rc = 0;
	pTask->taskPause = 0;
	rc = fwk_createMutex(NULL, &pTask->mid);
	CHECK(rc);
	rc = fwk_createCond(NULL, &pTask->cid);
	CHECK(rc);
	if (pTask->attr.resource.queueSize > 0) {
		fwk_queueAttr_t qAttr;
		strcpy(qAttr.name, pTask->attr.name);
		qAttr.depth = pTask->attr.resource.queueDepth;
		qAttr.size = pTask->attr.resource.queueSize;
		qAttr.maxBufSize = 0;
		rc = fwk_createQueue(&qAttr, &pTask->mid,  &pTask->qid);
		CHECK(rc);
	} else {
		pTask->qid = NULL;
	}
	pTask->pid = NULL;
	if (pTask->attr.resource.memPoolSize > 0) {
		pTask->pid = fwk_memmgmt_malloc(pTask->attr.resource.memPoolSize);
		CHECK(!pTask->pid);
	}
	return rc;
}

int fwk_threadExit(fwk_taskList_t* pTask)
{
	int rc = 0;
	if (pTask->qid) {
		rc = fwk_clearQueue(pTask->qid);
		CHECK(rc);
	}
	rc = fwk_deleteCond(pTask->cid);
	CHECK(rc);
	rc = fwk_deleteMutex(pTask->mid);
	CHECK(rc);
	if (pTask->pid) fwk_memmgmt_free(pTask->pid);
	return rc;
}

int fwk_pauseTask(fwk_taskList_t* pTask)
{
	int rc = 0, ck = 0;
	pthread_testcancel();
	if (pTask->taskPause) {
		rc = fwk_lockMutex(pTask->mid, FOREVER);
		CHECK(rc);
		if (rc) return FWK_E_INTERNAL;
		pTask->taskPause = 0;
		ck = fwk_waitCond(pTask->cid, pTask->mid, FOREVER);
		CHECK(ck);
		rc = fwk_unlockMutex(pTask->mid);
		CHECK(rc);
		if (ck) {
			fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
				"wait condition failed, rc %i, errno %i\n", rc, errno);
		}
	}
	return ck;
}

void* fwk_threadWrapper(void* args)
{
	fwk_taskList_t* pTask = (fwk_taskList_t*)args;
	void* prc = NULL;
	int rc = 0, i = 0;

	rc += pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	rc += pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	rc = fwk_threadInit(pTask);
	if (rc) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"thread initialization failed [%s], rc %i, errno %i\n", pTask->attr.name, rc, errno);
	}
	if (pTask->attr.initFunc) {
		(void)pTask->attr.initFunc(pTask->attr.args);
	}

	for(;;++i) {
		if ((pTask->attr.loopTimes != -1) && (i >= pTask->attr.loopTimes)) break;
		rc = fwk_pauseTask(pTask);
		prc = pTask->attr.func(pTask->attr.args);
	}
	rc = fwk_threadExit(pTask);
	return prc;
}

int fwk_createTask(fwk_taskAttr_t* pAttr, fwk_taskID_t* pTid)
{
	int i, tcbIdx;
	fwk_taskList_t* pTcb = NULL;
	if (!pAttr) return FWK_E_PARAM;

	for (i = 0; i < FWK_TASK_MAX_LIMIT; ++i) {
		if (gFwkTaskList[i].used == 0) {
			pTcb = &gFwkTaskList[i];
			tcbIdx = i;
			break;
		}
	}
	if (!pTcb) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"No mutex block to alloc, try to enlarge FWK_TASK_MAX_LIMIT.\n");
		return FWK_E_RESOURCE;
	}

	int len = strlen(pAttr->name);
	if (!len) {
		sprintf(pTcb->attr.name, "task%i", tcbIdx);
	} else {
		len = (len < FWK_TASK_NAME_MAX_LEN) ? len : (FWK_TASK_NAME_MAX_LEN - 1);
		strncpy(pTcb->attr.name, pAttr->name, len);
	}
	pTcb->attr.func = pAttr->func;
	pTcb->attr.args = pAttr->args;
	pTcb->attr.policy = pAttr->policy;
	pTcb->attr.priority = pAttr->priority;
	fwk_memmgmt_cpy(&pTcb->attr.resource, &pAttr->resource, sizeof(pAttr->resource));
	if (pTcb->attr.resource.stackSize < PTHREAD_STACK_MIN) { //ARM64=131072, ARM32=16384
		pTcb->attr.resource.stackSize = PTHREAD_STACK_MIN;
	}
	pTcb->attr.independent = pAttr->independent;
	pTcb->attr.taskType = pAttr->taskType;
	pTcb->attr.loopTimes = pAttr->loopTimes;
	pTcb->attr.initFunc = pAttr->initFunc;

	int rc = 0;
	pthread_attr_t attr;
	pthread_t pid = 0;
	rc += pthread_attr_init(&attr);
	CHECK(rc);
	rc += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //PTHREAD_CREATE_JOINABLE
	CHECK(rc);
#if 0
	size_t sz = 0;
	rc += pthread_attr_getstacksize (&attr, &sz);
	printf("default stack size: %u -> %u rc %i\n", (unsigned int)sz, (unsigned int)pTcb->attr.resource.stackSize, rc);
#endif
	rc += pthread_attr_setstacksize (&attr, pTcb->attr.resource.stackSize);
	CHECK(rc);
	rc += pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED); //PTHREAD_INHERIT_SCHED
	CHECK(rc);
#ifndef HOST //EPERM  No permission to set the scheduling policy and parameters specified in attr
	rc += pthread_attr_setschedpolicy (&attr, pTcb->attr.policy);
	CHECK(rc);
	if (pTcb->attr.policy == SCHED_FIFO || pTcb->attr.policy == SCHED_RR) {
		struct sched_param schedParm;
		rc += pthread_attr_getschedparam (&attr, &schedParm);
		CHECK(rc);
		schedParm.__sched_priority = pTcb->attr.priority;
		CHECK(rc);
		rc += pthread_attr_setschedparam (&attr, &schedParm);
		CHECK(rc);
	} else {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_WRN,
			"Warning: sched policy %i can't support priority %i\n", pTcb->attr.policy, pTcb->attr.priority);
	}
#endif
	//rc += pthread_attr_setscope(&attr, pTcb->attr.taskType ? PTHREAD_SCOPE_PROCESS : PTHREAD_SCOPE_SYSTEM);
	rc += pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	CHECK(rc);
	if (rc) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_WRN,
			"pthread_attr_set failed [%s], rc %i, errno %i\n", pTcb->attr.name, rc, errno);
	}
	rc = pthread_create(&pid, &attr, fwk_threadWrapper, pTcb);
	CHECK(rc);
	if (rc) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"pthread_create failed [%s], rc %i, errno %i\n", pTcb->attr.name, rc, errno);
		return FWK_E_INTERNAL;
	}
	if (pTid) *pTid = pTcb;
	pTcb->tid = pid;
	pTcb->used = 1;
	++gCurTaskCount;
	return rc;
}

int fwk_createPreemptiveTask(const char* const name, fwk_taskID_t * tid, fwk_taskFunc_proto_t func, void * args, uint8_t priority, uint8_t policy, fwk_taskRes_t resource, bool_t independent)
{
	fwk_taskAttr_t attr = {0,};
	if (name) strncpy(attr.name, name, FWK_TASK_NAME_MAX_LEN);
	attr.func = func;
	attr.args = args;
	attr.policy = policy;
	attr.priority = priority;
	fwk_memmgmt_cpy(&attr.resource, &resource, sizeof(resource));
	attr.independent = independent;
	attr.taskType = 0;
	attr.loopTimes = -1;
	attr.initFunc = NULL;
	return fwk_createTask(&attr, tid);
}

int fwk_createNormalTask(const char* const name, fwk_taskID_t * tid, fwk_taskFunc_proto_t func, void * args, uint8_t priority, fwk_taskRes_t resource, bool_t independent)
{
	fwk_taskAttr_t attr = {0,};
	if (name) strncpy(attr.name, name, FWK_TASK_NAME_MAX_LEN);
	attr.func = func;
	attr.args = args;
	attr.policy = SCHED_OTHER; //0-SCHED_NORMAL/OTHER, 1-SHCED_FIFO, 2-SCHED_RR
	attr.priority = priority;
	fwk_memmgmt_cpy(&attr.resource, &resource, sizeof(resource));
	attr.independent = independent;
	attr.taskType = 1;
	attr.loopTimes = -1;
	attr.initFunc = NULL;
	return fwk_createTask(&attr, tid);
}

int fwk_deleteTask(fwk_taskID_t tid)
{
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
	if (!pTask) return FWK_E_PARAM;
	pthread_t pid = pTask->tid;
	//int sig = SIGQUIT; //0-Test; SIGQUIT, SIGKILL - signal(SIGKILL, sig_handler);
	//int rc = pthread_kill(pid, sig);
	int rc = pthread_cancel(pid);
	if (rc && (rc != ESRCH)) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"thread[%i] is not exited, rc %i, errno %i.\n", pid, rc, errno);
	}
	void* prc = NULL;
	rc = pthread_join(pid, &prc);
	if (rc && (rc != EINVAL)) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"pthread_joid failed for thread[%i] is not exited, rc %i, errno %i. prc[0x%x=0x%x]\n", pid, rc, errno, (unsigned int*)prc, *(unsigned int*)prc);
	} else {
		rc = 0;
	}
	rc = fwk_threadExit(pTask);
	pTask->used = 0;
	--gCurTaskCount;
	return rc;
}

int fwk_clearTask()
{
	int i, rc = 0;
	for (i = 0; i < FWK_TASK_MAX_LIMIT; ++i) {
		if (gFwkTaskList[i].used != 0) {
			rc += fwk_deleteTask(&gFwkTaskList[i]);
			printf("task %i: %s exit with %i\n", i, fwk_taskName(&gFwkTaskList[i]), rc);
		}
	}
	return rc;
}

int fwk_yieldTask()
{
	return pthread_yield();
}

int fwk_suspendTask(fwk_taskID_t tid)
{
	int rc = 0;
	fwk_taskList_t* pTask = (fwk_taskList_t*)tid;
	if (!pTask) return FWK_E_PARAM;
	rc = fwk_lockMutex(pTask->mid, FOREVER);
	CHECK(rc);
	if (rc) return FWK_E_INTERNAL;
	pTask->taskPause = 1;
	rc = fwk_unlockMutex(pTask->mid);
	CHECK(rc);
	return rc;
}

int fwk_resumeTask(fwk_taskID_t tid)
{
	int rc = 0, ck = 0;
	fwk_taskList_t* pTask = (fwk_taskList_t*)tid;
	if (!pTask) return FWK_E_PARAM;
	rc = fwk_lockMutex(pTask->mid, FOREVER);
	CHECK(rc);
	pTask->taskPause = 0;
	ck = fwk_wakeupCond(pTask->cid);
	CHECK(ck);
	rc = fwk_unlockMutex(pTask->mid);
	return ck;
}

unsigned long int fwk_rawTid(fwk_taskID_t tid)
{
	if (tid) {
		fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
		return pTask->tid;
	} else {
		pthread_t pid = pthread_self();
		return pid;
	}
}

fwk_taskID_t fwk_myTaskId()
{
	int i;
	pthread_t pid = pthread_self();

	for (i = 0; i < FWK_TASK_MAX_LIMIT; ++i) {
		if ((gFwkTaskList[i].tid == pid) && gFwkTaskList[i].used) {
			return &gFwkTaskList[i];
		}
	}
	return NULL;
}

fwk_taskID_t fwk_taskId(const char* const name)
{
	int i = 0;
	int len = strlen(name);
	len =  (len < FWK_TASK_NAME_MAX_LEN) ? len : (FWK_TASK_NAME_MAX_LEN - 1);
	for (i = 0; i < FWK_TASK_MAX_LIMIT; ++i) {
		if (!gFwkTaskList[i].used) continue;
		if (!strncmp(name, gFwkTaskList[i].attr.name, len)) {
			return &gFwkTaskList[i];
		}
	}
	return NULL;
}

const char* const fwk_taskName(fwk_taskID_t tid)
{
	//pthread_getname_np(pid, name, len);
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
	if (pTask) return pTask->attr.name;
	return NULL;
}

void* fwk_taskMemPool(fwk_taskID_t tid)
{
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
	if (!pTask) return NULL;
	return pTask->pid;
}

int fwk_terminateTask()
{
	fwk_taskID_t tid = fwk_myTaskId();
	if (!tid) return FWK_E_INTERNAL;
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
	if (pTask) {
		pTask->attr.loopTimes = 0;
	}
	return 0;
}

void fwk_showTask(fwk_taskID_t tid)
{
	int i;
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid);
	if (pTask) {
		//fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
		printf(
			"task name=%s, func=%"fwk_addr_f", args=%"fwk_addr_f", policy=%i, priority=%i, "
			"resource[stackSize=%i, memPoolSize=%ld, queueSize=%i, queueDepth=%i], "
			"independent=%i, taskType=%i, loopTimes=%i, initFunc=%"fwk_addr_f"; tid=%"fwk_addr_f", taskPause=%i\n",
			pTask->attr.name,
			(fwk_addr_t)pTask->attr.func,
			(fwk_addr_t)pTask->attr.args,
			pTask->attr.policy,
			pTask->attr.priority,
			(int)pTask->attr.resource.stackSize,
			pTask->attr.resource.memPoolSize,
			pTask->attr.resource.queueSize,
			pTask->attr.resource.queueDepth,
			pTask->attr.independent,
			pTask->attr.taskType,
			pTask->attr.loopTimes,
			(fwk_addr_t)pTask->attr.initFunc,
			(fwk_addr_t)pTask->tid,
			pTask->taskPause);
		if (pTask->mid) fwk_showMutex(pTask->mid);
		if (pTask->cid) fwk_showCond(pTask->cid);
		if (pTask->qid) fwk_showQueue(pTask->qid);
	} else {
		for (i = 0; i < FWK_TASK_MAX_LIMIT; ++i) {
			if (gFwkTaskList[i].used != 0) {
				printf("Task[%i]: ", i);
				fwk_showTask(&gFwkTaskList[i]);
			}
		}
	}
}

