/*------------------------------------------------------------------
 * stw_timer.h -- Definitions for Single Timer Wheel
 *------------------------------------------------------------------
 */

#ifndef __STW_TIMER_H__
#define __STW_TIMER_H__

#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * This file provides the public definitions for external users of the
 * Timer Wheel Timer facility.
 *
 * The Single Timer Wheel Timer facility is optimized to support embedded
 * timer structures, ie where the timer structure is integrated into the
 * structure it is associated with.
 *
 * Definitions
 *
 * Spoke - A queue of timers set to expire. A timer wheel consists of 
 *  multiple spokes. 
 *
 * Granularity - The amount of time between the processing of each
 *  spoke.   Granularity is measured in time units between ticks. 
 *
 * Rotation - one complete turn around the wheel
 *
 * Timer Duration given 32 bits
 *    Time per Tick      Duration 
 *   ------------------------------------
 *        10ms          497.1 days
 *        20ms          994.2
 *        50ms         2485.5
 *
 *   milliseconds per day [1000 * 60 * 60 * 24]
 *
 */

typedef enum {
    RC_STW_OK = 0,
    RC_STW_NULL_NAME,
    RC_STW_NULL_FV,
    RC_STW_NULL_WHEEL,
    RC_STW_NULL_TMR,
    RC_STW_INVALID_WHEEL,
    RC_STW_INVALID_WHEEL_SIZE,
    RC_STW_INVALID_GRANULARITY,
    RC_STW_NO_RESOURCES,
} RC_STW_t;

/*
 * we need to put some bounds to protect against extremely
 * large numbers.  Ranges selected from pratical experience.
 */

/*
 * range of valid wheel sizes
 */
#define STW_MIN_WHEEL_SIZE    (   32 )
#define STW_MAX_WHEEL_SIZE    ( 4096 )

/*
 * Granularity of a timer tick in milliseconds   
 */
#define STW_MIN_GRANULARITY   (   1 )
#define STW_MAX_GRANULARITY   ( 100 )

#define STW_NUMBER_BUCKETS     ( 512 ) 
#define STW_RESOLUTION         ( 100 ) 

/*
 * stw_links
 *  Definition of the pointers used to link a timer into
 *  a spoke.  Double-linked list for efficiency.
 */
typedef struct stw_links_t_ {
    struct stw_links_t_ *stw_next;
    struct stw_links_t_ *stw_prev;
} stw_links_t;

/*
 * Timer Wheel Structure used to manage the timer wheel
 * and keep stats to help understand performance
 */
#define STW_NAME_LENGTH   ( 32 )
#define STW_MAX_TIMER_NUM (50)

typedef struct {
    char  wheel_name[ STW_NAME_LENGTH ];
    uint32_t  magic_tag;           /* for sanity check */
    uint32_t  wheel_size;
    uint32_t  spoke_index;         /* mod index around wheel */
    uint32_t  ticks;               /* absolute ticks */
    uint32_t  granularity;         /* millisecond per tick */

    /*
     * few book keeping parameters to help engineer the wheel
     */
    uint32_t  timer_hiwater_mark;
    uint32_t  timer_active;
    uint32_t  timer_cancelled;
    uint32_t  timer_expired;
    uint32_t  timer_starts;

    stw_links_t  *spokes;
} stw_t;

struct stw_tmr_t;
/*
 * Application call-back type to be invoked at timer
 * expiration.  The call-back must be short-n-sweet,
 * non-blocking.
 */
typedef void (*stw_call_back_t)(struct stw_tmr_t *tmr, void *parm);

/*
 * stw_tmr_t
 *  Definition of a timer element.
 *  This can be malloc'ed or embedded into an existing
 *  application structure.
 */
typedef struct stw_tmr_t {
    stw_links_t links;
    uint32_t    rotation_count;
    uint32_t    delay;            /* initial delay       */
    uint32_t    periodic_delay;   /* auto-restart if > 0 */
    void        *tid;
    stw_call_back_t func_ptr;
    void        *parm;
} stw_tmr_t;

/*
 * stw_tmr_Node_t
 *  Definition of a timer element in timer list.
 *  This can be malloc'ed or embedded into an existing
 *  application structure.
 */
typedef struct {
    bool_t       used;
    stw_t           *stw;
    char            timer_name[STW_NAME_LENGTH];
    //bool_t       threadInfo_valid;
    void* tid;
    stw_call_back_t func_ptr;
    //bool_t       parm_valid;
    void            *parm;
    stw_tmr_t       tmr;    
} stw_tmr_Node_t;

/*
 * Unlock the mutex for timer tick.
 */
extern int stw_unlock_mutex(void);

/*
 * Lock the mutex for timer tick.
 */
extern int stw_lock_mutex(void);

/*
 * Initial the mutex for timer tick.
 */
extern void stw_init_mutex(void);

/*
 * Destroy the mutex for timer tick.
 */
extern void stw_destroy_mutex(void);

#define LOCK_INTERRUPTS() stw_lock_mutex()
#define UNLOCK_INTERRUPTS() stw_unlock_mutex()

/*
 * stw_timer_running
 *  Returns TRUE if the timer is active
 */
extern bool_t
stw_timer_running(stw_tmr_t *tmr);

/*
 * stw_timer_prepare
 *  Utility routine to initialize the links of timer elements.
 */
extern void
stw_timer_prepare(stw_tmr_t *tmr);

/*
 * Displays timer wheel stats and counters to stdout.
 */
extern void
stw_timer_stats(const stw_t *stw);

/*
 * Starts a new timer.  If the timer is currently running,
 * it is stopped and restarted anew
 */
extern RC_STW_t
stw_timer_start(stw_t  *stw,
            stw_tmr_t  *tmr,
             uint32_t   delay,
             uint32_t   periodic_delay,
                void   *tid,
      stw_call_back_t   user_cb,
                void   *parm);

/*
 * stops a currently running timer
 */
extern RC_STW_t
stw_timer_stop(stw_t *stw, stw_tmr_t *tmr);

/*
 * Timer Wheel tick handler which drives time for the
 * specified wheel
 */
extern void
stw_timer_tick(stw_t *stw);

/*
 * Destroys a timer wheel
 */
extern RC_STW_t
stw_timer_destroy(stw_t *stw);

/*
 * creates and initializes a single timer wheel
 */
extern RC_STW_t
stw_timer_create(uint32_t   wheel_size,
                 uint32_t   granularity,
               const char  *name,
                    stw_t **stw);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STW_TIMER_H__ */

