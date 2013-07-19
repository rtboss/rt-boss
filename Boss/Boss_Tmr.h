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

typedef void (*boss_tmr_cb_t)(void);

typedef struct boss_tmr_struct {
  struct boss_tmr_struct  *next;
  
  boss_tcb_t        *p_tcb;         /* TCB                         */
  boss_sigs_t       sig;            /* Signal                      */
  
  boss_tmr_ms_t     tmr_ms;         /* Timer Countdown Millisecond */
  boss_tmr_ms_t     rpt_ms;         /* Repeat Count Millisecond    */
  
  boss_tmr_cb_t     callback;       /* Callback Function           */
} boss_tmr_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_timer_tick(boss_tmr_ms_t tick_ms);

boss_tmr_t *Boss_tmr_start( boss_tcb_t *p_tcb, boss_sigs_t sig,
            boss_tmr_ms_t tmr_ms, boss_tmr_ms_t rpt_ms, boss_tmr_cb_t callback );

boss_reg_t Boss_tmr_stop(boss_tmr_t *p_tmr);
boss_reg_t Boss_tmr_cancel(boss_tcb_t *p_tcb, boss_sigs_t sig,
                                                        boss_tmr_cb_t callback);

void Boss_sleep(boss_tmr_ms_t wait_ms);
boss_sigs_t Boss_wait_sleep(boss_sigs_t wait_sigs,  boss_tmr_ms_t wait_ms);

#endif  /* _BOSS_TMR_H_ */
