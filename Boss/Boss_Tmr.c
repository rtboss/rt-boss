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
#include "Boss.h"

#ifdef _BOSS_TMR_H_       // 사용 시 "Boss_Conf.h"에 #include "Boss_Tmr.h" 포함

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

/*static*/ void (*_tmr_exe_notify)(void) = _BOSS_NULL;  /* Timer Execute Notify */


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/


#ifdef _BOSS_MEMORY_H_
/*===========================================================================
    B O S S _ T M R _ C R E A T E
---------------------------------------------------------------------------*/
boss_tmr_t *Boss_tmr_create(void)
{
  boss_tmr_t *p_tmr;

  p_tmr = Boss_malloc(sizeof(boss_tmr_t));

  if(p_tmr != _BOSS_NULL) {
      p_tmr->prev = _BOSS_NULL;
      p_tmr->next = _BOSS_NULL;
      p_tmr->tmr_ms = 0;
  }

  return p_tmr;   // Timer ID
}


/*===========================================================================
    B O S S _ T M R _ M F R E E _ D E L
---------------------------------------------------------------------------*/
void Boss_tmr_mfree_del(boss_tmr_t *p_tmr)
{
  BOSS_ASSERT(p_tmr != _BOSS_NULL);
  
  Boss_tmr_stop(p_tmr);     // 등록된 Timer이면 중지
  Boss_mfree(p_tmr);
}
#endif /* _BOSS_MEMORY_H_ */


/*===========================================================================
    B O S S _ T M R _ S T A R T
---------------------------------------------------------------------------*/
void Boss_tmr_start(boss_tmr_t *p_tmr, boss_tmr_ms_t tmr_ms, tmr_cb_t callback)
{
  Boss_tmr_stop(p_tmr);     // 등록된 Timer이면 중지
  
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
    
    p_tmr->prev = _BOSS_NULL;
  }
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    _   B O S S _ T I M E R _ N O T I F Y _ S E T
---------------------------------------------------------------------------*/
void _Boss_timer_notify_set( void (*notify)(void) )
{
  BOSS_IRQ_DISABLE();
  _tmr_exe_notify = notify;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    _   B O S S _ T I M E R _ C A L L B A C K _ E X E C U T E
---------------------------------------------------------------------------*/
void _Boss_timer_callback_execute(void)
{
  boss_reg_t  irq_storage;
  
  _Boss_sched_lock();
  BOSS_IRQ_DISABLE_SR(irq_storage);
  while(_boss_timer_exe_list != _BOSS_NULL)
  {
    boss_tmr_t *p_done = _boss_timer_exe_list;
    _boss_timer_exe_list = p_done->next;
    
    BOSS_ASSERT(p_done->prev == _TRIMER_EXE_FIRST_PREV);
    
    if(p_done->next != _BOSS_NULL) {
      p_done->next->prev = _TRIMER_EXE_FIRST_PREV;
    }
    p_done->prev = _BOSS_NULL;
    p_done->next = _BOSS_NULL;
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    p_done->tmr_cb(p_done);   // Callback Execute
    
    BOSS_IRQ_DISABLE_SR(irq_storage);
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);
  _Boss_sched_free();
}


/*===========================================================================
    _   B O S S _ T I M E R _ T I C K                     [ Timer Tick ISR ]
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

      p_done->tmr_ms = 0;       // 완료된 타이머 tmr_ms "0"으로 설정.

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
    if(_tmr_exe_notify != _BOSS_NULL) {
      _tmr_exe_notify();
    } else {
      _Boss_timer_callback_execute();
    }
  }
}

#endif  /* _BOSS_TMR_H_ */
