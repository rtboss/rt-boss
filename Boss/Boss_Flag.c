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
    _flag_link_t *p_link = p_flag_obj->wait_list;

    while(p_link->next != _BOSS_NULL) {       /* find first wait */
        p_link = p_link->next;
    }
    
    do {
      boss_flags_t flags;

      flags = p_flag_obj->flags & p_link->wait_flags;
      if((p_link->wait_opt == __FLAG_OPT_AND) && (flags != p_link->wait_flags)) {
          flags = 0;
      }

      if(flags != 0) {
          _Boss_setting_signal(p_link->p_tcb, SIG_BOSS_SUCCESS);
      }

      p_link = p_link->prev;
    } while(p_link != _BOSS_NULL);
  }
  BOSS_IRQ_RESTORE();

  _Boss_schedule();       /* ������ȯ ����  */
}


/*===========================================================================
    _   B O S S _ F L A G _ O P T _ W A I T
---------------------------------------------------------------------------*/
static boss_flags_t _Boss_flag_opt_wait(boss_flag_obj_t *p_flag_obj,
                boss_flags_t wait_flags, boss_tmr_ms_t timeout, boss_u16_t opt)
{
  _flag_link_t  flag_link;
  boss_flags_t  flags;
  boss_reg_t    irq_storage;
  
  flag_link.prev        = _BOSS_NULL;
  flag_link.next        = _BOSS_NULL;
  flag_link.wait_opt    = opt;
  flag_link.wait_flags  = wait_flags;
  flag_link.p_tcb       = Boss_self();

  BOSS_IRQ_DISABLE_SR(irq_storage);
  flags = p_flag_obj->flags & wait_flags;

  if( (__FLAG_OPT_AND == opt) && (flags != wait_flags) ) {
      flags = 0;
  }
  
  if( (flags == 0) && (timeout != NO_WAIT) )
  {
    /* �̺�Ʈ �÷��� ����Ʈ�� �߰� */
    if(p_flag_obj->wait_list != _BOSS_NULL) {
        p_flag_obj->wait_list->prev = &flag_link;
        flag_link.next = p_flag_obj->wait_list;
    }
    p_flag_obj->wait_list = &flag_link;

    do {
      Boss_sig_clear( Boss_self(), SIG_BOSS_SUCCESS);
      BOSS_IRQ_RESTORE_SR(irq_storage);

      timeout = _Boss_wait_sig_timeout(SIG_BOSS_SUCCESS, timeout);  /* ��� (waiting)*/

      BOSS_IRQ_DISABLE_SR(irq_storage);
      flags = p_flag_obj->flags & wait_flags;
      if( (__FLAG_OPT_AND == opt) && (flags != wait_flags) ) {
          flags = 0;
      }
    } while((flags == 0) && (timeout != 0));

    /* �̺�Ʈ �÷��� ��� ����Ʈ���� ���� */
    if( flag_link.prev == _BOSS_NULL ) {
        p_flag_obj->wait_list = flag_link.next;
    } else {
        flag_link.prev->next  = flag_link.next;
    }

    if(flag_link.next != _BOSS_NULL) {
        flag_link.next->prev = flag_link.prev;
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
