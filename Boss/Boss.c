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
boss_stk_t *_Boss_stk_init( int (*task_entry)(void *p_arg), void *p_arg,
                                boss_stk_t *sp_base,  boss_uptr_t stk_bytes);

#ifdef _BOSS_SPY_
void _Boss_spy_elapse_tick(boss_u32_t tick_ms);
#endif

/*===========================================================================
    B O S S _ S E L F
---------------------------------------------------------------------------*/
BOSS_TID_T Boss_self(void)
{
  return _current_tcb;
}


/*===========================================================================
    _   B O S S _ T C B _ I N I T
---------------------------------------------------------------------------*/
static boss_tcb_t *_Boss_tcb_init(int (*task_entry)(void *p_arg), void *p_arg,
                                  boss_stk_t *p_stack, boss_uptr_t stk_bytes,
                                            boss_prio_t prio, const char *name)
{
  /* TCB Align Sizeof */
  #define TCB_SIZE      ( (sizeof(boss_tcb_t) + (sizeof(boss_align_t)-1)) \
                                  & ~((boss_uptr_t)(sizeof(boss_align_t)-1)) )
  
  
  boss_tcb_t *p_tcb   = (boss_tcb_t *)p_stack;
  boss_stk_t *sp_base = (boss_stk_t *)((boss_uptr_t)p_tcb + TCB_SIZE);
  
  stk_bytes = stk_bytes - TCB_SIZE;
  
  p_tcb->run_next   = _BOSS_NULL;
  p_tcb->prio       = prio;
  p_tcb->indicate   = BOSS_INDICATE_CLEAR;
  
  p_tcb->sp         = _Boss_stk_init(task_entry, p_arg, sp_base, stk_bytes);
  
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

  return p_tcb;
}


/*===========================================================================
    B O S S _ I N I T
---------------------------------------------------------------------------*/
BOSS_TID_T Boss_init(int (*idle_task_entry)(void *p_arg), void *p_arg,
                                    boss_stk_t *p_stack, boss_uptr_t stk_bytes)
{
  boss_tcb_t *p_idle_tcb;
  
  BOSS_ASSERT( (_sched_tcb_list == _BOSS_NULL) && (_sched_locking == 0) );
  
  BOSS_IRQ_DISABLE();
  _sched_locking  = 1;              /* 스케줄링 금지 */
  
  p_idle_tcb = _Boss_tcb_init(idle_task_entry, p_arg, p_stack, stk_bytes,
                                                        PRIO_BOSS_IDLE, "Idle");
  _sched_tcb_list = p_idle_tcb;     /* Idle Task TCB */
  BOSS_IRQ_RESTORE();
  
  return p_idle_tcb;  // Idle Task ID
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
void _Boss_schedule(void)
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
      boss_tcb_t *p_prev;
      
      BOSS_ASSERT(p_tcb->prio < PRIO_BOSS_IDLE);

      p_prev = _sched_tcb_list;

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


/*===========================================================================
    _   B O S S _ S C H E D _ R E A D Y
---------------------------------------------------------------------------*/
void _Boss_sched_ready(boss_tcb_t *p_tcb, boss_u08_t indicate)
{
  BOSS_IRQ_DISABLE();
  p_tcb->indicate = p_tcb->indicate | indicate;
  _Boss_sched_list_insert(p_tcb);
  BOSS_IRQ_RESTORE();
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                           [ Timeout Wait ]                                  *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
typedef struct _wait_timeout_struct {
  struct _wait_timeout_struct  *prev;
  struct _wait_timeout_struct  *next;

  boss_tmr_ms_t           timeout_ms;
  boss_tcb_t              *p_tcb;
} _wait_timeout_t;

_wait_timeout_t *_wait_timeout_list = _BOSS_NULL;

/*===========================================================================
    _   B O S S _ S C H E D _ W A I T
---------------------------------------------------------------------------*/
boss_tmr_ms_t _Boss_sched_wait(boss_tmr_ms_t timeout)
{  
  boss_tcb_t      *cur_tcb;
  
  BOSS_ASSERT(_BOSS_IRQ_() == 0);
  BOSS_ASSERT(_BOSS_ISR_() == 0);
  BOSS_ASSERT(Boss_sched_locking() == 0);
  
  cur_tcb = Boss_self();

  if( timeout == NO_WAIT )                          // timeout : 0x00000000
  {
    ;
  }
  else if( timeout == WAIT_FOREVER )                // timeout : 0xffffffff
  {
    BOSS_IRQ_DISABLE();
    if(cur_tcb->indicate == BOSS_INDICATE_CLEAR) {
      _Boss_sched_list_remove(cur_tcb);             /* 스케줄러 리스트에서 제거 */
    }
    BOSS_IRQ_RESTORE();
    
    _Boss_schedule();                               /* 문맥전환 실행 */
  }
  else                                              // timeout : 1 ~ 0xfffffffe
  {
    boss_reg_t        irq_storage;
    _wait_timeout_t   timeout_link;

    //timeout_link.prev = &timeout_link;
    //timeout_link.next = &timeout_link;
    timeout_link.p_tcb      = cur_tcb;
    timeout_link.timeout_ms = timeout;

    BOSS_IRQ_DISABLE_SR(irq_storage);
    if(cur_tcb->indicate == BOSS_INDICATE_CLEAR)
    {
      _Boss_sched_list_remove(cur_tcb);           /* 스케줄러 리스트에서 제거 */

      /* Timeout 등록 */
      if(_wait_timeout_list == _BOSS_NULL) {        // 첫번째 등록
          timeout_link.prev   = &timeout_link;
          timeout_link.next   = &timeout_link;
          _wait_timeout_list  = &timeout_link;
      } else {
          timeout_link.prev = _wait_timeout_list->prev;
          timeout_link.next = _wait_timeout_list; // _wait_timeout_list->prev->next 와 동일
          _wait_timeout_list->prev->next  = &timeout_link;
          _wait_timeout_list->prev        = &timeout_link;
      }
      BOSS_IRQ_RESTORE_SR(irq_storage);

      _Boss_schedule();                           /* 문맥전환 실행 */

      BOSS_IRQ_DISABLE_SR(irq_storage);
      /* Timeout 제거 */
      timeout_link.prev->next = timeout_link.next;
      timeout_link.next->prev = timeout_link.prev;

      if(_wait_timeout_list == &timeout_link)
      {
        if(timeout_link.next == &timeout_link) {  // 마지막 제거
          BOSS_ASSERT(timeout_link.prev == &timeout_link);
          _wait_timeout_list = _BOSS_NULL;
        } else {
          _wait_timeout_list = _wait_timeout_list->next;
        }
      }
    }
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    timeout = timeout_link.timeout_ms;            /* 남은 시간 반환 */
  }
  
  return timeout;
}


/*===========================================================================
    B O S S _ S L E E P
---------------------------------------------------------------------------*/
void Boss_sleep(boss_tmr_ms_t timeout)
{  
  BOSS_ASSERT(timeout != WAIT_FOREVER);           // timeout : 0xffffffff

  if(timeout != NO_WAIT)                          // timeout : 0x00000000 아닐때
  {
    Boss_self()->indicate = BOSS_INDICATE_CLEAR;

    (void)_Boss_sched_wait(timeout);              // timeout : 1 ~ 0xfffffffe
  }
}


/*===========================================================================
    _   B O S S _ T I C K                                       [ Tick ISR ]
---------------------------------------------------------------------------*/
void _Boss_tick(boss_tmr_ms_t tick_ms)
{
  #ifdef _BOSS_SPY_
  _Boss_spy_elapse_tick(tick_ms);
  #endif

  #ifdef _BOSS_RR_QUANTUM_MS
  _Boss_sched_rr_quantum_tick(tick_ms);
  #endif
  
  
  if(_wait_timeout_list != _BOSS_NULL)
  {
    _wait_timeout_t *p_timeout = _wait_timeout_list;
    
    do {
        if(tick_ms < p_timeout->timeout_ms) {
            p_timeout->timeout_ms = p_timeout->timeout_ms - tick_ms;
        } else {
            p_timeout->timeout_ms = 0;          /* Timeout 완료 */
            _Boss_sched_ready(p_timeout->p_tcb, BOSS_INDICATE_TIMEOUT);
        }
        p_timeout = p_timeout->next;
    } while(p_timeout != _wait_timeout_list);
  }
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                [ TASK ]                                     *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================
    B O S S _ T A S K _ C R E A T E
---------------------------------------------------------------------------*/
BOSS_TID_T Boss_task_create(int (*task_entry)(void *p_arg), void *p_arg,
                                  boss_stk_t *p_stack, boss_uptr_t stk_bytes,
                                            boss_prio_t prio, const char *name)
{
  boss_tcb_t *p_tcb;
  BOSS_ASSERT(_BOSS_ISR_() == 0);
  
  p_tcb = _Boss_tcb_init(task_entry, p_arg, p_stack, stk_bytes, prio, name);
  BOSS_IRQ_DISABLE();
  _Boss_sched_list_insert(p_tcb);
  BOSS_IRQ_RESTORE();
  
  _Boss_schedule();

  return p_tcb;     // Task ID
}


/*===========================================================================
    B O S S _ T A S K _ P R I O R I T Y
---------------------------------------------------------------------------*/
void Boss_task_priority(boss_tcb_t *p_tcb, boss_prio_t new_prio)
{
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

