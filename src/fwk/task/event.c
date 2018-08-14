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
#include <string.h>

int fwk_task_sendEvent_imp(fwk_taskID_t dest, fwk_eventType_t eventType, const void * data, int datalen, int timeout, int urgency)
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

int fwk_task_sendEvent(fwk_taskID_t dest, fwk_eventType_t eventType, const void * data, int datalen, int timeout)
{
	return fwk_task_sendEvent_imp(dest, eventType, data, datalen, timeout, 0);
}

int fwk_task_sendOverullingEvent(fwk_taskID_t dest, fwk_eventType_t eventType, const void * data, int datalen, int timeout)
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
#if 0
#define FWK_EVENT_NAME_MAX_LEN 12
typedef struct {
	char name[FWK_EVENT_NAME_MAX_LEN];
	uint8_t depth; /* max message number */
	uint16_t size; /* each message length */
}fwk_eventAttr_t;
#endif
typedef struct {
	//fwk_eventAttr_t attr;
	void* qid; //also for used flag
	void* cid;
	void* mid;
}fwk_eventList_t;
#define FWK_EVENT_MAX_LIMIT 100
static fwk_eventList_t gFwkEventList[FWK_EVENT_MAX_LIMIT];
static int gCurEventCount = 0;

int fwk_createEvent(uint16_t msgLen, uint8_t msgDepth, void** eid, const char* name)
{
	int i, rc = 0;
	fwk_eventList_t* pEid = NULL;
	if (!msgLen || !msgDepth || !name) return FWK_E_PARAM;
	for (i = 0; i < FWK_EVENT_MAX_LIMIT; ++i) {
		if (gFwkEventList[i].qid == NULL) {
			pEid = &gFwkEventList[i];
			break;
		}
	}

	rc = fwk_createMutex(NULL, &pEid->mid);
	CHECK(rc);
	rc = fwk_createCond(NULL, &pEid->cid);
	CHECK(rc);

	fwk_queueAttr_t qAttr;
	strcpy(qAttr.name, name);
	qAttr.depth = msgDepth;
	qAttr.size = msgLen;
	qAttr.maxBufSize = 0;
	rc = fwk_createQueue(&qAttr, &pEid->mid,  &pEid->qid);
	CHECK(rc);
	if (eid) *eid = pEid;
	++gCurEventCount;
	return rc;
}

int fwk_sendEvent(void* eid, const void * data, uint16_t datalen, int timeout)
{
	int rc = 0, ck = 0;
	fwk_eventList_t* pEid = (fwk_eventList_t*)eid;
	if (!pEid) return FWK_E_PARAM;

	rc = fwk_lockMutex(pEid->mid, timeout);
	CHECK(rc);
	if (rc) return rc;
	ck = fwk_msgQSend(pEid->qid, data, datalen, NULL, 0, 0);
	//ck = fwk_sendToQueue(pEid->qid, data, datalen, timeout);
	CHECK(ck);
	if (!ck) { //Notify condition id
		rc = fwk_wakeupCond(pEid->cid);
		CHECK(rc);
	}
	rc = fwk_unlockMutex(pEid->mid);
	CHECK(rc);
	return ck;
}

int fwk_waitEvent(void* eid, void * buffer, uint16_t bufsize, int timeout)
{
	int rc = 0, len = 0;
	fwk_eventList_t* pEid = (fwk_eventList_t*)eid;
	if (!pEid) return FWK_E_PARAM;

	int remain = timeout;
	struct timespec ts1;
	int clkType;
	if (timeout > 0) {
		clkType = fwk_getCondClock(pEid->cid);
		if (timeout > 0) clock_gettime(clkType, &ts1);
	}
	rc = fwk_lockMutex(pEid->mid, timeout);
	CHECK(rc);
	if (timeout > 0) {
		remain = fwk_msDeltaRemain(timeout, clkType, &ts1);
	}

	len = fwk_msgQRecv(pEid->qid, buffer, bufsize, remain, NULL, 0, pEid->cid, pEid->mid);
	CHECK((len < 1));

	rc = fwk_unlockMutex(pEid->mid);
	CHECK(rc);
	return len;
}

void* fwk_findEvent(const char* name)
{
	int i;
	if (!name) return NULL;
	for (i = 0; i < FWK_EVENT_MAX_LIMIT; ++i) {
		if (gFwkEventList[i].qid) {
			fwk_queueAttr_t* pQid = (fwk_queueAttr_t*)(gFwkEventList[i].qid);
			if (!strcmp(pQid->name, name)) {
				return &gFwkEventList[i];
			}
		}
	}
	return NULL;
}

int fwk_clearEvent(void* eid)
{
	int rc = 0;
	fwk_eventList_t* pEid = (fwk_eventList_t*)eid;
	if (!pEid) return FWK_E_PARAM;
	rc = fwk_clearQueue(pEid->qid);
	CHECK(rc);
	pEid->qid = NULL;
	rc = fwk_deleteCond(pEid->cid);
	CHECK(rc);
	pEid->cid = NULL;
	rc = fwk_deleteMutex(pEid->mid);
	CHECK(rc);
	pEid->mid = NULL;
	--gCurEventCount;
	return rc;
}

void fwk_showEvent(void* eid)
{
	int i;
	fwk_eventList_t* pEid = (fwk_eventList_t*)eid;
	if (pEid) {
		fwk_showQueue(pEid->qid);
		fwk_showCond(pEid->cid);
		fwk_showMutex(pEid->mid);
	} else {
		for (i = 0; i < FWK_EVENT_MAX_LIMIT; ++i) {
			if (gFwkEventList[i].qid) {
				printf("Event[%i]: ", i);
				fwk_showEvent(&gFwkEventList[i]);
			}
		}
	}
}

