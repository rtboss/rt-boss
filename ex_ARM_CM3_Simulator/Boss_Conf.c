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

boss_u32_t  _spy_elapse_us  = 0;              /* 경과 시간 (us)       */


struct {                /* [ ARM Cortex-Mx MSP (Main Stack Pointer) ] */
  boss_stk_t    *sp_base;
  boss_stk_t    *sp_peak;
  boss_stk_t    *sp_limit;
} _spy_msp;


/*===========================================================================
    _   B O S S _ S P Y _ E L A P S E _ U S
---------------------------------------------------------------------------*/
boss_u32_t _Boss_spy_elapse_us(void)
{
  boss_u32_t us;
  boss_u32_t reload = SysTick->LOAD;
  boss_u32_t value  = reload - SysTick->VAL;   /* count-down value */
  
                                /* SysTick->VAL => micro second */
  us = (value * ((boss_u32_t)_BOSS_TICK_MS_ * (boss_u32_t)1000)) / (reload + 1);
  us = _spy_elapse_us + us;
  
  return us;
}


/*===========================================================================
    _   B O S S _ S P Y _ T I C K
---------------------------------------------------------------------------*/
void _Boss_spy_tick(void)
{
  _spy_elapse_us += (boss_u32_t)_BOSS_TICK_MS_ * (boss_u32_t)1000;  /* tick_ms -> us */
  
  if(_spy_elapse_us > 4200000000u)  /* 70분 = 4,200,000,000 us */
  {
    Boss_spy_restart();             /* 70분 경과시 */
  }
}


/*===========================================================================
    _   B O S S _ S P Y _ C O N T E X T
---------------------------------------------------------------------------*/
void _Boss_spy_context(boss_tcb_t *curr_tcb, boss_tcb_t *best_tcb)
{
  { /* [ Stack ] */
    BOSS_ASSERT(curr_tcb->ex.sp_base[0] == (boss_stk_t)0xEEEEEEEE);  // Stack invasion
    while( (curr_tcb->ex.sp_peak[-1] != (boss_stk_t)0xEEEEEEEE) 
        || (curr_tcb->ex.sp_peak[-2] != (boss_stk_t)0xEEEEEEEE)
        || (curr_tcb->ex.sp_peak[-3] != (boss_stk_t)0xEEEEEEEE)
        || (curr_tcb->ex.sp_peak[-4] != (boss_stk_t)0xEEEEEEEE) )
    {
      curr_tcb->ex.sp_peak--;
      BOSS_ASSERT(curr_tcb->ex.sp_peak > curr_tcb->ex.sp_base);   // Stack overflow
    }
  }

  { /* [ C P U ] */
    boss_u32_t now_us = _Boss_spy_elapse_us();

    if( now_us < curr_tcb->ex.cpu_ent_us ) {       /* Tick Timer Pend */
      now_us = now_us + ((boss_u32_t)_BOSS_TICK_MS_ * (boss_u32_t)1000);
      BOSS_ASSERT(now_us >= curr_tcb->ex.cpu_ent_us);
    }
    
    curr_tcb->ex.cpu_sum_us += now_us - curr_tcb->ex.cpu_ent_us;
    best_tcb->ex.cpu_ent_us = now_us;
  }
  
  /* [ Context Switch Number ] */
  best_tcb->ex.context++;
  
  
  /* [ ARM Cortex-Mx MSP (Main Stack Pointer) ] */
  BOSS_ASSERT(_spy_msp.sp_base[0] == (boss_stk_t)0xEEEEEEEE);   // Stack invasion
  while( (_spy_msp.sp_peak[-1] != (boss_stk_t)0xEEEEEEEE) 
      || (_spy_msp.sp_peak[-2] != (boss_stk_t)0xEEEEEEEE)
      || (_spy_msp.sp_peak[-3] != (boss_stk_t)0xEEEEEEEE)
      || (_spy_msp.sp_peak[-4] != (boss_stk_t)0xEEEEEEEE) )
  {
    _spy_msp.sp_peak--;
    BOSS_ASSERT(_spy_msp.sp_peak > _spy_msp.sp_base);  // MSP Stack overflow
  }
}


/*===========================================================================
    _   B O S S _ S P Y _ S E T U P
---------------------------------------------------------------------------*/
void _Boss_spy_setup(boss_tcb_t *p_tcb, boss_stk_t *sp_base, boss_uptr_t bytes)
{
  { /* [ Stack ] */
    boss_uptr_t size  = bytes / sizeof(boss_stk_t);
    
    p_tcb->ex.sp_base  = &sp_base[0];
    p_tcb->ex.sp_peak  = &sp_base[size-1];
    p_tcb->ex.sp_limit = &sp_base[size];
  }

  /* [ C P U ] */
  p_tcb->ex.cpu_ent_us = 0;
  p_tcb->ex.cpu_sum_us = 0;

  /* [ Context Switch Number ] */
  p_tcb->ex.context = 0;
}


/*===========================================================================
    B O S S _ S P Y _ R E S T A R T
---------------------------------------------------------------------------*/
void Boss_spy_restart(void)
{
  boss_tcb_t *p_tcb;

  _Boss_sched_lock();

  BOSS_IRQ_DISABLE();
  _spy_elapse_us  = 0;
  BOSS_IRQ_RESTORE();

  p_tcb = Boss_ex_task_list(0);
  while(p_tcb != _BOSS_NULL)
  {
    p_tcb->ex.cpu_ent_us  = 0;
    p_tcb->ex.cpu_sum_us  = 0;
    p_tcb->ex.context     = 0;
    p_tcb = p_tcb->ex_task_link;
  }
  _Boss_sched_free();
}


/*===========================================================================
    B O S S _ S P Y _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_spy_report(void)
{
  boss_tcb_t *curr_tcb;
  boss_u32_t total_us;
  
  boss_u32_t cpu_pct_sum = 0;
  boss_u32_t context_sum = 0;
  boss_tcb_t *p_tcb = _BOSS_NULL;
  
  _Boss_sched_lock();
  PRINTF("\n[TASK]\t  STACK %%(u/t)\t  C P U    Context\n");
  PRINTF("------------------------------------------\n");
  
  BOSS_IRQ_DISABLE();
  total_us = _Boss_spy_elapse_us();
  _spy_elapse_us = 0;
  
  curr_tcb = Boss_self();
  curr_tcb->ex.cpu_sum_us += total_us - curr_tcb->ex.cpu_ent_us;
  curr_tcb->ex.cpu_ent_us = 0;
  BOSS_IRQ_RESTORE();
  
  p_tcb = Boss_ex_task_list(0);
  while( p_tcb != _BOSS_NULL )
  {
    PRINTF(" %s", p_tcb->name);

    { /* [ Stack ] */
      boss_uptr_t stk_total;
      boss_uptr_t stk_used;
      boss_reg_t  stk_pct;
      
      stk_total = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_base;
      stk_used  = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_peak;
      stk_pct = (boss_reg_t)(((boss_u32_t)stk_used * 100) / (boss_u32_t)stk_total);
      
      PRINTF("\t  %2d%%(%3d/%3d)", stk_pct, stk_used, stk_total);
    }

    { /* [ C P U ] */
      boss_u32_t cpu_pct = 0;     /* percent XX.xxx % */
      
      if(p_tcb->ex.cpu_sum_us != 0)
      {
        cpu_pct = (boss_u32_t)(((boss_u64_t)(p_tcb->ex.cpu_sum_us) * (boss_u64_t)100000)
                                                    / (boss_u64_t)total_us);
        p_tcb->ex.cpu_sum_us = 0;
      }
      
      PRINTF("\t %2d.%03d%%", (int)(cpu_pct/1000), (int)(cpu_pct%1000));
      cpu_pct_sum = cpu_pct_sum  + cpu_pct;
    }
    
    PRINTF("   %7d\n", p_tcb->ex.context);
    context_sum = context_sum + p_tcb->ex.context;
    p_tcb->ex.context = 0;
    
    p_tcb = p_tcb->ex_task_link;    // Next Task link
  }

  PRINTF("[TOTAL] :\t\t %2d.%03d%%   %7d\n\n",
          (int)(cpu_pct_sum/1000), (int)(cpu_pct_sum%1000), context_sum);
  _Boss_sched_free();
  
  { /* [ ARM Cortex-Mx MSP (Main Stack Pointer) ] */
    boss_uptr_t msp_total;
    boss_uptr_t msp_used;
    boss_reg_t  msp_pct;
    
    msp_total = (boss_uptr_t)_spy_msp.sp_limit - (boss_uptr_t)_spy_msp.sp_base;
    msp_used  = (boss_uptr_t)_spy_msp.sp_limit - (boss_uptr_t)_spy_msp.sp_peak; 
    msp_pct = (boss_reg_t)(((boss_u32_t)msp_used * 100) / (boss_u32_t)msp_total);
    
    PRINTF("[ M S P ] %%(u/t) :  %2d%% (%3d/%3d)\n", msp_pct, msp_used, msp_total);
  }
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
  boss_stk_t *msp = (boss_stk_t *)__get_MSP(); /* Get Current Main Stack Pointer (MSP) */
  
  _spy_msp.sp_base  = _Boss_spy_msp_base();
  _spy_msp.sp_limit = _Boss_spy_msp_limit();
  _spy_msp.sp_peak  = (boss_stk_t *)msp;
  
  msp = msp - 1;    /* FD(Full Descending) Stack */
  
  for(; _spy_msp.sp_base <= msp; --msp)
  {
    *msp = (boss_stk_t)0xEEEEEEEE;  // 스택 [E] empty
  }
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

  if (SysTick_Config(SystemCoreClock / 1000)) { /* Setup SysTick Timer for 1 msec interrupts  */
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
    S Y S   T I C K _   H A N D L E R                 [Cortex-M3 SysTick ISR]
---------------------------------------------------------------------------*/
void SysTick_Handler(void)    /* Boss Tick Timer */
{
  _BOSS_ISR_BEGIN();
  {
    _Boss_timer_tick(_BOSS_TICK_MS_);
    
    #ifdef _BOSS_SPY_
    _Boss_spy_tick();
    #endif
  }
  _BOSS_ISR_FINIS();
}

