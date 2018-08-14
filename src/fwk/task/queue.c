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

#include <fwk/task/queue.h>
#include <fwk/memmgmt/memmgmt.h>
#include <fwk/basic/basictrace.h>

#include <errno.h>
#include <string.h>

typedef struct {
	uint16_t msgLen;
	void* msgBuf;
}fwk_msgNode_t;
typedef struct {
	fwk_queueAttr_t attr;
	void* mid; //also for used flag
	int aloneMutex;
	int head;
	int tail;
	void* msgQ;
	fwk_msgNode_t* varQ;
}fwk_queueList_t;
#define FWK_QUEUE_MAX_LIMIT 100
static fwk_queueList_t gFwkQueueList[FWK_QUEUE_MAX_LIMIT];
static int gCurQueueCount = 0;

int fwk_createQueue(fwk_queueAttr_t* pQAttr, void* mid, fwk_queueID_t* qid)
{
	int i, qIdx, rc = 0;
	fwk_queueList_t* pQid = NULL;
	fwk_queueAttr_t attr;

	for (i = 0; i < FWK_QUEUE_MAX_LIMIT; ++i) {
		if (!gFwkQueueList[i].mid) {
			qIdx = i;
			pQid = &gFwkQueueList[i];
			break;
		}
	}
	if (!pQid) {
		return FWK_E_QUEUE_FULL;
	}

	if (!mid) {
		//fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_WRN,
		//	"Create independent mutex for queue.\n");
		rc = fwk_createMutex(NULL, &pQid->mid);
		CHECK(rc);
		if (rc) {
			fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
				"%s create mutex failed, rc %i, errno %i\n", __func__, rc, errno);
			return FWK_E_INTERNAL;
		}
		pQid->aloneMutex = 1;
	} else {
		pQid->mid = mid;
		pQid->aloneMutex = 0;
	}

	if (!pQAttr) {
		pQAttr->name[0] = 0;
		pQAttr->depth = FWK_QUEUE_MSG_DEF_NUM;
		pQAttr->size = FWK_QUEUE_MSG_DEF_LEN;
		pQAttr->maxBufSize = 0;
		pQAttr = &attr;
	}
	if (!pQAttr->name[0]) sprintf(pQAttr->name, "Queue%i", qIdx);
	fwk_memmgmt_cpy(&pQid->attr, pQAttr, sizeof(*pQAttr));

	uint32_t qSize = 0;
	if (pQAttr->size) { //FixSize queue
		pQAttr->maxBufSize = pQAttr->depth * pQAttr->size;
	} else { //varSize queue: maxBufSize + node list
		if (!pQAttr->depth) pQAttr->depth = FWK_QUEUE_MSG_DEF_NUM;
	}
	qSize = pQAttr->maxBufSize + sizeof(fwk_msgNode_t) * pQAttr->depth;
	pQid->msgQ = fwk_memmgmt_malloc(qSize);
	if (!pQid->msgQ) {
		if (pQid->aloneMutex) {
			rc = fwk_deleteMutex(pQid->mid);
			CHECK(rc);
		}
		pQid->mid = NULL;
		return FWK_E_NO_MEM;
	}
	fwk_memmgmt_set(pQid->msgQ, 0, qSize);
	pQid->head = 0;
	pQid->tail = pQid->head;
	pQid->varQ = (fwk_msgNode_t*)((uint32_t*)pQid->msgQ + (pQAttr->maxBufSize + sizeof(uint32_t) - 1)/sizeof(uint32_t));
	if (qid) *qid = pQid;
	++gCurQueueCount;
	if (rc) {
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
			"%s failed, rc %i, errno %i\n", __func__, rc, errno);
	}

	return rc;
}

int fwk_createFixsizeQueue(fwk_queueID_t * qID, char * name, uint8_t depth, uint16_t size)
{
	fwk_queueAttr_t attr = {{0}, depth, size, 0};
	//if (name) strncmp(attr.name, name, FWK_QUEUE_NAME_MAX_LEN);
	if (name) sprintf(attr.name, "%s", name);
	int rc = fwk_createQueue(&attr, NULL, qID);
	return rc;
}

int fwk_createVarSizeQueue(fwk_queueID_t * qID, char * name, uint16_t maxBufSize)
{
	fwk_queueAttr_t attr = {{0}, 0, 0, maxBufSize};
	//if (name) strncmp(attr.name, name, FWK_QUEUE_NAME_MAX_LEN);
	//if (name) strcmp(attr.name, name); //statement with no effect [-Wunused-value]
	if (name) sprintf(attr.name, "%s", name);
	int rc = fwk_createQueue(&attr, NULL, qID);
	return rc;
}

int fwk_insertToVarQ_imp(fwk_queueList_t* pQid, const void * data, uint16_t size, int msgIdx, void* head, uint16_t headLen)
{
	//used for building loop msgQ, in case of varSize queue
	fwk_msgNode_t* pHead = pQid->varQ + pQid->head;
	fwk_msgNode_t* pTail = pQid->varQ + pQid->tail;
	fwk_msgNode_t* pNode = pQid->varQ + msgIdx;
	uint8_t* msgQTail = (uint8_t*)(pQid->msgQ) + pQid->attr.maxBufSize;
	pNode->msgBuf = (uint8_t*)pHead->msgBuf - headLen - size;
	if ((uint8_t*)pTail->msgBuf + pTail->msgLen > msgQTail) { //old tail is loop
		int msgLoopLen = pTail->msgLen - ((ubase_t)msgQTail - (ubase_t)pTail->msgBuf);
		if ((uint8_t *)pNode->msgBuf < (uint8_t *)pQid->msgQ + msgLoopLen) return FWK_E_QUEUE_FULL;
		if (headLen) fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
		fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, size);
	} else if (pNode->msgBuf < pQid->msgQ) { //new head is loop
		int msgLoopLen = (uint8_t *)pQid->msgQ - (uint8_t *)pNode->msgBuf;
		pNode->msgBuf = msgQTail - (headLen + size - msgLoopLen);
		if ((uint8_t*)pNode->msgBuf < ((uint8_t*)pTail->msgBuf + pTail->msgLen)) return FWK_E_QUEUE_FULL;
		if ((uint8_t*)pNode->msgBuf + headLen > msgQTail) {
			fwk_memmgmt_cpy(pNode->msgBuf, head, msgLoopLen);
			fwk_memmgmt_cpy(pQid->msgQ, (uint8_t*)head + msgLoopLen, headLen - msgLoopLen);
			fwk_memmgmt_cpy((uint8_t*)pQid->msgQ + headLen - msgLoopLen, data, size);
		} else {
			fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
			fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, msgLoopLen - headLen);
			fwk_memmgmt_cpy(pQid->msgQ, (uint8_t*)data + msgLoopLen - headLen, size - (msgLoopLen - headLen));
		}
	} else { //normal
		if (headLen) fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
		fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, size);
	}
	pQid->head = msgIdx;
	pNode->msgLen = headLen + size;
	return 0;
}

int fwk_sendToVarQ_imp(fwk_queueList_t* pQid, const void * data, uint16_t size, int msgIdx, void* head, uint16_t headLen)
{
	//used for building loop msgQ, in case of varSize queue
	fwk_msgNode_t* pHead = pQid->varQ + pQid->head;
	fwk_msgNode_t* pTail = pQid->varQ + pQid->tail;
	fwk_msgNode_t* pNode = pQid->varQ + msgIdx;
	uint8_t* msgQTail = (uint8_t*)(pQid->msgQ) + pQid->attr.maxBufSize;
	pNode->msgBuf = (uint8_t*)pTail->msgBuf + pTail->msgLen;
	if ((uint8_t*)pNode->msgBuf > msgQTail) { //old tail is loop
		int msgLoopLen = pTail->msgLen - ((ubase_t)msgQTail - (ubase_t)pTail->msgBuf);
		pNode->msgBuf = (uint8_t*)pQid->msgQ + msgLoopLen;
		if (((uint8_t*)pNode->msgBuf + headLen + size) > (uint8_t*)pHead->msgBuf) return FWK_E_QUEUE_FULL;
		if (headLen) fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
		fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, size);
	} else if ((uint8_t*)pNode->msgBuf + headLen + size > msgQTail) { //new tail is loop
		int msgLoopLen = headLen + size - ((ubase_t)msgQTail - (ubase_t)pNode->msgBuf);
		if ((uint8_t*)pQid->msgQ + msgLoopLen > (uint8_t*)pHead->msgBuf) return FWK_E_QUEUE_FULL;
		if ((uint8_t*)pNode->msgBuf + headLen > msgQTail) {
			int tailLen = headLen + size - msgLoopLen;
			fwk_memmgmt_cpy(pNode->msgBuf, head, tailLen);
			fwk_memmgmt_cpy(pQid->msgQ, (uint8_t*)head + tailLen , headLen - tailLen);
			fwk_memmgmt_cpy((uint8_t*)pQid->msgQ + headLen - tailLen, data, size);
		} else {
			int sectLen = size - msgLoopLen;
			fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
			fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, sectLen);
			fwk_memmgmt_cpy(pQid->msgQ, (uint8_t*)data + sectLen, msgLoopLen);
		}
	} else { //normal
		if (headLen) fwk_memmgmt_cpy(pNode->msgBuf, head, headLen);
		fwk_memmgmt_cpy((uint8_t*)pNode->msgBuf + headLen, data, size);
	}
	pQid->tail = msgIdx;
	pNode->msgLen = headLen + size;
	return 0;
}

int fwk_sendToQueue(fwk_queueID_t qID, const void * data, uint16_t size, int timeout)
{
	int lck = 0, rc = 0;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (!pQid) return FWK_E_PARAM;
	lck = fwk_lockMutex(pQid->mid, timeout);
	CHECK(lck);
	rc = fwk_msgQSend(qID, data, size, NULL, 0, 0);
	CHECK(rc);
	lck = fwk_unlockMutex(pQid->mid);
	CHECK(lck);
	return rc;
}

int fwk_msgQSend(fwk_queueID_t qID, const void * data, uint16_t size, void* head, uint16_t headLen, int priority)
{
	int rc = 0;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (!pQid || !data) return FWK_E_PARAM;
	if ((!head && headLen) || (head && !headLen)) return FWK_E_PARAM;
	int msgIdx = -1, newTailIdx = -1;
	void* msgBuf = NULL;

	if (priority) {
		msgIdx = (pQid->head - 1 + pQid->attr.depth)%(pQid->attr.depth);
		if (msgIdx == pQid->tail) return FWK_E_QUEUE_FULL;
	} else {
		msgIdx = pQid->tail; //Reserve a node as seperator, tail point new node
		newTailIdx =  (pQid->tail + 1)%(pQid->attr.depth);
		if (newTailIdx == pQid->head) return FWK_E_QUEUE_FULL;
	}
	if (pQid->attr.size) { //Fixsize queue
		if (size > pQid->attr.size) {
			return FWK_E_PARAM;
		}
		//Commit
		if (priority) {
			pQid->head = msgIdx;
		} else {
			pQid->tail = newTailIdx;
		}
		msgBuf = (uint8_t*)(pQid->msgQ) + msgIdx * pQid->attr.size;
		if (head && headLen) fwk_memmgmt_cpy(msgBuf, head, headLen);
		fwk_memmgmt_cpy((uint8_t*)msgBuf + headLen, data, size);
#if 0
		if (size + headLen < pQid->attr.size) {
			fwk_memmgmt_set((uint8_t*)msgBuf + headLen + size, 0, pQid->attr.size - headLen - size);
		}
#else
		fwk_msgNode_t* pNode = pQid->varQ + msgIdx;
		pNode->msgLen = size + headLen;
#endif
	} else { //Varsize queue
		if (size > pQid->attr.maxBufSize) {
			return FWK_E_PARAM;
		}
		if (priority) {
			rc = fwk_insertToVarQ_imp(pQid, data, size, msgIdx, head, headLen);
		} else {
			rc = fwk_sendToVarQ_imp(pQid, data, size, msgIdx, head, headLen);
		}
		if (rc) {
			return FWK_E_QUEUE_FULL;
		}
	}
	return rc;
}

int fwk_recvFromVarQ_imp(fwk_queueList_t* pQid, void * buffer, uint16_t bufsize)
{
	fwk_msgNode_t* pNode = pQid->varQ + pQid->head;
	//fwk_msgNode_t* pTail = pQid->varQ + pQid->tail;
	uint8_t* msgQTail = (uint8_t*)(pQid->msgQ) + pQid->attr.maxBufSize;

	if (bufsize < pNode->msgLen) return FWK_E_QUEUE_BUFOVERFLOW;
	if ((uint8_t*)pNode->msgBuf + pNode->msgLen > msgQTail) { //loop
		int msgLoopLen = pNode->msgLen - ((ubase_t)msgQTail - (ubase_t)pNode->msgBuf);
		fwk_memmgmt_cpy(buffer, pNode->msgBuf, pNode->msgLen - msgLoopLen);
		fwk_memmgmt_cpy((uint8_t*)buffer + pNode->msgLen - msgLoopLen, pQid->msgQ, msgLoopLen);
	} else { //normal
		fwk_memmgmt_cpy(buffer, pNode->msgBuf, pNode->msgLen);
	}
	return pNode->msgLen;
}

int fwk_msgQRecv(fwk_queueID_t qID, void * buffer, uint16_t bufsize, int timeout, void* head, uint16_t headLen, void* cid, void* mid)
{
	int efficient = 0;
	int rc = 0;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (!pQid || !buffer) return FWK_E_PARAM;
	if ((!head && headLen) || (head && !headLen)) return FWK_E_PARAM;
	int len = 0;

	if (pQid->head == pQid->tail) {
		int isEmpty = 1;
		if (cid && mid) { //Wait condition id
			rc = fwk_waitCond(cid, mid, timeout);
			CHECK(rc);
			if (!rc && (pQid->head != pQid->tail)) {
				isEmpty = 0;
			}
		}
		if (isEmpty) {
			return FWK_E_QUEUE_NODATA;
		}
	}

	if (pQid->attr.size) { //Fixsize queue
#if 0
		len = pQid->attr.size;
#else //use the actual message length
		fwk_msgNode_t* pNode = pQid->varQ + pQid->head;
		len = pNode->msgLen;
#endif
		if (bufsize < len + headLen) {
			return FWK_E_QUEUE_BUFOVERFLOW;
		}
		uint8_t* msgBuf  = (uint8_t*)(pQid->msgQ) + pQid->head * pQid->attr.size;
		if (efficient) {
			if (head) *(fwk_addr_t*)head = (fwk_addr_t)msgBuf;
			*(fwk_addr_t*)buffer = (fwk_addr_t)(msgBuf + headLen);
		} else {
			if (head && headLen) fwk_memmgmt_cpy(head, msgBuf, headLen);
			fwk_memmgmt_cpy(buffer, msgBuf + headLen, len - headLen);
		}
		pQid->head = (pQid->head + 1) % pQid->attr.depth;
	} else { //Varsize queue
		if (head || headLen) return FWK_E_PARAM; //No support
		len = fwk_recvFromVarQ_imp(pQid, buffer, bufsize);
		if (len < 0) {
			return FWK_E_QUEUE_BUFOVERFLOW;
		}
	}
	return len;
}

int fwk_receiveFromQueue(fwk_queueID_t qID, void * buffer, uint16_t bufsize, int timeout)
{
	int lck = 0, len = 0;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (!pQid || !buffer) return FWK_E_PARAM;
	lck = fwk_lockMutex(pQid->mid, timeout);
	CHECK(lck);
	len = fwk_msgQRecv(qID, buffer, bufsize, timeout, NULL, 0, NULL, NULL);
	lck = fwk_unlockMutex(pQid->mid);
	CHECK(lck);
	return len;
}

int fwk_clearQueue(fwk_queueID_t qID)
{
	int rc = 0;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (!pQid) return FWK_E_PARAM;
	rc += fwk_lockMutex(pQid->mid, FOREVER);
	fwk_memmgmt_free(pQid->msgQ);
	pQid->msgQ = NULL;
	rc += fwk_unlockMutex(pQid->mid);
	if (pQid->aloneMutex) rc += fwk_deleteMutex(pQid->mid);
	pQid->mid = NULL;
	return rc;
}

void fwk_showQueue(fwk_queueID_t qID)
{
	int i, k = 0, idx, end;
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	if (pQid) {
		//fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK, FWK_BASICTRACE_LEVEL_ERR,
		printf("attr[name=%s, depth=%i, size=%i, maxBufSize=%i], mid[%"fwk_addr_f"], msg[head=%i, tail=%i]:\n",
			pQid->attr.name,
			pQid->attr.depth,
			pQid->attr.size,
			pQid->attr.maxBufSize,
			(fwk_addr_t)pQid->mid,
			pQid->head,
			pQid->tail
			);
		if (pQid->head == pQid->tail) {
			printf("Queue is empty.\n");
			return;
		}
		end = (pQid->head < pQid->tail) ? pQid->tail : (pQid->tail + pQid->attr.depth);
		if (pQid->attr.size) {
			for (k = pQid->head; k < end; ++k) {
				idx = k % pQid->attr.depth;
				fwk_msgNode_t* pNode = pQid->varQ + idx;
				printf("\t%i: msg[len %i]=%s\n", idx, pNode->msgLen, (char*)pQid->msgQ + idx * pQid->attr.size);
			}
		} else {
			uint8_t* msgQTail = (uint8_t*)(pQid->msgQ) + pQid->attr.maxBufSize;
			for (k = pQid->head; k < end; ++k) {
				idx = k % pQid->attr.depth;
				fwk_msgNode_t* pNode = pQid->varQ + idx;
				printf("\t%i: len=%i, addr=%"fwk_addr_f", msg=%s\n", idx, pNode->msgLen, (fwk_addr_t)pNode->msgBuf, (char*)pNode->msgBuf);
				if ((uint8_t*)pNode->msgBuf + pNode->msgLen > msgQTail) {
					printf("\t--: %s\n", (char*)pQid->msgQ);
				}
			}
		}
	} else {
		for (i = 0; i < FWK_QUEUE_MAX_LIMIT; ++i) {
			if (gFwkQueueList[i].mid) {
				printf("Queue[%i]: ", i);
				fwk_showQueue(&gFwkQueueList[i]);
			}
		}
	}
}

uint16_t fwk_getMsgSize(fwk_queueID_t qID)
{
	fwk_queueList_t* pQid = (fwk_queueList_t*)qID;
	return pQid ? pQid->attr.size : 0;
}

