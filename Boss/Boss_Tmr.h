#ifndef _BOSS_TMR_H_
#define _BOSS_TMR_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                             RT-BOSS (Timer)                                 *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
#define BOSS_TMR_INFINITE               (~(boss_tmr_ms_t)0)

typedef struct boss_tmr_struct {
  struct boss_tmr_struct  *prev;
  struct boss_tmr_struct  *next;

  boss_tmr_ms_t           tmr_ms;             /* Timer Countdown Millisecond  */
  void  (*tmr_cb)(struct boss_tmr_struct *);  /* Timer Execute Callback       */
} boss_tmr_t;

typedef void (*tmr_cb_t)(boss_tmr_t *);   // Timer Callback Type

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_timer_tick(boss_tmr_ms_t tick_ms);
void _Boss_timer_callback_execute(void);
void _Boss_timer_cb_task_set(boss_tcb_t *p_cb_tcb, boss_sigs_t cb_sig);

void Boss_tmr_start(boss_tmr_t *p_tmr, boss_tmr_ms_t tmr_ms, tmr_cb_t callback);
void Boss_tmr_stop(boss_tmr_t *p_tmr);

void Boss_sleep(boss_tmr_ms_t wait_ms);
boss_sigs_t Boss_wait_sleep(boss_sigs_t wait_sigs,  boss_tmr_ms_t wait_ms);

#endif  /* _BOSS_TMR_H_ */
