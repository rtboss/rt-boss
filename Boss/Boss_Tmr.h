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
void _Boss_timer_notify_set( void (*notify)(void) );

void Boss_tmr_start(boss_tmr_t *p_tmr, boss_tmr_ms_t tmr_ms, tmr_cb_t callback);
void Boss_tmr_stop(boss_tmr_t *p_tmr);

#endif  /* _BOSS_TMR_H_ */
