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
#include "Boss_Q_Msg.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/


/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/


/*===========================================================================
    B O S S _ M S G _ Q _ I N I T
---------------------------------------------------------------------------*/
void Boss_msg_q_init(boss_msg_q_t *msg_q, boss_tcb_t *p_tcb, boss_sigs_t sig)
{
  BOSS_ASSERT(msg_q->owner_tcb == _BOSS_NULL);
  BOSS_ASSERT(msg_q->msg_fifo == _BOSS_NULL);
  
  msg_q->owner_tcb  = p_tcb;
  msg_q->msg_sig    = sig;
  msg_q->msg_fifo   = _BOSS_NULL;
}


/*===========================================================================
    B O S S _ M S G _ S E N D
---------------------------------------------------------------------------*/
void Boss_msg_send(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param)
{
  _msg_fifo_t *p_msg;
  
  p_msg = Boss_malloc(sizeof(_msg_fifo_t));
  
  p_msg->m_cmd  = m_cmd;
  p_msg->param  = param;
  p_msg->next   = _BOSS_NULL;

  BOSS_IRQ_DISABLE();
  if(msg_q->msg_fifo == _BOSS_NULL)
  {
    msg_q->msg_fifo = p_msg;
  }
  else
  {
    _msg_fifo_t *p_tail = msg_q->msg_fifo;

    while(p_tail->next != _BOSS_NULL)
    {
      p_tail = p_tail->next;
    }
    p_tail->next = p_msg;             /* Å¥ Ãß°¡ */
  }
  BOSS_IRQ_RESTORE();
  
  Boss_send(msg_q->owner_tcb, msg_q->msg_sig);
}


/*===========================================================================
    B O S S _ M S G _ R E C E I V E
---------------------------------------------------------------------------*/
msg_cmd_t Boss_msg_receive(boss_msg_q_t *msg_q, boss_uptr_t *p_param)
{
  msg_cmd_t m_cmd = M_CMD_EMPTY;

  BOSS_ASSERT(msg_q->owner_tcb == Boss_self());

  if(msg_q->msg_fifo != _BOSS_NULL)
  {
    _msg_fifo_t *p_msg;
    
    BOSS_IRQ_DISABLE();
    p_msg = msg_q->msg_fifo;
    msg_q->msg_fifo = p_msg->next;
    BOSS_IRQ_RESTORE();

    m_cmd     = p_msg->m_cmd;
    *p_param = p_msg->param;
    
    Boss_mfree(p_msg);
  }

  return m_cmd;
}

