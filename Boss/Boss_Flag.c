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
#define __FLAG_OPT_AND      0x00
#define __FLAG_OPT__OR      0x01


/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void          _Boss_schedule(void);
void          _Boss_setting_signal(boss_tcb_t *p_tcb, boss_sigs_t sigs);
boss_tmr_ms_t _Boss_wait_sig_timeout(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout);


/*===========================================================================
    B O S S _ F L A G _ O B J _ I N I T
---------------------------------------------------------------------------*/
void Boss_flag_obj_init(boss_flag_obj_t *p_flag_obj)
{
  p_flag_obj->flags     = 0;
  p_flag_obj->wait_list = _BOSS_NULL;
}


/*===========================================================================
    B O S S _ F L A G _ C L E A R
---------------------------------------------------------------------------*/
void Boss_flag_clear(boss_flag_obj_t *p_flag_obj, boss_flags_t clr_flags)
{
  BOSS_IRQ_DISABLE();
  p_flag_obj->flags = p_flag_obj->flags & ~clr_flags;
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    B O S S _ F L A G _ S E N D
---------------------------------------------------------------------------*/
void Boss_flag_send(boss_flag_obj_t *p_flag_obj, boss_flags_t set_flags)
{
  BOSS_IRQ_DISABLE();
  p_flag_obj->flags = p_flag_obj->flags | set_flags;

  if(p_flag_obj->wait_list != _BOSS_NULL)
  {
    _flag_link_t *p_wait = p_flag_obj->wait_list;

    while(p_wait->next != _BOSS_NULL) {       /* find first wait */
      p_wait = p_wait->next;
    }
    
    do {
      boss_flags_t flags;

      flags = p_flag_obj->flags & p_wait->wait_flags;
      if(p_wait->wait_opt == __FLAG_OPT_AND) {
          if(flags != p_wait->wait_flags) {
              flags = 0;
          }
      }

      if(flags != 0) {
          _Boss_setting_signal(p_wait->p_tcb, SIG_BOSS_SUCCESS);
      }

      p_wait = p_wait->prev;
    } while(p_wait != _BOSS_NULL);
  }
  BOSS_IRQ_RESTORE();

  _Boss_schedule();       /* 문맥전환 실행  */
}


/*===========================================================================
    _   B O S S _ F L A G _ O P T _ W A I T
---------------------------------------------------------------------------*/
static boss_flags_t _Boss_flag_opt_wait(boss_flag_obj_t *p_flag_obj,
                boss_flags_t wait_flags, boss_tmr_ms_t timeout, boss_u16_t opt)
{
  _flag_link_t  f_wait;
  boss_flags_t  flags;
  boss_reg_t    irq_storage;
  
  f_wait.prev   = _BOSS_NULL;
  f_wait.next   = _BOSS_NULL;
  f_wait.wait_opt   = opt;
  f_wait.wait_flags = wait_flags;
  f_wait.p_tcb  = Boss_self();

  BOSS_IRQ_DISABLE_SR(irq_storage);
  flags = p_flag_obj->flags & wait_flags;

  if( (__FLAG_OPT_AND == opt) && (flags != wait_flags) ) {
      flags = 0;
  }
  
  if( (flags == 0) && (timeout != NO_WAIT) )
  {
    /* 이벤트 플래그 리스트에 추가 */
    if(p_flag_obj->wait_list != _BOSS_NULL) {
        p_flag_obj->wait_list->prev = &f_wait;
        f_wait.next = p_flag_obj->wait_list;
    }
    p_flag_obj->wait_list = &f_wait;

    do {
      Boss_sig_clear( Boss_self(), SIG_BOSS_SUCCESS);
      BOSS_IRQ_RESTORE_SR(irq_storage);

      timeout = _Boss_wait_sig_timeout(SIG_BOSS_SUCCESS, timeout);  /* 대기 (waiting)*/

      BOSS_IRQ_DISABLE_SR(irq_storage);
      flags = p_flag_obj->flags & wait_flags;
      if( (__FLAG_OPT_AND == opt) && (flags != wait_flags) ) {
          flags = 0;
      }
    } while((flags == 0) && (timeout != 0));

    /* 이벤트 플래그 리스트에서 제거 */
    if(f_wait.prev == _BOSS_NULL) {
        BOSS_ASSERT(p_flag_obj->wait_list == &f_wait);
        p_flag_obj->wait_list    = f_wait.next;
    } else {
        f_wait.prev->next = f_wait.next;
    }

    if(f_wait.next != _BOSS_NULL) {
        f_wait.next->prev = f_wait.prev;
    }
  }
  p_flag_obj->flags = p_flag_obj->flags & ~flags;     // receive flags clear
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return flags;
}


/*===========================================================================
    B O S S _ F L A G _ W A I T
---------------------------------------------------------------------------*/
boss_flags_t Boss_flag_wait(boss_flag_obj_t *p_flag_obj,
                                boss_flags_t wait_flags, boss_tmr_ms_t timeout)
{
  return _Boss_flag_opt_wait(p_flag_obj, wait_flags, timeout, __FLAG_OPT__OR);
}


/*===========================================================================
    B O S S _ F L A G _ W A I T _ A N D
---------------------------------------------------------------------------*/
boss_flags_t Boss_flag_wait_and(boss_flag_obj_t *p_flag_obj,
                                boss_flags_t wait_flags, boss_tmr_ms_t timeout)
{
  return _Boss_flag_opt_wait(p_flag_obj, wait_flags, timeout, __FLAG_OPT_AND);
}
