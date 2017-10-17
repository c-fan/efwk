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

#include <fwk/task/event.h>
#include <fwk/task/queue.h>
#include <fwk/memmgmt/memmgmt.h>
#include <fwk/basic/basictrace.h>
#include <errno.h>

int fwk_task_sendEvent_imp(fwk_taskID_t dest, fwk_eventType_t eventType, void * data, int datalen, int timeout, int urgency)
{
	int rc = 0, ck = 0;
	fwk_taskList_t* pTask = (fwk_taskList_t*)(dest);
	if (!pTask) return FWK_E_PARAM;
	fwk_event_t head = {eventType, datalen, 0, fwk_myTaskId()};

	rc = fwk_lockMutex(pTask->mid, timeout);
	CHECK(rc);
	if (rc) return rc;
	ck = fwk_msgQSend(pTask->qid, data, datalen, &head, sizeof(head), urgency);
	CHECK(ck);
	if (!ck) { //Notify condition id
		rc = fwk_wakeupCond(pTask->cid);
		CHECK(rc);
	}
	rc = fwk_unlockMutex(pTask->mid);
	CHECK(rc);
	return ck;
}

int fwk_task_sendEvent(fwk_taskID_t dest, fwk_eventType_t eventType, void * data, int datalen, int timeout)
{
	return fwk_task_sendEvent_imp(dest, eventType, data, datalen, timeout, 0);
}

int fwk_task_sendOverullingEvent(fwk_taskID_t dest, fwk_eventType_t eventType, void * data, int datalen, int timeout)
{
	return fwk_task_sendEvent_imp(dest, eventType, data, datalen, timeout, 1);
}

int fwk_task_receiveEvent(fwk_event_t* event, int timeout)
{
	int rc = 0, len = 0;
	fwk_taskList_t* pTask = (fwk_taskList_t*)fwk_myTaskId();
	if (!event || !pTask) return FWK_E_PARAM;

	int remain = timeout;
	struct timespec ts1;
	int clkType;
	if (timeout > 0) {
		clkType = fwk_getCondClock(pTask->cid);
		if (timeout > 0) clock_gettime(clkType, &ts1);
	}
	rc = fwk_lockMutex(pTask->mid, timeout);
	CHECK(rc);
	if (timeout > 0) {
		remain = fwk_msDeltaRemain(timeout, clkType, &ts1);
	}

	uint16_t msgLen = fwk_getMsgSize(pTask->qid);
	len = fwk_msgQRecv(pTask->qid, event, msgLen, remain, NULL, 0, pTask->cid, pTask->mid);

	rc = fwk_unlockMutex(pTask->mid);
	CHECK(rc);
	return len;
}

int fwk_task_clearEventQueue(fwk_taskID_t tid)
{
	int rc = 0;
	fwk_taskList_t* pTask = (fwk_taskList_t*)(tid ? tid : fwk_myTaskId());
	if (!pTask) return FWK_E_PARAM;
	rc = fwk_clearQueue(pTask->qid);
	return rc;
}

