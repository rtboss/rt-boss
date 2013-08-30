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
boss_stk_t  *_boss_spy_msp_peak;    /* Cortex-Mx MSP(Main Stack Pointer) Peak */

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



/*===========================================================================
    B O S S _ S P Y _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_spy_report(void)
{
  boss_u32_t total_us = 0;
  
  boss_u32_t cpu_pct_sum = 0;
  boss_u32_t context_sum = 0;
  boss_tcb_t *p_tcb;

  _Boss_spy_msp_check();
  Boss_spy_msp_report();

  _Boss_sched_lock();
  PRINTF("\n[TASK]\t  STACK %%(u/t)\t  C P U    Context\n");
  PRINTF("------------------------------------------\n");
  
  total_us = Boss_spy_elapse_prev_us();

  p_tcb = Boss_ex_task_list(0);
  while( p_tcb != _BOSS_NULL )
  {
    PRINTF("%5s", p_tcb->name);

    { /* [ Stack ] */
      boss_uptr_t stk_total;
      boss_uptr_t stk_used;
      boss_reg_t  stk_pct;      /* percent xx % */
      
      stk_total = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_base;
      stk_used  = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_peak;
      stk_pct = (boss_reg_t)(((boss_u32_t)stk_used * 100) / (boss_u32_t)stk_total);
      
      PRINTF("\t  %2d%%(%3d/%3d)", stk_pct, stk_used, stk_total);
    }

    { /* [ C P U ] */
      boss_u32_t cpu_pct = 0;     /* percent XX.xxx % */

      cpu_pct = (boss_u32_t)(((boss_u64_t)(p_tcb->ex.run_time) * (boss_u64_t)100000)
                                                    / (boss_u64_t)total_us);
      
      PRINTF("\t %2d.%03d%%", (int)(cpu_pct/1000), (int)(cpu_pct%1000));
      cpu_pct_sum = cpu_pct_sum  + cpu_pct;
    }
    
    PRINTF("   %7d\n", p_tcb->ex.context);
    context_sum = context_sum + p_tcb->ex.context;

    p_tcb = p_tcb->ex_task_link;    // Next Task link
  }

  PRINTF("[TOTAL] :\t\t %2d.%03d%%   %7d\n\n",
          (int)(cpu_pct_sum/1000), (int)(cpu_pct_sum%1000), context_sum);

  PRINTF("   total_us = %d\n", total_us);
  PRINTF("   SysTick->LOAD = %d\n", SysTick->LOAD);
  
  _Boss_sched_free();
}


#if defined ( __CC_ARM )                                /*!< KEIL Compiler >!*/
/*===========================================================================
    _   B O S S _ S P Y _ M S P _ L I M I T
---------------------------------------------------------------------------*/
static boss_stk_t *_Boss_spy_msp_limit(void)
{
  extern const boss_u32_t STACK$$Limit;

  return (boss_stk_t *)&STACK$$Limit;
}

/*===========================================================================
    _   B O S S _ S P Y _ M S P _ B A S E
---------------------------------------------------------------------------*/
static boss_stk_t *_Boss_spy_msp_base(void)
{
  extern const boss_u32_t STACK$$Base;

  return (boss_stk_t *)&STACK$$Base;
}


#elif defined ( __ICCARM__ )                            /*!< IAR Compiler >!*/
/*===========================================================================
    _   B O S S _ S P Y _ M S P _ L I M I T
---------------------------------------------------------------------------*/
static boss_stk_t *_Boss_spy_msp_limit(void)
{
  extern const boss_u32_t CSTACK$$Limit;

  return (boss_stk_t *)&CSTACK$$Limit;
}

/*===========================================================================
    _   B O S S _ S P Y _ M S P _ B A S E
---------------------------------------------------------------------------*/
static boss_stk_t *_Boss_spy_msp_base(void)
{
  extern const boss_u32_t CSTACK$$Base;

  return (boss_stk_t *)&CSTACK$$Base;
}


#else
  #error "Compiler Check!"
#endif


/*===========================================================================
    _   B O S S _ S P Y _ M S P _ S E T U P
---------------------------------------------------------------------------*/
void _Boss_spy_msp_setup(void)
{
  boss_stk_t *msp_curr = (boss_stk_t *)__get_MSP(); /* Get Current Main Stack Pointer (MSP) */
  boss_stk_t *msp_base = _Boss_spy_msp_base();

  _boss_spy_msp_peak = msp_curr;
    
  for(; msp_base < msp_curr; msp_base++)
  {
    *msp_base = (boss_stk_t)0xEEEEEEEE;  // 스택 [E] empty
  }
}


/*===========================================================================
    _   B O S S _ S P Y _ M S P _ C H E C K
---------------------------------------------------------------------------*/
void _Boss_spy_msp_check(void)
{  
  /* [ ARM Cortex-Mx MSP (Main Stack Pointer) ] */
  while( (_boss_spy_msp_peak[-1] != (boss_stk_t)0xEEEEEEEE) 
      || (_boss_spy_msp_peak[-2] != (boss_stk_t)0xEEEEEEEE)
      || (_boss_spy_msp_peak[-3] != (boss_stk_t)0xEEEEEEEE)
      || (_boss_spy_msp_peak[-4] != (boss_stk_t)0xEEEEEEEE)
    #if 1
      || (_boss_spy_msp_peak[-5] != (boss_stk_t)0xEEEEEEEE)
      || (_boss_spy_msp_peak[-6] != (boss_stk_t)0xEEEEEEEE)
      || (_boss_spy_msp_peak[-7] != (boss_stk_t)0xEEEEEEEE)
      || (_boss_spy_msp_peak[-8] != (boss_stk_t)0xEEEEEEEE)
    #endif
      )
  {
    _boss_spy_msp_peak--;
    if(_boss_spy_msp_peak <= _Boss_spy_msp_base())
    {
      BOSS_ASSERT(_BOSS_FALSE);             // MSP Stack overflow
      for(;;);
    }
  }
}


/*===========================================================================
    B O S S _ S P Y _ M S P _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_spy_msp_report(void)
{
  boss_u32_t msp_total;
  boss_u32_t msp_used;
  boss_u32_t percent;

  msp_total = (boss_u32_t)_Boss_spy_msp_limit() - (boss_u32_t)_Boss_spy_msp_base();
  msp_used  = (boss_u32_t)_Boss_spy_msp_limit() - (boss_u32_t)_boss_spy_msp_peak;
  percent   = (msp_used * 100) / msp_total;


  PRINTF("\n[ M S P ] %%(u/t) :  %2d%% (%3d/%3d)\n", percent, msp_used, msp_total);
}
#endif /* _BOSS_SPY_ */


#ifdef _BOSS_MEM_INFO_
/*===========================================================================
    B O S S _ M E M _ I N F O _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_mem_info_report(void)
{
  boss_uptr_t total;
  boss_uptr_t used;
  boss_uptr_t peak;

  _Boss_sched_lock();
  PRINTF("\n[Mmory]  Peak byte  Used byte  Total  Block  first\n");

  total = _Boss_mem_info_total();
  used  = _Boss_mem_info_used();
  peak  = _Boss_mem_info_peak();
  
  PRINTF("[Info]  %4d (%2d%%) %4d (%2d%%)  %4d    %2d    %4d\n\n", 
                      peak, (boss_uptr_t)((peak * 100) / total),
                      used, (boss_uptr_t)((used * 100) / total),
                      total,_Boss_mem_info_block(),
                      _Boss_mem_info_first_free() );
  _Boss_sched_free();
}
#endif


/*===========================================================================
    _ A S S E R T
---------------------------------------------------------------------------*/
void _assert(const char *file, unsigned int line)
{
  __disable_irq();
  PRINTF("\n ASSERT : %s %d", file, line);
  for(;;)
  {
  }
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                   [ Cortex-M3 Debug (printf) Viewer ]                       *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================
    F P U T C
---------------------------------------------------------------------------*/
#if defined ( __CC_ARM )  /*---------------RealView Compiler --------------*/
int fputc(int ch, FILE *f)
{
  if(ch == '\n') {
    ITM_SendChar('\r');
  }

  ITM_SendChar(ch);
  
  return (ch);
}
#endif




/*===========================================================================
    B O S S _ D E V I C E _ I N I T
---------------------------------------------------------------------------*/
void Boss_device_init(void)
{
  #ifdef _BOSS_SPY_
  _Boss_spy_msp_setup();
  #endif

  if (SysTick_Config(SystemCoreClock / (1000 / _BOSS_TICK_MS_))) { /* Setup SysTick Timer for 1 msec interrupts  */
    while (1);                                  /* Capture error */
  }
  
  NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* PendSV IRQ 우선순위 */
}



/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                       [ RT-BOSS Tick Timer (ISR) ]                          *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
/*===========================================================================
    S Y S   T I C K _   H A N D L E R                         [ SysTick ISR ]
---------------------------------------------------------------------------*/
void SysTick_Handler(void)    /* Boss Tick Timer */
{
  _BOSS_ISR_BEGIN();
  {
    #ifdef _BOSS_RR_QUANTUM_MS
    _Boss_sched_rr_quantum_tick(_BOSS_TICK_MS_);
    #endif

    #ifdef _BOSS_SPY_
    _Boss_spy_elapse_tick(_BOSS_TICK_MS_);
    #endif
    
    _Boss_timer_tick(_BOSS_TICK_MS_);
  }
  _BOSS_ISR_FINIS();
}

