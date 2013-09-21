/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Kernel)                               *
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
/*static*/ boss_tcb_t *_current_tcb     = _BOSS_NULL;
/*static*/ boss_tcb_t *_sched_tcb_list  = _BOSS_NULL;

/*static*/ boss_reg_t _sched_locking    = 0;


#ifdef _BOSS_TCB_EXTEND_
/*static*/ boss_tcb_t   *_ex_task_list  = _BOSS_NULL;
/*static*/ unsigned int _ex_task_count  = 0;
#endif

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_start_schedule(void);
void _Boss_context_switch(void);
boss_stk_t *_Boss_stk_init( int (*task)(void *p_arg), void *p_arg,
                                boss_stk_t *sp_base,  boss_uptr_t stk_bytes);

/*===========================================================================
    B O S S _ S E L F
---------------------------------------------------------------------------*/
boss_tcb_t *Boss_self(void)
{
  return _current_tcb;
}


/*===========================================================================
    _   B O S S _ T C B _ I N I T
---------------------------------------------------------------------------*/
static void _Boss_tcb_init( boss_tcb_t *p_tcb, boss_prio_t prio,
                            int (*task)(void *p_arg), void *p_arg, 
                            boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                            const char *name )
{
  p_tcb->run_next = _BOSS_NULL;
  p_tcb->prio   = prio;
  
  p_tcb->sigs   = 0;
  p_tcb->wait   = 0;
  
  p_tcb->sp     = _Boss_stk_init(task, p_arg, sp_base, stk_bytes);  
    
  #ifdef _BOSS_TCB_NAME_SIZE
  {
    int  i;
    for(i = 0; i < _BOSS_TCB_NAME_SIZE; i++) {
      if(name != _BOSS_NULL) {
        p_tcb->name[i] = name[i];
      } else {
        p_tcb->name[i] = 0;
      }
    }
    p_tcb->name[_BOSS_TCB_NAME_SIZE - 1] = 0;
  }
  #endif

  #ifdef _BOSS_SPY_
  _Boss_spy_set(p_tcb, sp_base, stk_bytes);
  #endif

  #ifdef _BOSS_TCB_EXTEND_
  BOSS_IRQ_DISABLE();  
  _ex_task_count++;
  
  p_tcb->ex_task_link = _ex_task_list;
  _ex_task_list = p_tcb;
  BOSS_IRQ_RESTORE();
  #endif
}


/*===========================================================================
    B O S S _ I N I T
---------------------------------------------------------------------------*/
void Boss_init(int (*idle_task)(void *), boss_tcb_t *idle_tcb,
                                  boss_stk_t *sp_base, boss_uptr_t stk_bytes)
{
  BOSS_ASSERT( (_sched_tcb_list == _BOSS_NULL) && (_sched_locking == 0) );
  
  BOSS_IRQ_DISABLE();
  _sched_locking  = 1;              /* 스케줄링 금지 */

  _Boss_tcb_init(idle_tcb, PRIO_BOSS_IDLE, idle_task, _BOSS_NULL,
                                                sp_base, stk_bytes, "Idle");  
  _sched_tcb_list = idle_tcb;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ S T A R T
---------------------------------------------------------------------------*/
void Boss_start(void)
{
  BOSS_ASSERT( (_sched_locking == 1) && (_current_tcb == _BOSS_NULL) );
  
  BOSS_IRQ_DISABLE();
  _current_tcb = _sched_tcb_list;               /* Best TCB       */
  _sched_locking = 0;                           /* 스케줄링 허용  */  
  BOSS_IRQ_RESTORE();
  
  _Boss_start_schedule();                       /* 스케줄러 시작  */  
}


/*===========================================================================
    _ B O S S _ S W I T C H _ C U R R E N T _ T C B
---------------------------------------------------------------------------*/
boss_stk_t *_Boss_switch_current_tcb(boss_stk_t *cur_task_sp)
{
  BOSS_ASSERT(_sched_locking == 0);
  
  _current_tcb->sp = cur_task_sp;       /* 실행중인 Task SP */

  #ifdef _BOSS_SPY_
  _Boss_spy_context(_current_tcb, _sched_tcb_list);
  #endif
  
  BOSS_IRQ_DISABLE();
  _current_tcb = _sched_tcb_list;       /* Current TCB 변경 */
  BOSS_IRQ_RESTORE();
  
  cur_task_sp = _current_tcb->sp;       /* 실행할 Task SP   */
  
  return cur_task_sp;
}


/*===========================================================================
    B O S S _ S C H E D U L E
---------------------------------------------------------------------------*/
static void _Boss_schedule(void)
{
  if( (_sched_locking == 0) && (_current_tcb != _sched_tcb_list) )
  {
    _Boss_context_switch();   /* 문맥 전환 */
  }
}


/*===========================================================================
    _   B O S S _ S C H E D _ L I S T _ I N S E R T
---------------------------------------------------------------------------*/
static void _Boss_sched_list_insert(boss_tcb_t *p_tcb)
{
  //BOSS_ASSERT(_BOSS_IRQ_() != 0);
  
  if(p_tcb->run_next == _BOSS_NULL)
  {
    if(p_tcb->prio < _sched_tcb_list->prio)
    {
      p_tcb->run_next = _sched_tcb_list;
      _sched_tcb_list = p_tcb;
    }
    else
    {
      boss_tcb_t *p_prev = _sched_tcb_list;

      while(p_prev->run_next->prio <= p_tcb->prio)
      {
        p_prev = p_prev->run_next;
      }

      p_tcb->run_next   = p_prev->run_next;
      p_prev->run_next  = p_tcb;
    }

    #ifdef _BOSS_RR_QUANTUM_MS
    p_tcb->quantum_ms = _BOSS_RR_QUANTUM_MS;
    #endif
  }
}


/*===========================================================================
    _   B O S S _ S C H E D _ L I S T _ R E M O V E
---------------------------------------------------------------------------*/
static void _Boss_sched_list_remove(boss_tcb_t *p_tcb)
{
  //BOSS_ASSERT(_BOSS_IRQ_() != 0);
  
  if(p_tcb->run_next != _BOSS_NULL)
  {
    if(_sched_tcb_list == p_tcb)
    {
      _sched_tcb_list = p_tcb->run_next;
    }
    else
    {
      boss_tcb_t *p_prev = _sched_tcb_list;
      
      while(p_prev->run_next != p_tcb)
      {
        p_prev = p_prev->run_next;
      }
      
      p_prev->run_next = p_tcb->run_next;
    }
    
    p_tcb->run_next = _BOSS_NULL;         /* 스케줄러 리스트에서 제거됨 */
  }
}


#ifdef _BOSS_RR_QUANTUM_MS
/*===========================================================================
    _   B O S S _ S C H E D _ R R _ Q U A N T U M _ T I C K
---------------------------------------------------------------------------*/
void _Boss_sched_rr_quantum_tick(boss_tmr_ms_t tick_ms)
{
  boss_tcb_t  *cur_tcb = Boss_self();
  
  if( cur_tcb->run_next && (cur_tcb->prio == cur_tcb->run_next->prio) )
  {
    if(cur_tcb->quantum_ms > tick_ms)
    {
      cur_tcb->quantum_ms = cur_tcb->quantum_ms - tick_ms;
    }
    else
    {
      cur_tcb->quantum_ms = 0;
      BOSS_IRQ_DISABLE();
      _Boss_sched_list_remove(cur_tcb);
      _Boss_sched_list_insert(cur_tcb);
      BOSS_IRQ_RESTORE();
    }
  }
  else
  {
    cur_tcb->quantum_ms = _BOSS_RR_QUANTUM_MS;
  }
}
#endif /* _BOSS_RR_QUANTUM_MS */


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                               [ Signal ]                                    *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================
    _   B O S S _ W A I T _ S I G _ F O R E V E R
---------------------------------------------------------------------------*/
void _Boss_wait_sig_forever(boss_sigs_t wait_sigs)
{
  boss_tcb_t  *cur_tcb;

  BOSS_ASSERT(_BOSS_IRQ_() == 0);
  BOSS_ASSERT(_BOSS_ISR_() == 0);
  BOSS_ASSERT(Boss_sched_locking() == 0);
  BOSS_ASSERT(wait_sigs != 0);
  
  cur_tcb = Boss_self();
  cur_tcb->wait = wait_sigs;

  BOSS_IRQ_DISABLE();
  if( (cur_tcb->sigs & wait_sigs) == 0 ) {    /* 실행할 시그널이 없을면 */
      _Boss_sched_list_remove(cur_tcb);       /* 스케줄러 리스트에서 제거 */
  }
  BOSS_IRQ_RESTORE();

  _Boss_schedule();       /* 문맥전환 실행  */
}


/* Timeout Timer */
typedef struct {
  boss_tmr_t  tmr;
  boss_tcb_t  *p_tcb;
} _timeout_tmr_t;

/*===========================================================================
    _ T I M E O U T _ C A L L B A C K
---------------------------------------------------------------------------*/
static void _timeout_callback(boss_tmr_t *p_tmr)
{
  _timeout_tmr_t *p_timeout = (_timeout_tmr_t *)p_tmr;

  Boss_sig_send(p_timeout->p_tcb, SIG_BOSS_TIMEOUT);
}


/*===========================================================================
    _   B O S S _ W A I T _ S I G _ T I M E O U T
---------------------------------------------------------------------------*/
boss_tmr_ms_t _Boss_wait_sig_timeout(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout)
{
  _timeout_tmr_t  timeout_tmr;
  
  Boss_sig_clear(Boss_self(), SIG_BOSS_TIMEOUT);
  
  timeout_tmr.tmr.prev  = _BOSS_NULL;
  timeout_tmr.p_tcb     = Boss_self();
  
  Boss_tmr_start((boss_tmr_t *)&timeout_tmr, timeout, _timeout_callback);

  _Boss_wait_sig_forever(wait_sigs | SIG_BOSS_TIMEOUT);
  
  Boss_tmr_stop((boss_tmr_t *)&timeout_tmr);

  return timeout_tmr.tmr.tmr_ms;            /* 남은 시간 반환 */
}


/*===========================================================================
    B O S S _ S L E E P
---------------------------------------------------------------------------*/
void Boss_sleep(boss_tmr_ms_t wait_ms)
{
  (void)_Boss_wait_sig_timeout( (boss_sigs_t)0, wait_ms);
}


/*===========================================================================
    B O S S _ S I G _ W A I T
---------------------------------------------------------------------------*/
boss_sigs_t Boss_sig_wait(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout)
{
  if(WAIT_FOREVER == timeout) {
      _Boss_wait_sig_forever(wait_sigs);
  } else {
      (void)_Boss_wait_sig_timeout(wait_sigs, timeout);
  }

  return Boss_sig_receive(wait_sigs);
}


/*===========================================================================
    B O S S _ S I G _ R E C E I V E
---------------------------------------------------------------------------*/
boss_sigs_t Boss_sig_receive(boss_sigs_t wait_sigs)
{
  boss_sigs_t recv_sigs;
  boss_tcb_t  *cur_tcb = Boss_self();
  
  BOSS_IRQ_DISABLE();
  recv_sigs     = cur_tcb->sigs & wait_sigs;
  cur_tcb->sigs = cur_tcb->sigs & ~recv_sigs;   /* 수신한 시그널 클리어 */
  BOSS_IRQ_RESTORE();
  
  return recv_sigs;
}


/*===========================================================================
    _   B O S S _ S E T T I N G _ S I G N A L
---------------------------------------------------------------------------*/
void _Boss_setting_signal(boss_tcb_t *p_tcb, boss_sigs_t sigs)
{
  BOSS_IRQ_DISABLE();
  p_tcb->sigs = p_tcb->sigs | sigs;

  if( p_tcb->wait & sigs ) {
      _Boss_sched_list_insert(p_tcb);
  }
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ S I G _ S E N D
---------------------------------------------------------------------------*/
void Boss_sig_send(boss_tcb_t *p_tcb, boss_sigs_t sigs)
{
  _Boss_setting_signal(p_tcb, sigs);
  
  _Boss_schedule();       /* 문맥전환 실행  */
}


/*===========================================================================
    B O S S _ S I G _ C L E A R
---------------------------------------------------------------------------*/
void Boss_sig_clear(boss_tcb_t *p_tcb, boss_sigs_t sigs)
{
  BOSS_IRQ_DISABLE();
  p_tcb->sigs = p_tcb->sigs & ~sigs;
  BOSS_IRQ_RESTORE();
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                [ TASK ]                                     *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================
    B O S S _ T A S K _ C R E A T E
---------------------------------------------------------------------------*/
void Boss_task_create(  int (*task)(void *p_arg), void *p_arg, 
                        boss_tcb_t *p_tcb, boss_prio_t prio, 
                        boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                        const char *name )
{
  BOSS_ASSERT(_BOSS_ISR_() == 0);
  
  _Boss_tcb_init(p_tcb, prio, task, p_arg, sp_base, stk_bytes, name);
  BOSS_IRQ_DISABLE();
  _Boss_sched_list_insert(p_tcb);  
  BOSS_IRQ_RESTORE();
  
  _Boss_schedule();
}


/*===========================================================================
    B O S S _ T A S K _ P R I O R I T Y
---------------------------------------------------------------------------*/
void Boss_task_priority(boss_tcb_t *p_tcb, boss_prio_t new_prio)
{
  BOSS_ASSERT(new_prio < PRIO_BOSS_IDLE);
  
  BOSS_IRQ_DISABLE();
  p_tcb->prio = new_prio;
  
  if(p_tcb->run_next != _BOSS_NULL)     /* schedule list update */
  {
    _Boss_sched_list_remove(p_tcb);
    _Boss_sched_list_insert(p_tcb);
  }
  BOSS_IRQ_RESTORE();

  _Boss_schedule();
}


/*===========================================================================
    _   B O S S _ T A S K _ E X I T
---------------------------------------------------------------------------*/
void _Boss_task_exit(int exit_code)
{  
  boss_tcb_t  *cur_tcb;

  BOSS_ASSERT(exit_code == 0);
  BOSS_ASSERT(_BOSS_IRQ_() == 0);
  BOSS_ASSERT(_BOSS_ISR_() == 0);
  BOSS_ASSERT(Boss_sched_locking() == 0);
  
  cur_tcb = Boss_self();
  
  #ifdef _BOSS_TCB_EXTEND_
  _Boss_sched_lock();
  BOSS_IRQ_DISABLE();
  _ex_task_count--;
  BOSS_IRQ_RESTORE();
  
  if(_ex_task_list == cur_tcb) {
    _ex_task_list = cur_tcb->ex_task_link;
  } else {
    boss_tcb_t *p_prev = _ex_task_list;
    while(p_prev->ex_task_link != cur_tcb) {
      p_prev = p_prev->ex_task_link;
      BOSS_ASSERT(p_prev != _BOSS_NULL);
    }
    p_prev->ex_task_link = cur_tcb->ex_task_link;
  }
  cur_tcb->ex_task_link = _BOSS_NULL;
  _Boss_sched_free();
  #endif
  
  cur_tcb->wait   = 0;
  BOSS_IRQ_DISABLE();
  _Boss_sched_list_remove(cur_tcb);
  BOSS_IRQ_RESTORE();
  
  _Boss_schedule();
}


#ifdef _BOSS_TCB_EXTEND_
/*===========================================================================
    B O S S _ E X _ T A S K _ C O U N T
---------------------------------------------------------------------------*/
unsigned int Boss_ex_task_count(void)
{
  return _ex_task_count;
}


/*===========================================================================
    B O S S _ E X _ T A S K _ L I S T
---------------------------------------------------------------------------*/
boss_tcb_t *Boss_ex_task_list(unsigned int i)
{
  boss_tcb_t *p_tcb = _BOSS_NULL;
  
  _Boss_sched_lock();
  if(i < _ex_task_count)
  {
    p_tcb = _ex_task_list;
    while(i != 0) {
      p_tcb = p_tcb->ex_task_link;
      BOSS_ASSERT(p_tcb != _BOSS_NULL);
      i--;
    }
  }
  _Boss_sched_free();

  return p_tcb;
}
#endif /* _BOSS_TCB_EXTEND_ */

/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                          [ Schedule Lock ]                                  *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================
    _ B O S S _ S C H E D _ L O C K
---------------------------------------------------------------------------*/
void _Boss_sched_lock(void)
{
  BOSS_IRQ_DISABLE();
  _sched_locking++;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    _   B O S S _ S C H E D _ F R E E
---------------------------------------------------------------------------*/
void _Boss_sched_free(void)
{
  BOSS_ASSERT(_sched_locking > 0);

  BOSS_IRQ_DISABLE();
  _sched_locking--;
  BOSS_IRQ_RESTORE();

  _Boss_schedule();
}


/*===========================================================================
    B O S S _ S C H E D _ L O C K I N G
---------------------------------------------------------------------------*/
boss_reg_t Boss_sched_locking(void)
{
  return _sched_locking;
}

