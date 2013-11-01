/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                                M  A  I  N                                   *
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
#ifdef _BOSS_TMR_H_
BOSS_TMR_ID_T   ex_tmr_id;
#endif

#ifdef _BOSS_FLAG_H_
BOSS_FLAG_ID_T  ex_flag_id;
#endif

#ifdef _BOSS_MSG_Q_H_
BOSS_MSG_Q_ID_T ex_msg_id;
#endif

#ifdef _BOSS_SEM_H_
BOSS_SEM_ID_T   ex_sem_id;
#endif


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_device_init(void);

/*===========================================================================
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{
  Boss_sleep(100);

  #ifdef _BOSS_SPY_
  Boss_spy_report();
  Boss_spy_restart();
  #endif
  
  #ifdef _BOSS_MEMORY_H_
  Boss_mfree( Boss_malloc(1));
  
  #ifdef _BOSS_MEM_INFO_
  Boss_mem_info_report();
  #endif
  #endif

  #ifdef _BOSS_TMR_H_
  ex_tmr_id = Boss_tmr_create();
  Boss_tmr_start(ex_tmr_id, 2000, _BOSS_NULL);
  Boss_tmr_stop(ex_tmr_id);
  Boss_tmr_del(ex_tmr_id);
  #endif

  #ifdef _BOSS_FLAG_H_
  ex_flag_id = Boss_flag_grp_create();
  Boss_flag_send(ex_flag_id, 0x0001);
  (void)Boss_flag_wait(ex_flag_id, 0x0003, _FLAG_OPT_OR, WAIT_FOREVER);
  Boss_flag_clear(ex_flag_id, 0x0001);
  Boss_flag_grp_del(ex_flag_id);
  #endif

  #ifdef _BOSS_MSG_Q_H_
  ex_msg_id = Boss_msg_q_create(5, MSG_Q_PRIORITY);
  Boss_msg_send(ex_msg_id, M_CMD_1, _BOSS_FALSE);
  Boss_msg_send_front(ex_msg_id, M_CMD_2, _BOSS_FALSE);
  {
    boss_msg_t msg;
    msg = Boss_msg_wait(ex_msg_id, WAIT_FOREVER);
    PRINTF("%d : %d\n", msg.m_cmd, msg.param);
  }
  #endif
  
  #ifdef _BOSS_SEM_H_
  ex_sem_id = Boss_sem_create(1);
  if( _BOSS_SUCCESS == Boss_sem_obtain(ex_sem_id, WAIT_FOREVER) )
  {
    Boss_sem_release(ex_sem_id);
  }
  #endif

  for(;;)
  {
    Boss_sleep(500);  /* 500ms */
  }
    
  return 0;       // 테스크 종료
}



/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                         RT-BOSS ( IDLE TASK )                               *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
boss_stk_t idle_stack[ 160 / sizeof(boss_stk_t)]; 

int idle_task(void *p_arg)
{
  for(;;)
  {
  }
}


/*===========================================================================
    M A I N
---------------------------------------------------------------------------*/
int main(void)
{
  (void)Boss_init(idle_task, _BOSS_NULL, idle_stack, sizeof(idle_stack));
  
  (void)Boss_task_create( aa_task,              /* Task Entry Point       */
                          _BOSS_NULL,           /* Task Argument          */
                          aa_stk,               /* 스택 포인터(base)      */
                          sizeof(aa_stk),       /* 스택 크기(Bytes)       */
                          PRIO_1,               /* 우선순위               */
                          "AA"                  /* 테스크 이름            */
                        );
  
  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


#ifdef _BOSS_SPY_
/*===========================================================================
    B O S S _ S P Y _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_spy_report(void)
{
  boss_u32_t total_us = 0;
  
  boss_u32_t cpu_pct_sum = 0;
  boss_u32_t context_sum = 0;
  boss_tcb_t *p_tcb;

  _Boss_sched_lock();
  PRINTF("\n==================================================\n");
  PRINTF(  "PRI  NAME  STACK %%(u/t)   C P U   Status  Context\n");
  PRINTF(  "--------------------------------------------------\n");
  
  total_us = Boss_spy_elapse_prev_us();

  p_tcb = Boss_ex_task_list(0);
  while( p_tcb != _BOSS_NULL )
  {
    PRINTF("%3d %5s", p_tcb->prio, p_tcb->name);

    { /* [ Stack ] */
      boss_uptr_t stk_total;
      boss_uptr_t stk_used;
      boss_reg_t  stk_pct;      /* percent xx % */
      
      stk_total = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_base;
      stk_used  = (boss_uptr_t)p_tcb->ex.sp_limit - (boss_uptr_t)p_tcb->ex.sp_peak;
      stk_pct = (boss_reg_t)(((boss_u32_t)stk_used * 100) / (boss_u32_t)stk_total);

      stk_total = stk_total;
      stk_used = stk_used;
      stk_pct = stk_pct;
      
      PRINTF("  %2d%%(%3d/%3d)", stk_pct, stk_used, stk_total);
    }

    { /* [ C P U ] */
      boss_u32_t cpu_pct = 0;     /* percent XX.xxx % */

      cpu_pct = (boss_u32_t)(((boss_u64_t)(p_tcb->ex.run_time) * (boss_u64_t)100000)
                                                    / (boss_u64_t)total_us);
      
      PRINTF("  %2d.%03d%%", (int)(cpu_pct/1000), (int)(cpu_pct%1000));
      cpu_pct_sum = cpu_pct_sum  + cpu_pct;
    }

    { /* [ Task Priority / Status ]*/
      int status = Boss_spy_task_status(p_tcb);
      PRINTF("   %s", (status == 2) ? "RUN "
                    : (status == 1) ? "Pend"
                    :                 "Wait" );
    }

    PRINTF("  %7d\n", p_tcb->ex.context);
    context_sum = context_sum + p_tcb->ex.context;

    p_tcb = p_tcb->ex_task_link;    // Next Task link
  }


  cpu_pct_sum = cpu_pct_sum;
  context_sum = context_sum;

  PRINTF("\n---[TOTAL]-------------  %2d.%03d%%  ------ %7d\n\n",
          (int)(cpu_pct_sum/1000), (int)(cpu_pct_sum%1000), context_sum);
  
  _Boss_sched_free();

  total_us = total_us;

  PRINTF("   total_us = %d\n", total_us);
  PRINTF("   SysTick->LOAD = %d\n", SysTick->LOAD);

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
void _Boss_tick(boss_tmr_ms_t tick_ms);
void _Boss_timer_tick(boss_tmr_ms_t tick_ms);

/*===========================================================================
    S Y S   T I C K _   H A N D L E R                         [ SysTick ISR ]
---------------------------------------------------------------------------*/
void SysTick_Handler(void)    /* Boss Tick Timer */
{
  _BOSS_ISR_BEGIN();
  {
    _Boss_tick(_BOSS_TICK_MS_);

    #ifdef _BOSS_TMR_H_
    _Boss_timer_tick(_BOSS_TICK_MS_);
    #endif
  }
  _BOSS_ISR_FINIS();
}

