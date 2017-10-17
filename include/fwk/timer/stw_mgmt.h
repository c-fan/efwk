/*------------------------------------------------------------------
 * stw_mgmt.h -- System Timer Wheel management
 *------------------------------------------------------------------
 */

#ifndef __STW_MGMT_H__
#define __STW_MGMT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <fwk/timer/stw_timer.h>

/*
If NANO_SLEEP_TIMER available,  timer will apply nanosleep to trigger the tick
Otherwise sigaction with alarm signal and setitimer with ITIMER_REAL to trigger the tick,
For the later, it relies on the mechanism of alarm signal in the system. So some time it is of inconsistency
Here we use nanosleep to trigger tick of timer
*/
#define NANO_SLEEP_TIMER
#ifdef NANO_SLEEP_TIMER
#define   SYS_TIME_TICKS_IN_A_SEC            100
#define   NTERVAL_PER_SECOND                 (100 * SYS_TIME_TICKS_IN_A_SEC)
#endif

/* Timer Types */
enum {
   STW_TICK_TMR_TYPE     = 1,
   STW_PERIODIC_TMR_TYPE = 2
};

/*
 * Unlock the mutex for timer.
 */
extern int tmr_unlock_mutex(void);

/*
 * Lock the mutex for timer.
 */
extern int tmr_lock_mutex(void);

/*
 * Initial the mutex for timer.
 */
extern void tmr_init_mutex(void);

/*
 * Destroy the mutex for timer.
 */
extern void tmr_destroy_mutex(void);

/*
 * Initial the whole static timer list.
 */
extern void tmr_timer_list_init();

/*
 * Initial the single timer task .
 */
extern void tmr_timer_init();

/*
 * Find the first available timer node from the timer list .
 */
extern bool_t tmr_find_free_tmrNode(char * timer_name,stw_tmr_Node_t **tmrNode);

/*
 * Find the specific timer node from the timer list .
 */
extern bool_t tmr_find_tmrNode(char * timer_name,stw_tmr_Node_t **tmrNode);

/*
 * Add a new timer to the timer list .
 */
extern bool_t tmr_add_timerNode(char *timer_name, uint32_t delay, uint32_t periodic_delay,
                                void *tid, void *user_cb, void *parm);

/*
 * Delete the specific timer from the timer list .
 */
extern bool_t tmr_delete_timerNode(char *timer_name);

/*
 * Show the specific timer from the timer list .
 */
extern bool_t tmr_show_timerNode(char *timer_name);

/*
 * Start the specific timer .
 */
extern bool_t tmr_start_timer(char *timer_name);

/*
 * Stop the specific timer .
 */
extern bool_t tmr_stop_timer(char *timer_name);

/*
 * Main timer task function.
 */
extern void * tmr_main_task(void * ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
