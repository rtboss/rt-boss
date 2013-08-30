/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                               RT-BOSS (SPY)                                 *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
#include "Boss.h"

#ifdef _BOSS_SPY_
/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss_SPY.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/


/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/
/*static*/ boss_u32_t  _spy_elapse_curr_us  = 0;        /* 진행 시간 (us)  */
/*static*/ boss_u32_t  _spy_elapse_watch_us = 0;        /* 이전 시간 (us)  */

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
boss_u32_t Boss_spy_cpu_timer_us(void);
void Boss_spy_stack_check(boss_tcb_t *p_tcb);


/*===========================================================================
    _   B O S S _ S P Y _ E L A P S E _ T I C K
---------------------------------------------------------------------------*/
void _Boss_spy_elapse_tick(boss_u32_t tick_ms)
{
  _spy_elapse_curr_us += (boss_u32_t)tick_ms * (boss_u32_t)1000;  /* tick_ms -> us */
  
  if(_spy_elapse_curr_us < _spy_elapse_watch_us) {
    _spy_elapse_watch_us = _spy_elapse_curr_us;
  }
}

/*===========================================================================
    B O S S _ S P Y _ E L A P S E _ U S
---------------------------------------------------------------------------*/
boss_u32_t Boss_spy_elapse_us(void)
{
  boss_u32_t elapse_us;

  elapse_us = _spy_elapse_curr_us + Boss_spy_cpu_timer_us();
  
  return elapse_us;
}

/*===========================================================================
    _   B O S S _ S P Y _ W A T C H _ U S
---------------------------------------------------------------------------*/
static boss_u32_t _Boss_spy_stopwatch_us(void)
{
  boss_u32_t watch_us = 0;
  boss_u32_t elapse_us = Boss_spy_elapse_us();

  if(elapse_us > _spy_elapse_watch_us)
  {
    watch_us = elapse_us - _spy_elapse_watch_us;
    _spy_elapse_watch_us = elapse_us;
  }

  return watch_us;
}


/*===========================================================================
    B O S S _ S P Y _ E L A P S E _ P R E V _ U S
---------------------------------------------------------------------------*/
boss_u32_t Boss_spy_elapse_prev_us(void)
{
  return _spy_elapse_watch_us;
}


/*===========================================================================
    _   B O S S _ S P Y _ C O N T E X T
---------------------------------------------------------------------------*/
void _Boss_spy_context(boss_tcb_t *curr_tcb, boss_tcb_t *best_tcb)
{
  /* [ C P U ] */
  curr_tcb->ex.run_time = curr_tcb->ex.run_time + _Boss_spy_stopwatch_us();
  
  /* [ Context Switch Number ] */
  curr_tcb->ex.context++;

  /* [ Stack ] */
  Boss_spy_stack_check(curr_tcb);
}


/*===========================================================================
    _   B O S S _ S P Y _ S E T
---------------------------------------------------------------------------*/
void _Boss_spy_set(boss_tcb_t *p_tcb, boss_stk_t *sp_base, boss_uptr_t bytes)
{
  boss_uptr_t size  = bytes / sizeof(boss_stk_t);

  p_tcb->ex.sp_base  = &sp_base[0];
  p_tcb->ex.sp_peak  = &sp_base[size-1];
  p_tcb->ex.sp_limit = &sp_base[size];
  
  p_tcb->ex.run_time = 0;     /* Task run-time sum (us) */
  p_tcb->ex.context  = 0;     /* Context Switch Number  */
}


/*===========================================================================
    B O S S _ S P Y _ R E S T A R T
---------------------------------------------------------------------------*/
void Boss_spy_restart(void)
{
  boss_tcb_t *p_tcb;
  
  _Boss_sched_lock();

  BOSS_IRQ_DISABLE();
  _spy_elapse_curr_us   = 0;
  _spy_elapse_watch_us  = 0;
  BOSS_IRQ_RESTORE();
  
  p_tcb = Boss_ex_task_list(0);
  while(p_tcb != _BOSS_NULL)
  {
    p_tcb->ex.run_time = 0;
    p_tcb->ex.context  = 0;
    p_tcb = p_tcb->ex_task_link;
  }
  _Boss_sched_free();
}
#endif /* _BOSS_SPY_ */
