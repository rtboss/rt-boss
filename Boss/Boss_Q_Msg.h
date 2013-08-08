#ifndef _BOSS_MSG_Q_H_
#define _BOSS_MSG_Q_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                          RT-BOSS (Message Queue)                            *
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
typedef struct _msg_fifo_struct {
  msg_cmd_t           m_cmd;
  boss_uptr_t         param;
} _msg_fifo_t;


typedef struct {
  boss_tcb_t    *owner_tcb;
  boss_sigs_t   msg_sig;

  _msg_fifo_t   *msg_fifo;
  boss_u08_t    fifo_max;
  boss_u08_t    fifo_count;
  boss_u08_t    fifo_put;
  boss_u08_t    fifo_get;
} boss_msg_q_t;

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_msg_q_init(boss_msg_q_t *msg_q, _msg_fifo_t *p_fifo, boss_uptr_t fifo_bytes,
                                            boss_tcb_t *p_tcb, boss_sigs_t sig);
boss_reg_t Boss_msg_send(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param);
msg_cmd_t Boss_msg_receive(boss_msg_q_t *msg_q, boss_uptr_t *p_param);

#endif  /* _BOSS_MSG_Q_H_ */
