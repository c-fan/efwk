#include  <fwk/task/task.h>
#include  <fwk/task/queue.h>
#include  <fwk/task/event.h>
#include  <fwk/basic/basictrace.h>
#include  <fwk/memmgmt/memmgmt.h>
#include  <fwk/timer/stw_mgmt.h>

#include <stdio.h>
#include <string.h>

#include <sched.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "fwk/cli/cli.h"
int cmd_test(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "called %s with \"%s\"", __func__, command);
    cli_print(cli, "%d arguments:", argc);
    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    printf("cmd_test called\n");

    return CLI_OK;
}
int cmd_test_2(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "execute command: %s ",command);

    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    printf("cmd_test_2 was called with command: %s ",command);
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    return CLI_OK;
}
void* cliDemo(void* args)
{
    (void)args;
    (void)efwk_cli_init();
    efwk_cli_register_command("test",cmd_test,"My registe test function");
    efwk_cli_register_command("test_2",cmd_test_2,"My registe test function_2");
    efwk_cli_start();
    return NULL;
}
void* mainLoop(void* args)
{
    (void)args;
    sleep(10);
    return NULL;
}
int initCliSvr()
{
  fwk_taskID_t tid;
  fwk_taskAttr_t tAttr = {"cliSvr", cliDemo, mainLoop, NULL, SCHED_FIFO, 50, {0, 20*1024, 0, 0}, 1, 0, -1};
  int rc = fwk_createTask(&tAttr, &tid);
  printf("cliServer tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);
  return rc;
}

extern unsigned long int fwk_rawTid(fwk_taskID_t tid);

int randomN(int N)
{
  struct timeval tpstart;
  gettimeofday(&tpstart, NULL);
  srand(tpstart.tv_usec);

  float num = N*rand()/(RAND_MAX);
  return (int)(N + num);
}

fwk_queueID_t gQid = NULL;

void* gMid = NULL;
void* gSid = NULL;
int gTaskGui = 1;
int gCounter = 0;

void* consumer(void* args)
{
  int rc = fwk_takeSemaphore(gSid, -1);
  if (rc) {
  	printf("takeSemaphore failed: rc %i, errno %i\n", rc, errno);
  }
  printf("%s: gCount=%i\n", __func__, --gCounter);
  sleep(randomN(2));
  return NULL;
}

void* producer(void* args)
{
//bearer,  fabricant,  begetter, generator
  int rc = fwk_giveSemaphore(gSid);
  if (rc) {
  	printf("giveSemaphore failed: rc %i, errno %i\n", rc, errno);
  }
  printf("%s: gCount=%i\n", __func__, ++gCounter);
  sleep(randomN(3));
  return NULL;
}

void* master(void* args)
{
  int rc = fwk_lockMutex(gMid, -1);
  if (rc) {
  	printf("lockMutex failed: rc %i, errno %i\n", rc, errno);
  }
  printf("%s: gCount=%i\n", __func__, ++gCounter);
  sleep(randomN(1));
  rc = fwk_unlockMutex(gMid);
  if (rc) {
  	printf("unlockMutex failed: rc %i, errno %i\n", rc, errno);
  }
  sleep(randomN(5));
  return NULL;
}

void* slave(void* args)
{
  int rc = fwk_lockMutex(gMid, -1);
  if (rc) {
  	printf("lockMutex failed: rc %i, errno %i\n", rc, errno);
  }
  printf("%s: gCount=%i\n", __func__, ++gCounter);
  sleep(randomN(1));
  rc = fwk_unlockMutex(gMid);
  if (rc) {
  	printf("unlockMutex failed: rc %i, errno %i\n", rc, errno);
  }
  sleep(randomN(5));
  return NULL;
}

void* msgA(void* args)
{
  static int sn = 0, rc = 0;
  int i;
  char msg[FWK_QUEUE_MSG_DEF_LEN*2];
  uint16_t len = 0;
  int timeout = -1;

  for (i = 0; i < randomN(3); ++i) {
    sprintf(msg, "%s:%i", __func__, ++sn);
    len = strlen(msg);
    rc = fwk_sendToQueue(gQid, msg, len, timeout);
    printf("%s sent with %i\n", msg, rc);
    sleep(randomN(2));
  }

  len = sizeof(msg);
  for (i = 0; i < randomN(2); ++i) {
    msg[0] = 0;
    rc = fwk_receiveFromQueue(gQid, msg, len, timeout);
    printf("%s received by %s with len %i\n", msg, __func__, rc);
    sleep(randomN(1));
  }
  return &rc;
}

void* msgB(void* args)
{
  static int sn = 0, rc = 0;
  int i;
  char msg[FWK_QUEUE_MSG_DEF_LEN*2];
  uint16_t len = 0;
  int timeout = -1;

  for (i = 0; i < randomN(3); ++i) {
    sprintf(msg, "%s:%i", __func__, ++sn);
    len = strlen(msg);
    rc = fwk_sendToQueue(gQid, msg, len, timeout);
    printf("%s sent with %i\n", msg, rc);
    sleep(randomN(2));
  }

  len = sizeof(msg);
  for (i = 0; i < randomN(2); ++i) {
    msg[0] = 0;
    rc = fwk_receiveFromQueue(gQid, msg, len, timeout);
    printf("%s received by %s with len %i\n", msg, __func__, rc);
    sleep(randomN(1));
  }
  return &rc;
}

void* evtA(void* args)
{
  static int sn = 0, rc = 0;
  int i;
  int timeout = -1;
  fwk_taskID_t dest;

  char buf[FWK_QUEUE_MSG_DEF_LEN*3];
  fwk_event_t* event = (fwk_event_t*)buf;
  char* msg = buf + FWK_QUEUE_MSG_DEF_LEN - sizeof(fwk_event_t);

  for (i = 0; i < randomN(2); ++i) {
    sprintf(msg, "%s:%i", __func__, ++sn);
    fwk_eventType_t eventType = i%2;
    dest = fwk_taskId("evtB");
    if (dest) {
      rc = fwk_task_sendEvent(dest, eventType, msg, strlen(msg), timeout);
      printf("%s, %i sent with %i\n", msg, eventType, rc);
    }
    sleep(randomN(3));
  }

  for (i = 0; i < randomN(3); ++i) {
    rc = fwk_task_receiveEvent(event, timeout);
    printf("event[%s, %i, 0x%x, %i, %"fwk_addr_f"] received by %s with rc %i, timeout %i\n",
      (char*)event->data, event->eventType, event->datalen, event->sourceSys, (fwk_addr_t)event->sourceTask, __func__, rc, timeout);
    //if (!rc || event.data) fwk_memmgmt_free(event.data);
    //event.data = NULL;
    sleep(randomN(1));
  }
  return &rc;
}

void* evtB(void* args)
{
  static int sn = 0, rc = 0;
  int i;
  int timeout = 0;//-1;
  fwk_taskID_t dest;
#if 1
  char buf[FWK_QUEUE_MSG_DEF_LEN*3];
  fwk_event_t* event = (fwk_event_t*)buf;
  char* msg = buf + FWK_QUEUE_MSG_DEF_LEN - sizeof(fwk_event_t);
#else
  fwk_event_t event;
  char msg[FWK_QUEUE_MSG_DEF_LEN - sizeof(fwk_event_t)];
#endif
  for (i = 0; i < randomN(3); ++i) {
    sprintf(msg, "%s:%i", __func__, ++sn);
    fwk_eventType_t eventType = i%5;
    dest = fwk_taskId("evtA");
    if (dest) {
      rc = fwk_task_sendEvent(dest, eventType, msg, strlen(msg), timeout);
      printf("%s, %i sent with %i\n", msg, eventType, rc);
    }
    sleep(randomN(4));
  }

  for (i = 0; i < randomN(4); ++i) {
    rc = fwk_task_receiveEvent(event, timeout);
    printf("event[%s, %i, 0x%x, %i, %"fwk_addr_f"] received by %s with rc %i, timeout %i\n",
      (char*)event->data, event->eventType, event->datalen, event->sourceSys, (fwk_addr_t)event->sourceTask, __func__, rc, timeout);
      //(char*)event.data, event.eventType, event.datalen, event.sourceSys, (fwk_addr_t)event.sourceTask, __func__, rc, timeout);
    //if (!rc || event.data) fwk_memmgmt_free(event.data);
    //event.data = NULL;
    sleep(randomN(2));
  }
  return &rc;
}

void* arbitor(void* args)
{
  return NULL;
}

void* background(void* args)
{
  return NULL;
}

void* initMutex(void* args)
{
  int rc = 0;
  rc = fwk_createMutex(NULL, &gMid);
  if (rc) {
  	printf("create mutex failed: rc %i, errno %i\n", rc, errno);
  }
  return NULL;
}

void* initSema(void* args)
{
  int rc = 0;
  rc = fwk_createSemaphore(NULL, &gSid);
  if (rc) {
  	printf("create semaphore failed: rc %i, errno %i\n", rc, errno);
  }
  return NULL;
}

void* initQueue(void* args)
{
  uint8_t depth = 8;
  uint16_t size = FWK_QUEUE_MSG_DEF_LEN;
  static int rc = 0;
  rc = fwk_createFixsizeQueue(&gQid, NULL, depth, size);
  return &rc;
}

void* preFunc(void* args)
{
  static int rc = 0;
  initMutex(NULL);
  initSema(NULL);
  rc += *(int*)initQueue(NULL);
  rc += initCliSvr();
  return &rc;
}

void* postFunc(void* args)
{
  int rc = 0, i, retry = 2; //200

  rc = fwk_clearQueue(gQid);
  if (rc) {
  	printf("release queue failed: rc %i, errno %i\n", rc, errno);
  }
#if 0
  rc = fwk_task_clearAllEvent();
  if (rc) {
  	printf("release event failed: rc %i, errno %i\n", rc, errno);
  }
  extern int fwk_clearTask();
  rc = fwk_clearTask();
  if (rc) {
  	printf("clear tasks failed: rc %i, errno %i\n", rc, errno);
  }
#endif
  for (i = 0; i < retry; ++i) {
	  rc = fwk_deleteMutex(gMid);
	  if (rc) {
	  	printf("release mutex failed: rc %i, errno %i, retry %i\n", rc, errno, i);
	  } else {
		  break;
	  }
	  usleep(10*1000);
  }

  for (i = 0; i < retry; ++i) {
	  rc = fwk_deleteSemaphore(gSid);
	  if (rc) {
	  	printf("release semaphore failed: rc %i, errno %i, retry %i\n", rc, errno, i);
	  } else {
		  break;
	  }
	  usleep(10*1000);
  }
  return NULL;
}

void* timer(void* args)
{
  static int i = 0;
  fwk_taskID_t tid = fwk_myTaskId();
  unsigned long int pid = fwk_rawTid(tid);
  if (gTaskGui) printf("%s[%ld]: %i\n", __func__, pid, ++i);
  sleep(2);
  return NULL;
}

int gTimeATick = 0;
int gTimeBTick = 0;
void a_repeating_timer (stw_tmr_t *tmr, void *parm) 
{
    int cnt = *(int*)parm + 1;
    *(int*)parm = cnt;
    if (cnt % 10 == 0) {
      printf("TimerA cb: %s timer a count=%i\n", __FUNCTION__, cnt);
      //stw_timer_stats();
    } 
} 

void b_timer (void *parm)
{
    printf("TimerB cb: %s timer b count=%i\n", __FUNCTION__, ++*(int*)parm);
}

void* timerClient(void* args)
{
  char buf[FWK_QUEUE_MSG_DEF_LEN*2];
  fwk_event_t* event = (fwk_event_t*)buf;
  int timeout = FOREVER;
  int len = fwk_task_receiveEvent(event, timeout);
  printf("event[%s, %i, 0x%x, %i, %"fwk_addr_f"] received by %s with len %i, timeout %i\n",
    (char*)event->data, event->eventType, event->datalen, event->sourceSys, (fwk_addr_t)event->sourceTask, __func__, len, timeout);
  gTimeBTick += 1;
  if (len < 0) sleep(5);
  return NULL;
}

int initTimerDemo()
{
  int rc = 0;
  fwk_taskID_t tid;
  extern void * tmr_main_task(void * ptr);
  fwk_taskAttr_t tAttr = {"timerM", NULL, tmr_main_task, NULL, SCHED_FIFO, 50, {0, 20*1024, FWK_QUEUE_MSG_DEF_LEN, 8}, 1, 0, -1};
  rc = fwk_createTask(&tAttr, &tid);
  printf("timer_main_task tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  fwk_taskAttr_t tcAttr = {"tmClient", NULL, timerClient, NULL, SCHED_FIFO, 50, {0, 20*1024, FWK_QUEUE_MSG_DEF_LEN, 8}, 1, 0, -1};
  rc = fwk_createTask(&tcAttr, &tid);
  printf("timer_client tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  tmr_timer_init();
  //rc = stw_timer_create(STW_NUMBER_BUCKETS,STW_RESOLUTION, "Demo Timer Wheel");

  uint32_t delay = NTERVAL_PER_SECOND/10;//10 times in 1 second
  uint32_t periodic_delay = NTERVAL_PER_SECOND/10;//10 times in 1 second
  //add timer node with name TimerA & TimerB
  rc = tmr_add_timerNode("TimerA",delay,periodic_delay,NULL, (void *)a_repeating_timer, &gTimeATick);
  delay = 100 * SYS_TIME_TICKS_IN_A_SEC;// 1 times in 1 second
  periodic_delay = 100 * SYS_TIME_TICKS_IN_A_SEC; // 1 times in 1 second 
  rc = tmr_add_timerNode("TimerB",delay,periodic_delay, tid, (void *)b_timer, &gTimeBTick);

  rc = tmr_start_timer("TimerA");
  rc = tmr_start_timer("TimerB");
  return rc;
}

int cmdMain(int argc, char** argv)
{
  char buf[FWK_QUEUE_MSG_DEF_LEN];
  char* name = NULL;
  fwk_taskID_t tid = 0;
  int timeout = 0;
  int rc = 0;
  int i;

  if (argc < 1) return -1;
  char* cmd = argv[0];
  char* cmdList[] = {
    "quit: quit this tool",
    "echo: display input in turn",
    "help: show this info list",
    "taskCreate: create timer thread with assigned parameters",
    "taskDelete: delete thread, arg1 specify taskName",
    "taskPause: pause thread, arg1 specify taskName",
    "taskResume: resume thread, arg1 specify taskName",
    "taskName: show thread name, arg1 specify taskName",
    "taskShow: show thread list",
    "mutexDemo: create mutex tasks: master vs. slave",
    "mutexShow: show mutex list",
    "mutexLock: lock mutex",
    "mutexUnlock: unlock mutex",
    "semaDemo: create semaphore tasks: consumer vs. producer",
    "semaShow: show semaphore list",
    "queueDemo: create queue sender and receiver: msgA, msgB",
    "msgSend: send msg to queue of msgA and msgB",
    "msgRecv: receive msg from queue of msgA and msgB",
    "queueShow: show queue list",
    "eventDemo: create event tasks: evtA, evtB",
    "eventSend: send event to evtA and evtB",
    "eventShow: show event list",
    "timerDemo: initialize timer wheel and start timer",
    "timerStart: start timer ",
    "timerStop: stop timer",
    "",
  };

  if (!strcmp(cmd, "q") || !strcmp(cmd, "quit") || !strcmp(cmd, "exit")) {
    rc = 886;

  } else if (!strcmp(cmd, "echo")) {
    for (i = 1; i < argc; ++i) {
      printf("%s ", argv[i]);
    }
    printf("\n");

  } else if (!strcmp(cmd, "help")) {
    for (i = 0; i < sizeof(cmdList)/sizeof(cmdList[0]); ++i) {
      printf("%s\n", cmdList[i]);
    }

  } else if (!strcmp(cmd, "taskCreate")) {
    uint8_t priority = 1;
    uint8_t policy = SCHED_FIFO;
    fwk_taskRes_t resource;
    resource.stackSize = 1024*20;
    bool_t independent = 1;
    rc = fwk_createPreemptiveTask("timer", &tid, timer, NULL, priority, policy, resource, independent);
    printf("pid:%ld\n", fwk_rawTid(tid));
    printf("tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "taskDelete")) {
    if (argc > 1) {
      name = argv[1];
    } else {
      name = "timer";
    }
    tid = fwk_taskId(name);
    printf("pid:%ld\n", fwk_rawTid(tid));
    rc = fwk_deleteTask(tid);
    printf("delete %s tid: %"fwk_addr_f"\n", name, (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "taskPause")) {
    if (argc > 1) {
      name = argv[1];
    } else {
      name = "timer";
    }
    tid = fwk_taskId(name);
    rc = fwk_suspendTask(tid);
    printf("pid:%ld\n", fwk_rawTid(tid));
    printf("tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "taskResume")) {
    if (argc > 1) {
      name = argv[1];
    } else {
      name = "timer";
    }
    tid = fwk_taskId(name);
    rc = fwk_resumeTask(tid);
    printf("pid:%ld\n", fwk_rawTid(tid));
    printf("tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "taskName")) {
    if (argc > 1) {
      name = argv[1];
    } else {
      name = "timer";
    }
    tid = fwk_taskId(name);
    const char* tName = fwk_taskName(tid);
    printf("tid: %"fwk_addr_f", pid:%ld, taskName:%s\n", (fwk_addr_t)tid, fwk_rawTid(tid), tName);

  } else if (!strcmp(cmd, "taskShow")) {
    fwk_showTask(NULL);

  } else if (!strcmp(cmd, "mutexDemo")) {
    fwk_taskAttr_t MAttr = {"master", NULL, master, NULL, SCHED_FIFO, 80, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&MAttr, &tid);
    printf("master tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

    fwk_taskAttr_t SAttr = {"slave", NULL, slave, NULL, SCHED_FIFO, 20, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&SAttr, &tid);
    printf("slave tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "mutexShow")) {
    fwk_showMutex(NULL);
  } else if (!strcmp(cmd, "mutexLock")) {
    if (argc > 1) {
      timeout = strtol(argv[1], NULL, 0);
    }
    rc = fwk_lockMutex(gMid, timeout);
  } else if (!strcmp(cmd, "mutexUnlock")) {
    rc = fwk_unlockMutex(gMid);

  } else if(!strcmp(cmd, "semaDemo")) {
    fwk_taskAttr_t CAttr = {"consumer", NULL, consumer, NULL, SCHED_FIFO, 50, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&CAttr, &tid);
    printf("consumer tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

    fwk_taskAttr_t PAttr = {"producer", NULL, producer, NULL, SCHED_FIFO, 10, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&PAttr, &tid);
    printf("producer tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "semaShow")) {
    fwk_showSemaphore(NULL);

  } else if (!strcmp(cmd, "queueDemo")) {
    fwk_taskAttr_t AAttr = {"msgA", NULL, msgA, NULL, SCHED_FIFO, 80, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&AAttr, &tid);
    printf("msgA tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

    fwk_taskAttr_t BAttr = {"msgB", NULL, msgB, NULL, SCHED_FIFO, 20, {0, 20*1024, 0, 10}, 1, 0, -1};
    rc = fwk_createTask(&BAttr, &tid);
    printf("msgB tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if (!strcmp(cmd, "msgSend")) {
    if (argc > 1) {
      strcpy(buf, argv[1]);
    } else {
      strcpy(buf, "Empty");
    }
    timeout = (argc > 2) ? strtol(argv[2], NULL, 0) : -1;
    rc = fwk_sendToQueue(gQid, buf, strlen(buf), timeout);

  } else if (!strcmp(cmd, "msgRecv")) {
    timeout = (argc > 1) ? strtol(argv[1], NULL, 0) : -1;
    rc = fwk_receiveFromQueue(gQid, buf, sizeof(buf), timeout);
    printf("Received len=%i, msg[%s]\n", rc, buf);

  } else if (!strcmp(cmd, "queueShow")) {
    fwk_showQueue(NULL);

  } else if(!strcmp(cmd, "eventDemo")) {
    fwk_taskAttr_t AAttr = {"evtA", NULL, evtA, NULL, SCHED_FIFO, 50, {0, 20*1024, FWK_QUEUE_MSG_DEF_LEN, 8}, 1, 0, -1};
    rc = fwk_createTask(&AAttr, &tid);
    //rc = fwk_task_createEvent(tid);
    printf("evtA tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

    fwk_taskAttr_t BAttr = {"evtB", NULL, evtB, NULL, SCHED_FIFO, 10, {0, 20*1024, FWK_QUEUE_MSG_DEF_LEN, 8}, 1, 0, -1};
    rc = fwk_createTask(&BAttr, &tid);
    //rc = fwk_task_createEvent(tid);
    printf("evtB tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);

  } else if(!strcmp(cmd, "eventSend")) {
    if (argc > 1) {
      strcpy(buf, argv[1]);
    } else {
      strcpy(buf, "evtA");
    }
    fwk_taskID_t dest = fwk_taskId(buf);
    fwk_eventType_t eventType = (fwk_eventType_t)(__LINE__);
    rc = fwk_task_sendEvent(dest, eventType, "Hello", 8, -1);

  //} else if (!strcmp(cmd, "eventShow")) {
  //  fwk_task_showEvent();

  } else if(!strcmp(cmd, "timerDemo")) {
    rc = initTimerDemo();
  } else if(!strcmp(cmd, "timerStart")) {
    if (argc > 1) {
      rc = tmr_start_timer(argv[1]);
    } else {
      printf("Please append timer name.\n");
    }
  } else if(!strcmp(cmd, "timerStop")) {
    if (argc > 1) {
      rc = tmr_stop_timer(argv[1]);
    } else {
      printf("Please append timer name.\n");
    }
  } else if(!strcmp(cmd, "timerShow")) {
    if (argc > 1) {
      rc = tmr_show_timerNode(argv[1]);
    } else {
      printf("Please append timer name.\n");
    }
  } else if(!strcmp(cmd, "timerDelete")) {
    if (argc > 1) {
      rc = tmr_delete_timerNode(argv[1]);
      //rc = stw_timer_destroy();
    } else {
      printf("Please append timer name.\n");
    }

  } else {
    printf("Unsupported command %s\n", cmd);
    rc = -2;
  }
  return rc;
}

int main(int argc, char** argv)
{
#define ARGS_CNT        8             /* Max argv entries */
#define ARGS_BUFFER  256         /* bytes total for arguments */
  int rc = 0;
  char cmdLine[ARGS_BUFFER];
  char ch = 0;
  char* cmdArr[ARGS_CNT];

  preFunc(NULL);
  for (;;) {
    cmdLine[0] = 0;
    printf("cmd>");
    scanf("%[^\n]", cmdLine);
    scanf("%c", &ch);
    if (!strlen(cmdLine)) continue;

    int i = 0;
    int k = 0;   // key index
    int n = 0;   // args number
    int len = 0; // args length

    while (cmdLine[i]) {
      if (cmdLine[i] == '\t' || cmdLine[i] == ' ') {
        if (len) {
          cmdArr[n] = &cmdLine[k];
          cmdLine[i] = 0;  //Terminate the argment
          len = 0;
          ++n;
        }
      } else {
        if (len) {
          ++len;
        } else {
          k = i;
          len = 1;
        }
      }
      ++i;
    }
    if (len) {
      cmdArr[n] = &cmdLine[k];
      ++n;
    }
    rc = cmdMain(n, cmdArr);
    if (rc == 886) break;
    printf("Execute: [");
    for (i = 0; i < n; ++i) {
      printf("%s ", cmdArr[i]);
    }
    printf("], Finished with rc %i\n", rc);
  }
  postFunc(NULL);
  return rc;
}
