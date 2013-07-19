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
#define _BOSS_SPY_TCB_MAX       10            /* SPY TCB Table Size   */

boss_u32_t  _spy_elapse_us  = 0;              /* 경과 시간 (us)       */
boss_tcb_t  *_spy_tcb_tbl[_BOSS_SPY_TCB_MAX]; /* SPY TCB list         */


/*===========================================================================
    _   B O S S _ S P Y _ E L A P S E _ U S
---------------------------------------------------------------------------*/
boss_u32_t _Boss_spy_elapse_us(void)
{
  boss_u32_t us;
  boss_u32_t reload = OCR1A;
  boss_u32_t value  = TCNT1;     /* count value */
  
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
}


/*===========================================================================
    _   B O S S _ S P Y _ S E T U P
---------------------------------------------------------------------------*/
void _Boss_spy_setup(boss_tcb_t *p_tcb, boss_stk_t *sp_base, boss_uptr_t bytes)
{
  { /* SPY TCB 등록 */
    boss_reg_t idx;
  
    for(idx = 0; idx < _BOSS_SPY_TCB_MAX; idx++)
    {
      if(_spy_tcb_tbl[idx] == _BOSS_NULL) {
        _spy_tcb_tbl[idx] = p_tcb;
        break;
      }
    }
  
    BOSS_ASSERT(idx < _BOSS_SPY_TCB_MAX);
  }

  { /* [ Stack ] */
    boss_uptr_t size  = bytes / sizeof(boss_stk_t);
    boss_uptr_t i;
    
    for(i = 0; i < size; i++) {
      sp_base[i] = (boss_stk_t)0xEEEEEEEE;  // 스택 [E] empty
    }
    
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
  boss_reg_t idx;
  
  BOSS_IRQ_DISABLE();
  _spy_elapse_us  = 0;
  
  for(idx = 0; idx < _BOSS_SPY_TCB_MAX; idx++)
  {
    if(_spy_tcb_tbl[idx] != _BOSS_NULL)
    {
      _spy_tcb_tbl[idx]->ex.cpu_ent_us = 0;
      _spy_tcb_tbl[idx]->ex.cpu_sum_us = 0;
      _spy_tcb_tbl[idx]->ex.context    = 0;
    }
  }
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ S P Y _ R E P O R T
---------------------------------------------------------------------------*/
void Boss_spy_report(void)
{
  boss_tcb_t *curr_tcb;
  boss_u32_t total_us;
  boss_reg_t idx;
  
  boss_u32_t cpu_pct_sum = 0;
  boss_u32_t context_sum = 0;
  
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
  
  for(idx = 0; idx < _BOSS_SPY_TCB_MAX; idx++)
  {
    if(_spy_tcb_tbl[idx] != _BOSS_NULL)
    {
      boss_tcb_t *p_tcb = _spy_tcb_tbl[idx];

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
    }
  }

  PRINTF("[TOTAL] :\t\t %2d.%03d%%   %7d\n\n",
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
/*===========================================================================
   Timer1 Compare Match A                              [ATmega128 Timer1 ISR]
---------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect)
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

