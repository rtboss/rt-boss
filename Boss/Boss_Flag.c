/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Flag)                                 *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss_Flag.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/



/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_schedule(void);
void _Boss_sched_ready(boss_tcb_t *p_tcb, boss_u08_t indicate);
boss_tmr_ms_t _Boss_sched_wait(boss_tmr_ms_t timeout);


/*===========================================================================
    B O S S _ F L A G _ G R P _ I N I T
---------------------------------------------------------------------------*/
void Boss_flag_grp_init(boss_flag_grp_t *p_grp)
{
  p_grp->flags      = 0;
  p_grp->wait_list  = _BOSS_NULL;
}


/*===========================================================================
    B O S S _ F L A G _ C L E A R
---------------------------------------------------------------------------*/
void Boss_flag_clear(boss_flag_grp_t *p_grp, boss_flags_t clr_flags)
{
  BOSS_IRQ_DISABLE();
  p_grp->flags = p_grp->flags & ~clr_flags;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ F L A G _ S E N D
---------------------------------------------------------------------------*/
void Boss_flag_send(boss_flag_grp_t *p_grp, boss_flags_t set_flags)
{
  BOSS_IRQ_DISABLE();
  p_grp->flags = p_grp->flags | set_flags;

  if(p_grp->wait_list != _BOSS_NULL)
  {
    _flag_link_t *p_link = p_grp->wait_list;
    
    do {
      if( (p_link->wait_flags & set_flags) != 0 )
      {
        _Boss_sched_ready(p_link->p_tcb, BOSS_INDICATE_SUCCESS);
      }
      
      p_link = p_link->next;
    } while(p_link != p_grp->wait_list);
  }
  BOSS_IRQ_RESTORE();

  _Boss_schedule();       /* 문맥전환 실행  */
}


/*===========================================================================
    B O S S _ F L A G _ W A I T
---------------------------------------------------------------------------*/
boss_flags_t Boss_flag_wait(boss_flag_grp_t *p_grp, boss_flags_t wait_flags,
                                    boss_reg_t wait_opt, boss_tmr_ms_t timeout)
{
  _flag_link_t  flag_link;
  boss_flags_t  flags;
  boss_reg_t    irq_storage;
  
  BOSS_ASSERT((_BOSS_ISR_() == 0) || (timeout == NO_WAIT));
  
  //flag_link.prev        = &flag_link;
  //flag_link.next        = &flag_link;
  flag_link.wait_flags  = wait_flags;
  flag_link.p_tcb       = Boss_self();

  BOSS_IRQ_DISABLE_SR(irq_storage);
  flags = p_grp->flags & wait_flags;

  if( (wait_opt & _FLAG_OPT_AND) && (flags != wait_flags) ) {
      flags = 0;
  }
  
  if( (flags == 0) && (timeout != NO_WAIT) )
  {
    /* 이벤트 플래그 리스트에 추가 */
    if( p_grp->wait_list == _BOSS_NULL ) {    // 첫번째 등록
        flag_link.prev    = &flag_link;
        flag_link.next    = &flag_link;
        p_grp->wait_list  = &flag_link;
    } else {                                  // 추가 (Tail)
        _flag_link_t *p_head = p_grp->wait_list;
        
        flag_link.prev = p_head->prev;
        flag_link.next = p_head;        // p_head->prev->next 와 동일
        p_head->prev->next  = &flag_link;
        p_head->prev        = &flag_link;
    }

    do {
      Boss_self()->indicate = BOSS_INDICATE_CLEAR;
      BOSS_IRQ_RESTORE_SR(irq_storage);

      timeout = _Boss_sched_wait(timeout);          /* 대기 (waiting)*/

      BOSS_IRQ_DISABLE_SR(irq_storage);
      flags = p_grp->flags & wait_flags;
      if( (wait_opt & _FLAG_OPT_AND) && (flags != wait_flags) ) {
          flags = 0;
      }
    } while((flags == 0) && (timeout != 0));

    /* 이벤트 플래그 대기 리스트에서 제거 */
    flag_link.prev->next = flag_link.next;
    flag_link.next->prev = flag_link.prev;

    if(p_grp->wait_list == &flag_link)
    {
      if(flag_link.prev == &flag_link) {
          BOSS_ASSERT(flag_link.next == &flag_link);
          p_grp->wait_list = _BOSS_NULL;    // 마지막 제거
      } else {
          p_grp->wait_list = p_grp->wait_list->next;
      }
    }
  }
  
  if(wait_opt & _FLAG_OPT_CONSUME) {
      p_grp->flags = p_grp->flags & ~flags;     // receive flags clear
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return flags;
}
