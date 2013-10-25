/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                 RT-BOSS (Config File)         [ ATmega128 ]                 *
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
  boss_u32_t reload = OCR1A;
  boss_u32_t value  = TCNT1;     /* count value */
  
                                /* value => micro second */
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

  PRINTF("\n---[TOTAL]-------------  %2d.%03d%%  ------ %7d\n\n",
          (int)(cpu_pct_sum/1000), (int)(cpu_pct_sum%1000), context_sum);
  
  _Boss_sched_free();
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
  cli();
  PRINTF("\n ASSERT :");
  printf_P(file);
  PRINTF(" %d", line);
  for(;;)
  {
  }
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                       [ ATmega128 UART0 (printf) ]                          *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
static int uart0_putc(char ch, FILE *f);
static FILE uart_stdout = FDEV_SETUP_STREAM(uart0_putc, NULL, _FDEV_SETUP_WRITE);

/*===========================================================================
    U A R T 0 _ I N I T
---------------------------------------------------------------------------*/
void uart0_init(void)
{
  /* Set baud rate (16Mhz 115200bps) */
  UBRR0H = 0;
  UBRR0L = 16;
  UCSR0A |= (1 << U2X0);      /* Disable Double USART Speed */

  /* Enable receiver and transmitter  */
  UCSR0B = (1<<RXEN0) | (1<<TXEN0);
 
  /* Set frame format: 8data, 1stop bit */
  UCSR0C = (3 << UCSZ00);
}


/*===========================================================================
    U A R T 0 _ P U T C
---------------------------------------------------------------------------*/
static int uart0_putc(char ch, FILE *f)
{
  if(ch == '\n') {
    uart0_putc('\r', f);
  }
  
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = ch;
  
  return (ch);
}



/*===========================================================================
    B O S S _ D E V I C E _ I N I T
---------------------------------------------------------------------------*/
void Boss_device_init(void)
{
  uart0_init();
  stdout = &uart_stdout;    /* uart printf stdout init            */
  
  /* [RT-BOSS Tick Timer Init] Timer1 Compare Match A (16Mhz 10ms)*/
  TCCR1A = 0x00;            /* WGM1[3:0] = 4 (CTC)                */
  TCCR1B = 0x0A;            /* prescaler = 8                      */
  TCCR1C = 0x00;

  OCR1A  = 20000 - 1;       /* (8 * (1 + 19999) ) / 16MHZ = 10ms  */

  TCNT1  = 0;               /* Clear counter1                     */ 
  TIMSK = 1 << OCIE1A;      /* Enable OC1A interrupt              */
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
   Timer1 Compare Match A                              [ATmega128 Timer1 ISR]
---------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect)
{
  _BOSS_ISR_BEGIN();
  {
    _Boss_tick(_BOSS_TICK_MS_);
    
    _Boss_timer_tick(_BOSS_TICK_MS_);
  }
  _BOSS_ISR_FINIS();
}

