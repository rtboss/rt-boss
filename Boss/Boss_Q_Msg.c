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
void Boss_msg_q_init(boss_msg_q_t *msg_q, _msg_fifo_t *p_fifo, boss_uptr_t fifo_bytes,
                                              boss_tcb_t *p_tcb, boss_sigs_t sig)
{
  BOSS_ASSERT(msg_q->owner_tcb == _BOSS_NULL);
  BOSS_ASSERT(msg_q->msg_fifo == _BOSS_NULL);
  
  msg_q->owner_tcb  = p_tcb;
  msg_q->msg_sig    = sig;
  
  msg_q->msg_fifo   = p_fifo;
  msg_q->fifo_max   = fifo_bytes / sizeof(_msg_fifo_t);
  msg_q->fifo_count = 0;
  msg_q->fifo_put   = 0;
  msg_q->fifo_get   = 0;
}


/*===========================================================================
    B O S S _ M S G _ S E N D
---------------------------------------------------------------------------*/
boss_reg_t Boss_msg_send(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param)
{
  boss_reg_t send = (boss_reg_t)_BOSS_FAILURE;

  BOSS_IRQ_DISABLE();
  if(msg_q->fifo_count < msg_q->fifo_max)
  {
    boss_u08_t put_idx = msg_q->fifo_put;
    
    msg_q->msg_fifo[put_idx].m_cmd  = m_cmd;
    msg_q->msg_fifo[put_idx].param  = param;
    msg_q->fifo_put = ++put_idx % msg_q->fifo_max;
    
    msg_q->fifo_count++;
    send = _BOSS_SUCCESS;
  }
  BOSS_IRQ_RESTORE();

  Boss_send(msg_q->owner_tcb, msg_q->msg_sig);

  return send;
}


/*===========================================================================
    B O S S _ M S G _ R E C E I V E
---------------------------------------------------------------------------*/
msg_cmd_t Boss_msg_receive(boss_msg_q_t *msg_q, boss_uptr_t *p_param)
{
  msg_cmd_t m_cmd = M_CMD_EMPTY;

  BOSS_IRQ_DISABLE();
  if(msg_q->fifo_count != 0)
  {
    boss_u08_t get_idx = msg_q->fifo_get;

    m_cmd     = msg_q->msg_fifo[get_idx].m_cmd;
    *p_param  = msg_q->msg_fifo[get_idx].param;

    msg_q->fifo_get = ++get_idx %  msg_q->fifo_max;

    msg_q->fifo_count--;
  }
  BOSS_IRQ_RESTORE();

  return m_cmd;
}

