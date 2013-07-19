#ifndef _BOSS_Q_MBOX_H_
#define _BOSS_Q_MBOX_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Mail Box)                             *
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
typedef enum {
  _MBOX_INVALID = 0,
  _MBOX_READY,
  _MBOX_EXECUTE
} _mbox_state_t;


typedef struct _mbox_head_struct {
  _mbox_state_t     state;
  
  boss_tcb_t        *sender;    /* Sender TCB */
  boss_uptr_t       *p_rsp;     /* Response  NULL = Send  : !NULL = Pend */
  
  struct _mbox_head_struct  *next;
} _mbox_head_t;


typedef struct {
  boss_tcb_t      *owner_tcb;
  boss_sigs_t     mbox_sig;

  _mbox_head_t    *mbox_fifo;
} boss_mbox_q_t;

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_mbox_q_init(boss_mbox_q_t *mbox_q, boss_tcb_t *p_tcb, boss_sigs_t sig);

void *Boss_mbox_alloc(boss_uptr_t size);
void Boss_mbox_free(void *p_mbox);

void Boss_mbox_send(boss_mbox_q_t *mbox_q, void *p_mbox);
boss_reg_t Boss_mbox_pend(boss_mbox_q_t *mbox_q, void *p_mbox,
                                    boss_uptr_t *p_rsp, boss_tmr_ms_t timeout);

void *Boss_mbox_receive(boss_mbox_q_t *mail_q);
void Boss_mbox_pend_done(void *p_mbox, boss_uptr_t rsp);

#endif  /* _BOSS_Q_MBOX_H_ */
