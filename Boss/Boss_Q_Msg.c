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
typedef enum {
  __MSG_SEND__REAR  = 0x00,
  __MSG_SEND_FRONT  = 0x01,
} _msg_send_opt_t;

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
    B O S S _ M S G _ Q _ I N I T
---------------------------------------------------------------------------*/
void Boss_msg_q_init(boss_msg_q_t *msg_q, boss_msg_t *p_fifo,
                                    boss_uptr_t fifo_bytes, _msg_q_opt_t q_opt)
{
  if(p_fifo != _BOSS_NULL) {
      msg_q->msg_fifo = p_fifo;
      msg_q->fifo_max = fifo_bytes / sizeof(boss_msg_t);
      BOSS_ASSERT(0 < msg_q->fifo_max);
  } else {
      msg_q->msg_fifo = _BOSS_NULL;
      msg_q->fifo_max = 0;
  }
  
  msg_q->fifo_count = 0;
  msg_q->fifo_head  = 0;
  msg_q->fifo_tail  = 0;

  msg_q->q_opt      = q_opt;
  
  msg_q->wait_list  = _BOSS_NULL;
}


/*===========================================================================
    _   B O S S _ M S G _ O P T _ S E N D
---------------------------------------------------------------------------*/
static boss_reg_t _Boss_msg_opt_send( boss_msg_q_t *msg_q, msg_cmd_t m_cmd,
                                    boss_uptr_t param, _msg_send_opt_t send_opt)
{
  boss_reg_t  sent = (boss_reg_t)_BOSS_FAILURE;
  boss_reg_t  irq_storage;
  
  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(msg_q->wait_list != _BOSS_NULL)      /* Msg wait TCB가 있을때 직접 전송 */
  {
    _msg_link_t *p_best = msg_q->wait_list;

    if(msg_q->q_opt == MSG_Q_PRIORITY)            /* Find Best */
    {
      _msg_link_t *p_find = p_best->next;
      
      while(p_find != _BOSS_NULL)
      {
        if(p_find->p_tcb->prio <= p_best->p_tcb->prio) {
            p_best = p_find;
        }
        p_find = p_find->next;
      }
    }
    else  /* MSG_Q_FIFO */                        /* Find First */
    {
      while(p_best->next != _BOSS_NULL)
      {
        p_best = p_best->next;
      }
    }
    
    /* 메시지 대기 리스트에서 제거 (메시지 전송) */
    if(p_best->prev == _BOSS_NULL) {
        msg_q->wait_list    = p_best->next;
    } else {
        p_best->prev->next  = p_best->next;
    }

    if(p_best->next != _BOSS_NULL) {
        p_best->next->prev = p_best->prev;
    }

    p_best->msg.m_cmd = m_cmd;
    p_best->msg.param = param;

    _Boss_sched_ready(p_best->p_tcb, BOSS_INDICATE_SUCCESS);
    
    sent = _BOSS_SUCCESS;
  }
  else if(msg_q->fifo_count < msg_q->fifo_max)     /* Msg fifo 에 넣는다 */
  {
    if( send_opt == __MSG_SEND__REAR )        // __MSG_SEND__REAR
    {
      boss_u08_t head_idx = msg_q->fifo_head;
    
      msg_q->msg_fifo[head_idx].m_cmd  = m_cmd;
      msg_q->msg_fifo[head_idx].param  = param;
      msg_q->fifo_head = ++head_idx % msg_q->fifo_max;
    }
    else                                      // __MSG_SEND_FRONT
    {
      boss_u08_t tail_idx = msg_q->fifo_tail;

      if( tail_idx == 0 ) {
        tail_idx = msg_q->fifo_max - 1;
      } else {
        --tail_idx;
      }

      msg_q->msg_fifo[tail_idx].m_cmd  = m_cmd;
      msg_q->msg_fifo[tail_idx].param  = param;
      msg_q->fifo_tail = tail_idx;
    }

    msg_q->fifo_count++;    
    sent = _BOSS_SUCCESS;
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);
  
  _Boss_schedule();
  
  return sent;
}


/*===========================================================================
    B O S S _ M S G _ S E N D
---------------------------------------------------------------------------*/
boss_reg_t Boss_msg_send(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param)
{
  return _Boss_msg_opt_send(msg_q, m_cmd, param, __MSG_SEND__REAR);
}


/*===========================================================================
    B O S S _ M S G _ S E N D _ F R O N T
---------------------------------------------------------------------------*/
boss_reg_t Boss_msg_send_front(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param)
{
  return _Boss_msg_opt_send(msg_q, m_cmd, param, __MSG_SEND_FRONT);
}


/*===========================================================================
    B O S S _ M S G _ W A I T
---------------------------------------------------------------------------*/
boss_msg_t Boss_msg_wait(boss_msg_q_t *msg_q, boss_tmr_ms_t timeout)
{  
  _msg_link_t   msg_wait;
  boss_reg_t    irq_storage;
  
  BOSS_ASSERT((_BOSS_ISR_() == 0) || (timeout == NO_WAIT));
  
  Boss_self()->indicate = BOSS_INDICATE_CLEAR;
  
  msg_wait.prev   = _BOSS_NULL;
  msg_wait.next   = _BOSS_NULL;
  msg_wait.p_tcb  = Boss_self();
  msg_wait.msg.m_cmd  = M_CMD_EMPTY;
  //msg_wait.msg.param  = 0;
  
  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(msg_q->fifo_count != 0)                                    /* 메시지가 있을때 */
  {
    boss_u08_t  tail_idx = msg_q->fifo_tail;

    msg_wait.msg.m_cmd = msg_q->msg_fifo[tail_idx].m_cmd;
    msg_wait.msg.param = msg_q->msg_fifo[tail_idx].param;

    msg_q->fifo_tail = ++tail_idx %  msg_q->fifo_max;
    msg_q->fifo_count--;    
  }
  else if( timeout != NO_WAIT )
  {
    /* 메시지 대기 리스트에 추가 */
    if(msg_q->wait_list != _BOSS_NULL) {
        msg_q->wait_list->prev  = &msg_wait;
        msg_wait.next = msg_q->wait_list;
    }
    msg_q->wait_list = &msg_wait;
    BOSS_IRQ_RESTORE_SR(irq_storage);

    (void)_Boss_sched_wait(timeout);              /* 대기 (waiting)  */

    BOSS_IRQ_DISABLE_SR(irq_storage);
    if((Boss_self()->indicate & BOSS_INDICATE_SUCCESS) == 0)    /* 타임아웃 */
    {
      /* 메시지 대기 리스트에서 제거 (메시지 타임 아웃) */
      if(msg_wait.prev == _BOSS_NULL) {
          msg_q->wait_list    = msg_wait.next;
      } else {
          msg_wait.prev->next = msg_wait.next;
      }
    
      if(msg_wait.next != _BOSS_NULL) {
          msg_wait.next->prev = msg_wait.prev;
      }

      BOSS_ASSERT(msg_wait.msg.m_cmd == M_CMD_EMPTY);
    }
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return msg_wait.msg;
}
