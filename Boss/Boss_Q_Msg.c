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
void          _Boss_wait_sig_forever(boss_sigs_t wait_sigs);
boss_tmr_ms_t _Boss_wait_sig_timeout(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout);


/*===========================================================================
    B O S S _ M S G _ Q _ I N I T
---------------------------------------------------------------------------*/
void Boss_msg_q_init(boss_msg_q_t *msg_q, boss_msg_t *p_fifo,
                                    boss_uptr_t fifo_bytes, _msg_q_opt_t q_opt)
{
  msg_q->wait_list  = _BOSS_NULL;
  
  if(p_fifo != _BOSS_NULL) {
      msg_q->msg_fifo = p_fifo;
      msg_q->fifo_max = fifo_bytes / sizeof(boss_msg_t);
  } else {
      msg_q->msg_fifo = _BOSS_NULL;
      msg_q->fifo_max = 0;
  }
  
  msg_q->fifo_count = 0;
  msg_q->fifo_head  = 0;
  msg_q->fifo_tail  = 0;

  msg_q->q_opt      = q_opt;
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
  if(msg_q->wait_list != _BOSS_NULL)      /* Msg Pend TCB가 있을때 직접 전송 */
  {
    _msg_wait_t *p_best = msg_q->wait_list;

    if(msg_q->q_opt == MSG_Q_PRIORITY)
    {
      _msg_wait_t *p_find = p_best->next;
      
      while(p_find != _BOSS_NULL)         /* Find Best */
      {
        if(p_find->p_tcb->prio <= p_best->p_tcb->prio) {
            p_best = p_find;
        }
        p_find = p_find->next;
      }
    }
    else
    {
      while(p_best->next != _BOSS_NULL)  /* Find First */
      {
        p_best = p_best->next;
      }
    }
    
    /* 메시지 대기 리스트에서 제거 (메시지 전송 완료) */
    if(p_best->prev == _BOSS_NULL) {
        msg_q->wait_list    = p_best->next;
    } else {
        p_best->prev->next  = p_best->next;
    }

    if(p_best->next != _BOSS_NULL) {
        p_best->next->prev = p_best->prev;
    }

    _Boss_sched_lock();
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    p_best->msg.m_cmd = m_cmd;
    p_best->msg.param = param;
    
    Boss_sig_send(p_best->p_tcb, SIG_BOSS_SUCCESS);
    _Boss_sched_free();
    
    return _BOSS_SUCCESS;
  }

  if(msg_q->fifo_count < msg_q->fifo_max)     /* Msg fifo 에 넣는다 */
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
    B O S S _ M S G _ R E C E I V E
---------------------------------------------------------------------------*/
boss_msg_t Boss_msg_receive(boss_msg_q_t *msg_q)
{
  boss_msg_t  msg;

  msg.m_cmd = M_CMD_EMPTY;
  
  BOSS_IRQ_DISABLE();
  if(msg_q->fifo_count != 0)                    /* 메시지가 있을때 */
  {
    boss_u08_t  tail_idx = msg_q->fifo_tail;

    msg.m_cmd = msg_q->msg_fifo[tail_idx].m_cmd;
    msg.param = msg_q->msg_fifo[tail_idx].param;

    msg_q->fifo_tail = ++tail_idx %  msg_q->fifo_max;

    msg_q->fifo_count--;
  }
  BOSS_IRQ_RESTORE();

  return msg;
}


/*===========================================================================
    B O S S _ M S G _ W A I T
---------------------------------------------------------------------------*/
boss_msg_t Boss_msg_wait(boss_msg_q_t *msg_q, boss_tmr_ms_t timeout)
{  
  _msg_wait_t   msg_wait;
  boss_reg_t    irq_storage;
  
  Boss_sig_clear( Boss_self(), SIG_BOSS_SUCCESS );

  msg_wait.prev   = _BOSS_NULL;
  msg_wait.next   = _BOSS_NULL;
  msg_wait.p_tcb  = Boss_self();
  msg_wait.msg.m_cmd  = M_CMD_EMPTY;
  //msg_wait.msg.param  = 0;
  
  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(msg_q->fifo_count != 0)                                      /* 메시지가 있을때 */
  {    
    msg_wait.msg = Boss_msg_receive(msg_q);
  }
  else
  {
    /* 메시지 대기 리스트에 추가 */
    if(msg_q->wait_list != _BOSS_NULL) {
        msg_q->wait_list->prev  = &msg_wait;
        msg_wait.next = msg_q->wait_list;
    }
    msg_q->wait_list = &msg_wait;
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    if(WAIT_FOREVER == timeout) {
        _Boss_wait_sig_forever(SIG_BOSS_SUCCESS);                 /* 무한 대기 */
    } else {
        (void)_Boss_wait_sig_timeout(SIG_BOSS_SUCCESS, timeout);  /* 타임아웃 대기  */
    }

    BOSS_IRQ_DISABLE_SR(irq_storage);
    if( (Boss_self()->sigs & SIG_BOSS_SUCCESS) == 0 )               /* 타임아웃 */
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
