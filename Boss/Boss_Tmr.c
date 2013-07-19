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


/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/
/*static*/ boss_tmr_t   *_boss_timer_list = _BOSS_NULL; /* 카운트할 타이머 리스트 */

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/

/*===========================================================================
    _   B O S S _ T I M E R _ T I C K
---------------------------------------------------------------------------*/
void _Boss_timer_tick(boss_tmr_ms_t tick_ms)
{
  boss_tmr_t  *p_prev;
  boss_tmr_t  *p_tmr;
  boss_tmr_t  *p_done_list = _BOSS_NULL;
  
  BOSS_IRQ_DISABLE();
  p_prev  = _BOSS_NULL;
  p_tmr   = _boss_timer_list;
  
  while(p_tmr != _BOSS_NULL)
  {    
    if(tick_ms < p_tmr->tmr_ms)
    {
      p_tmr->tmr_ms = p_tmr->tmr_ms - tick_ms;
      
      p_prev  = p_tmr;
      p_tmr   = p_tmr->next;
    }
    else  /* 타이머 완료 */
    {
      boss_tmr_t  *p_done;
      
      if(p_prev == _BOSS_NULL) {
        _boss_timer_list = p_tmr->next;
      } else {
        p_prev->next = p_tmr->next;
      }
      
      p_done  = p_tmr;
      p_tmr   = p_tmr->next;
      
      p_done->next = p_done_list;
      p_done_list = p_done;
    }
  }
  BOSS_IRQ_RESTORE();

  while(p_done_list != _BOSS_NULL)        /* 완료 타이머 처리 */
  {
    boss_tmr_cb_t callback;
    boss_tmr_t    *p_done;
    
    p_done = p_done_list;
    p_done_list = p_done->next;
    
    if(p_done->sig != 0) {                /* 시그널 처리 */
      Boss_send(p_done->p_tcb, p_done->sig);
    }

    callback = p_done->callback;

    if(p_done->rpt_ms != 0)                /* 반복 타이머 재등록 */
    {
      p_done->tmr_ms = p_done->rpt_ms;
      BOSS_IRQ_DISABLE();
      p_done->next = _boss_timer_list;
      _boss_timer_list = p_done;
      BOSS_IRQ_RESTORE();
    }
    else
    {
      Boss_mfree(p_done);
    }

    if(callback != _BOSS_NULL) {        /* 콜백 함수 실행 */
      callback();
    }
  }
}



/*===========================================================================
    B O S S _ T M R _ S T A R T
---------------------------------------------------------------------------*/
boss_tmr_t *Boss_tmr_start( boss_tcb_t *p_tcb, boss_sigs_t sig,
            boss_tmr_ms_t tmr_ms, boss_tmr_ms_t rpt_ms, boss_tmr_cb_t callback )
{
  boss_tmr_t *p_tmr;

  BOSS_ASSERT(p_tcb != _BOSS_NULL);

  p_tmr = Boss_malloc(sizeof(boss_tmr_t));

  p_tmr->p_tcb  = p_tcb;
  p_tmr->sig    = sig;

  p_tmr->tmr_ms = tmr_ms;
  p_tmr->rpt_ms = rpt_ms;

  p_tmr->callback = callback;

  BOSS_IRQ_DISABLE();
  p_tmr->next = _boss_timer_list;         /* 타이머 추가 */
  _boss_timer_list = p_tmr;
  
  Boss_sigs_clear(p_tcb, sig);
  BOSS_IRQ_RESTORE();

  return p_tmr;
}


/*===========================================================================
    B O S S _ T M R _ S T O P
---------------------------------------------------------------------------*/
boss_reg_t Boss_tmr_stop(boss_tmr_t *p_tmr)
{
  boss_tmr_t *p_prev;
  boss_tmr_t *p_find;

  BOSS_IRQ_DISABLE();
  p_prev = _BOSS_NULL;
  p_find = _boss_timer_list;
  while( (p_find != _BOSS_NULL) && (p_find != p_tmr) )
  {
    p_prev = p_find;
    p_find = p_find->next;
  }

  if(p_find != _BOSS_NULL)        /* 타이머 제거 */
  {
    if(p_prev == _BOSS_NULL) {
      _boss_timer_list = p_find->next;
    } else {
      p_prev->next = p_find->next;
    }
  }
  BOSS_IRQ_RESTORE();
  
  if(p_find != _BOSS_NULL) {
    Boss_mfree(p_find);
    return _BOSS_TRUE;
  }

  return _BOSS_FALSE;
}


/*===========================================================================
    B O S S _ T M R _ C A N C E L
---------------------------------------------------------------------------*/
boss_reg_t Boss_tmr_cancel(boss_tcb_t *p_tcb, boss_sigs_t sig,
                                                        boss_tmr_cb_t callback)
{
  boss_tmr_t *p_prev;
  boss_tmr_t *p_find;

  BOSS_IRQ_DISABLE();
  p_prev = _BOSS_NULL;
  p_find = _boss_timer_list;
  
  while( p_find != _BOSS_NULL )
  {
    if( (p_find->p_tcb == p_tcb) && (p_find->sig == sig) 
                    && (p_find->callback == callback) )
    {
      break;
    }
    
    p_prev = p_find;
    p_find = p_find->next;
  }
  
  if(p_find != _BOSS_NULL)        /* 타이머 제거 */
  {
    if(p_prev == _BOSS_NULL) {
      _boss_timer_list = p_find->next;
    } else {
      p_prev->next = p_find->next;
    }
  }
  BOSS_IRQ_RESTORE();

  if(p_find != _BOSS_NULL) {
    Boss_mfree(p_find);
    return _BOSS_TRUE;
  }

  return _BOSS_FALSE;
}


/*===========================================================================
    B O S S _ S L E E P
---------------------------------------------------------------------------*/
void Boss_sleep(boss_tmr_ms_t wait_ms)
{
  Boss_tmr_start(Boss_self(), BOSS_SIG_SLEEP, wait_ms, _BOSS_FALSE, _BOSS_NULL);

  Boss_wait(BOSS_SIG_SLEEP);
}


/*===========================================================================
    B O S S _ W A I T _ S L E E P
---------------------------------------------------------------------------*/
boss_sigs_t Boss_wait_sleep(boss_sigs_t wait_sigs,  boss_tmr_ms_t wait_ms)
{
  boss_sigs_t recv_sigs;
  
  Boss_tmr_start(Boss_self(), BOSS_SIG_SLEEP, wait_ms, _BOSS_FALSE, _BOSS_NULL);

  recv_sigs = Boss_wait(wait_sigs | BOSS_SIG_SLEEP);

  if( (recv_sigs & BOSS_SIG_SLEEP) == 0 ) {
    Boss_tmr_cancel(Boss_self(), BOSS_SIG_SLEEP, _BOSS_NULL);
  }
  
  return recv_sigs;
}

