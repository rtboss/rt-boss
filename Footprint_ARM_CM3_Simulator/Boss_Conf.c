/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                 RT-BOSS (Config File)         [ Cortex-M3 ]                 *
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

/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/

/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                            RT-BOSS ( SPY  )                                 *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
#ifdef _BOSS_SPY_
/*===========================================================================
    _   B O S S _ S P Y _ C P U _ T I M E R _ U S
---------------------------------------------------------------------------*/
boss_u32_t Boss_spy_cpu_timer_us(void)
{
  boss_u32_t us;
  boss_u32_t reload = SysTick->LOAD;
  boss_u32_t value  = reload - SysTick->VAL;   /* count-down value */
  
                                /* SysTick->VAL => micro second */
  us = (value * ((boss_u32_t)_BOSS_TICK_MS_ * (boss_u32_t)1000)) / (reload + 1);

  return us;
}

/*===========================================================================
    B O S S _ S P Y _ S T A C K _ C H E C K
---------------------------------------------------------------------------*/
void Boss_spy_stack_check(boss_tcb_t *p_tcb)
{
  BOSS_ASSERT(p_tcb->ex.sp_base[0] == (boss_stk_t)0xEEEEEEEE);  // Stack crack  
  BOSS_ASSERT(p_tcb->ex.sp_base[1] == (boss_stk_t)0xEEEEEEEE);
  
  while( (p_tcb->ex.sp_peak[-1] != (boss_stk_t)0xEEEEEEEE) 
      || (p_tcb->ex.sp_peak[-2] != (boss_stk_t)0xEEEEEEEE)
      || (p_tcb->ex.sp_peak[-3] != (boss_stk_t)0xEEEEEEEE)
      || (p_tcb->ex.sp_peak[-4] != (boss_stk_t)0xEEEEEEEE) 
    #if 0
      || (p_tcb->ex.sp_peak[-5] != (boss_stk_t)0xEEEEEEEE)
      || (p_tcb->ex.sp_peak[-6] != (boss_stk_t)0xEEEEEEEE)
      || (p_tcb->ex.sp_peak[-7] != (boss_stk_t)0xEEEEEEEE)
      || (p_tcb->ex.sp_peak[-8] != (boss_stk_t)0xEEEEEEEE)
    #endif
      )
  {
    p_tcb->ex.sp_peak--;
    if(p_tcb->ex.sp_peak <= p_tcb->ex.sp_base)
    {
      BOSS_ASSERT(_BOSS_FALSE);             // Stack overflow
      for(;;);
    }
  }
  
  BOSS_ASSERT(p_tcb->ex.sp_limit[-2] == (boss_stk_t)0xEEEEEEEE);  // Stack crack
  BOSS_ASSERT(p_tcb->ex.sp_limit[-1] == (boss_stk_t)0xEEEEEEEE);
}
#endif /* _BOSS_SPY_ */
