/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Timer)                                *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss_Tmr.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
#define _TRIMER_ACT_FIRST_PREV    ((boss_tmr_t *)(0xFFFFFFFF))
#define _TRIMER_EXE_FIRST_PREV    ((boss_tmr_t *)(0xFFFFFFFE))

/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/
/*static*/ boss_tmr_t *_boss_timer_act_list = _BOSS_NULL; /* Active Timer list */
/*static*/ boss_tmr_t *_boss_timer_exe_list = _BOSS_NULL; /* Execute Timer list*/

boss_tcb_t *_p_tmr_cb_tcb = _BOSS_NULL;
boss_sigs_t _tmr_cb_sig   = 0;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/


/*===========================================================================
    _   B O S S _ T I M E R _ C B _ T A S K _ S E T
---------------------------------------------------------------------------*/
void _Boss_timer_cb_task_set(boss_tcb_t *p_cb_tcb, boss_sigs_t cb_sig)
{
  BOSS_IRQ_DISABLE();
  _p_tmr_cb_tcb = p_cb_tcb;
  _tmr_cb_sig   = cb_sig;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    _   B O S S _ T I M E R _ T I C K
---------------------------------------------------------------------------*/
void _Boss_timer_tick(boss_tmr_ms_t tick_ms)
{
  boss_tmr_t  *p_tmr;
  
  BOSS_IRQ_DISABLE();
  p_tmr = _boss_timer_act_list;

  while(p_tmr != _BOSS_NULL)
  {
    if(tick_ms < p_tmr->tmr_ms)
    {
      p_tmr->tmr_ms = p_tmr->tmr_ms - tick_ms;
      p_tmr = p_tmr->next;
    }
    else  /* Timer done */
    {
      boss_tmr_t  *p_done = p_tmr;
      p_tmr = p_tmr->next;

      /* Remove Active list */
      if(p_done->prev == _TRIMER_ACT_FIRST_PREV) {
        _boss_timer_act_list = p_done->next;
      } else {
        p_done->prev->next = p_done->next;
      }
      
      if(p_done->next != _BOSS_NULL) {
        p_done->next->prev = p_done->prev;
      }

      /* Insert Execute list */
      p_done->prev = _TRIMER_EXE_FIRST_PREV;
      p_done->next = _BOSS_NULL;

      if(_boss_timer_exe_list != _BOSS_NULL) {
        _boss_timer_exe_list->prev = p_done;
        p_done->next = _boss_timer_exe_list;
      }
      _boss_timer_exe_list = p_done;
    }
  }
  BOSS_IRQ_RESTORE();

  if(_boss_timer_exe_list != _BOSS_NULL)
  {
    if(_p_tmr_cb_tcb != _BOSS_NULL) {
      Boss_send(_p_tmr_cb_tcb, _tmr_cb_sig);
    } else {
      _Boss_timer_callback_execute();
    }
  }
}


/*===========================================================================
    _   B O S S _ T I M E R _ C A L L B A C K _ E X E C U T E
---------------------------------------------------------------------------*/
void _Boss_timer_callback_execute(void)
{
  _Boss_sched_lock();
  while(_boss_timer_exe_list != _BOSS_NULL)
  {
    boss_tmr_t  *p_done;
    tmr_cb_t    callback;
    
    BOSS_IRQ_DISABLE();
    p_done = _boss_timer_exe_list;
    _boss_timer_exe_list = _boss_timer_exe_list->next;
    if(_boss_timer_exe_list != _BOSS_NULL) {
      _boss_timer_exe_list->prev = _TRIMER_EXE_FIRST_PREV;
    }
    p_done->prev = _BOSS_NULL;
    p_done->next = _BOSS_NULL;
    callback = p_done->tmr_cb;
    BOSS_IRQ_RESTORE();

    callback(p_done);   // Callback Execute
  }
  _Boss_sched_free();
}


/*===========================================================================
    B O S S _ T M R _ S T A R T
---------------------------------------------------------------------------*/
void Boss_tmr_start(boss_tmr_t *p_tmr, boss_tmr_ms_t tmr_ms, tmr_cb_t callback)
{
  if(p_tmr->prev != _BOSS_NULL) {
      Boss_tmr_stop(p_tmr);
  }
  
  p_tmr->tmr_ms = tmr_ms;
  p_tmr->tmr_cb = callback;
  
  p_tmr->prev = _TRIMER_ACT_FIRST_PREV;
  p_tmr->next = _BOSS_NULL;

  BOSS_IRQ_DISABLE();
  if(_boss_timer_act_list != _BOSS_NULL)
  {
    BOSS_ASSERT(_boss_timer_act_list->prev == _TRIMER_ACT_FIRST_PREV);
    
    _boss_timer_act_list->prev = p_tmr;
    p_tmr->next = _boss_timer_act_list;
  }
  _boss_timer_act_list = p_tmr;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ T M R _ S T O P
---------------------------------------------------------------------------*/
void Boss_tmr_stop(boss_tmr_t *p_tmr)
{
  BOSS_IRQ_DISABLE();
  if(p_tmr->prev != _BOSS_NULL)
  {
    if(p_tmr->prev == _TRIMER_ACT_FIRST_PREV) {
      _boss_timer_act_list = _boss_timer_act_list->next;
    } else if(p_tmr->prev == _TRIMER_EXE_FIRST_PREV) {
      _boss_timer_exe_list = _boss_timer_exe_list->next;
    } else {
      p_tmr->prev->next = p_tmr->next;
    }
    
    if(p_tmr->next != _BOSS_NULL) {
      p_tmr->next->prev = p_tmr->prev;
    }
  }
  p_tmr->prev = _BOSS_NULL;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ S L E E P
---------------------------------------------------------------------------*/
void Boss_sleep(boss_tmr_ms_t wait_ms)
{
  (void)Boss_wait_sleep( (boss_sigs_t)0, wait_ms);
}

/* Sleep Timer */
typedef struct {
  boss_tmr_t  tmr;
  boss_tcb_t  *p_tcb;
} _sleep_tmr_t;


/*===========================================================================
    _ S L E E P _ C A L L B A C K
---------------------------------------------------------------------------*/
void _sleep_callback(boss_tmr_t *p_tmr)
{
  _sleep_tmr_t *p_sleep_tmr = (_sleep_tmr_t *)p_tmr;

  Boss_send(p_sleep_tmr->p_tcb, BOSS_SIG_SLEEP);
}


/*===========================================================================
    B O S S _ W A I T _ S L E E P
---------------------------------------------------------------------------*/
boss_sigs_t Boss_wait_sleep(boss_sigs_t wait_sigs,  boss_tmr_ms_t wait_ms)
{
  boss_sigs_t   recv_sigs;
  
  if(wait_ms != 0)
  {
    _sleep_tmr_t  sleep_tmr;

    sleep_tmr.tmr.prev  = _BOSS_NULL;
    sleep_tmr.p_tcb     = Boss_self();
    
    Boss_tmr_start((boss_tmr_t *)&sleep_tmr, wait_ms, _sleep_callback);

    recv_sigs = Boss_wait(wait_sigs | BOSS_SIG_SLEEP);

    if( (recv_sigs & BOSS_SIG_SLEEP) == 0 ) {
      Boss_tmr_stop((boss_tmr_t *)&sleep_tmr);
    }
    
    recv_sigs = recv_sigs & ~BOSS_SIG_SLEEP;   /* BOSS_SIG_SLEEP 시그널 클리어 */
  }
  else
  {
    recv_sigs = Boss_wait( wait_sigs );
  }
  
  return recv_sigs;
}

