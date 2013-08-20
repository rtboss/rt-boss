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
#include "Boss_Q_MBox.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/


/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
static void _Boss_mbox_insert(boss_mbox_q_t *mbox_q, _mbox_head_t  *h_mbox);
static void _Boss_mbox_remove(boss_mbox_q_t *mbox_q, _mbox_head_t *h_mbox);


/*===========================================================================
    B O S S _ M B O X _ Q _ I N I T
---------------------------------------------------------------------------*/
void Boss_mbox_q_init(boss_mbox_q_t *mbox_q, boss_tcb_t *p_tcb, boss_sigs_t sig)
{
  BOSS_ASSERT(mbox_q->owner_tcb == _BOSS_NULL);
  BOSS_ASSERT(mbox_q->mbox_fifo == _BOSS_NULL);
  
  mbox_q->owner_tcb = p_tcb;
  mbox_q->mbox_sig  = sig;
  mbox_q->mbox_fifo = _BOSS_NULL;
}


/*===========================================================================
    B O S S _ M B O X _ A L L O C
---------------------------------------------------------------------------*/
void *Boss_mbox_alloc(boss_uptr_t size)
{
  _mbox_head_t  *h_mbox;

  BOSS_ASSERT(0 < size);
  
  h_mbox = Boss_malloc(sizeof(_mbox_head_t) + size);

  h_mbox->state   = _MBOX_INVALID;
  h_mbox->sender  = _BOSS_NULL;
  h_mbox->p_rsp   = _BOSS_NULL;
  h_mbox->next    = _BOSS_NULL;
  
  return  (h_mbox + 1);  // mbox : ( (void *)h_mbox + sizeof(_mbox_head_t) )
}


/*===========================================================================
    B O S S _ M B O X _ F R E E
---------------------------------------------------------------------------*/
void Boss_mbox_free(void *p_mbox)
{
  _mbox_head_t  *h_mbox = ((_mbox_head_t *)p_mbox) - 1;
                        // h_mbox = ( (void *)mbox - sizeof(_mbox_head_t) )
  Boss_mfree(h_mbox);
}


/*===========================================================================
    B O S S _ M B O X _ P E N D
---------------------------------------------------------------------------*/
boss_reg_t Boss_mbox_pend(boss_mbox_q_t *mbox_q, void *p_mbox,
                                    boss_uptr_t *p_rsp, boss_tmr_ms_t timeout)
{
  _mbox_head_t  *h_mbox = ((_mbox_head_t *)p_mbox) - 1;  
  boss_reg_t    irq_storage;
  boss_sigs_t   sigs;

  BOSS_ASSERT(mbox_q->owner_tcb != _BOSS_NULL);
  BOSS_ASSERT(mbox_q->owner_tcb != Boss_self());
  BOSS_ASSERT(p_rsp != _BOSS_NULL);                   /* MBox Pend */
  
  h_mbox->p_rsp = p_rsp;
  h_mbox->sender = Boss_self();
  Boss_sigs_clear(Boss_self(), BOSS_SIG_MBOX_PEND_DONE);
  _Boss_mbox_insert(mbox_q, h_mbox);
  Boss_send(mbox_q->owner_tcb, mbox_q->mbox_sig);

  if( timeout == 0 )                          /* 대기 (타임아웃 사용안함) */
  {
    (void)Boss_wait(BOSS_SIG_MBOX_PEND_DONE);
    
    return _BOSS_SUCCESS;
  }
                                                  /* 대기 (타임아웃 사용) */
  sigs = Boss_wait_sleep(BOSS_SIG_MBOX_PEND_DONE, timeout);

  BOSS_IRQ_DISABLE_SR(irq_storage);
  sigs = sigs | Boss_sigs_receive(BOSS_SIG_MBOX_PEND_DONE);
  if(sigs & BOSS_SIG_MBOX_PEND_DONE)  /* 처리 완료 */
  {
    BOSS_IRQ_RESTORE_SR(irq_storage);
    return _BOSS_SUCCESS;
  }

  BOSS_ASSERT(sigs == 0);              /* 타임아웃 */
  if(h_mbox->state == _MBOX_EXECUTE)    /* 처리 중 완료 까지 대기  */
  {
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    Boss_wait(BOSS_SIG_MBOX_PEND_DONE);
    
    return _BOSS_SUCCESS;
  }
  
  BOSS_ASSERT(h_mbox->state == _MBOX_READY);
  
  _Boss_mbox_remove(mbox_q, h_mbox);
  Boss_mfree(h_mbox);
  
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return (boss_reg_t)_BOSS_FAILURE;
}


/*===========================================================================
    B O S S _ M B O X _ R E C E I V E
---------------------------------------------------------------------------*/
void *Boss_mbox_receive(boss_mbox_q_t *mail_q)
{
  void *p_mbox = _BOSS_NULL;
  
  BOSS_ASSERT(mail_q->owner_tcb == Boss_self());

  if(mail_q->mbox_fifo != _BOSS_NULL)
  {
    _mbox_head_t *h_mbox;
    
    BOSS_IRQ_DISABLE();
    h_mbox = mail_q->mbox_fifo;
    mail_q->mbox_fifo = h_mbox->next;

    BOSS_ASSERT(h_mbox->state == _MBOX_READY);
    h_mbox->state = _MBOX_EXECUTE;
    BOSS_IRQ_RESTORE();

    p_mbox = (h_mbox + 1);
  }
  
  return p_mbox;
}


/*===========================================================================
    B O S S _ M B O X _ D O N E
---------------------------------------------------------------------------*/
void Boss_mbox_pend_done(void *p_mbox, boss_uptr_t rsp)
{
  _mbox_head_t  *h_mbox = ((_mbox_head_t *)p_mbox) - 1;

  BOSS_ASSERT(h_mbox->state == _MBOX_EXECUTE);

  _Boss_sched_lock();
  if(h_mbox->p_rsp != _BOSS_NULL)
  {
    *(h_mbox->p_rsp) = rsp;
    Boss_send(h_mbox->sender, BOSS_SIG_MBOX_PEND_DONE);
  }
  _Boss_sched_free();
}


/*===========================================================================
    _   B O S S _ M B O X _ I N S E R T
---------------------------------------------------------------------------*/
static void _Boss_mbox_insert(boss_mbox_q_t *mbox_q, _mbox_head_t  *h_mbox)
{
  BOSS_ASSERT(h_mbox->state == _MBOX_INVALID);
  BOSS_ASSERT(h_mbox->next == _BOSS_NULL);

  h_mbox->state = _MBOX_READY;
  
  BOSS_IRQ_DISABLE();
  if(mbox_q->mbox_fifo == _BOSS_NULL)
  {
    mbox_q->mbox_fifo = h_mbox;
  }
  else
  {
    _mbox_head_t *p_tail = mbox_q->mbox_fifo;
    
    while(p_tail->next != _BOSS_NULL)
    {
      p_tail = p_tail->next;
    }
    p_tail->next = h_mbox;            /* 큐 추가 */
  }
  BOSS_IRQ_RESTORE();
}


/*===========================================================================
    _   B O S S _ M B O X _ R E M O V E
---------------------------------------------------------------------------*/
static void _Boss_mbox_remove(boss_mbox_q_t *mbox_q, _mbox_head_t *h_mbox)
{
  if(mbox_q->mbox_fifo == h_mbox)
  {
    mbox_q->mbox_fifo = h_mbox->next;
  }
  else
  {
    _mbox_head_t *h_find = mbox_q->mbox_fifo;
    while(h_find->next != h_mbox)
    {
      h_find = h_find->next;
      BOSS_ASSERT(h_find != _BOSS_NULL);
    }

    h_find->next = h_mbox->next;
  }
}

