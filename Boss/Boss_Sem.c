/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                            RT-BOSS (Semaphore)                              *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss_Sem.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void          _Boss_schedule(void);
void          _Boss_setting_signal(boss_tcb_t *p_tcb, boss_sigs_t sigs);
void          _Boss_wait_sig_forever(boss_sigs_t wait_sigs);
boss_tmr_ms_t _Boss_wait_sig_timeout(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout);


/*===========================================================================
    B O S S _ S E M _ I N I T
---------------------------------------------------------------------------*/
void Boss_sem_init(boss_sem_t *p_sem, boss_reg_t sem_max)
{
  BOSS_ASSERT(0 < sem_max);
  
  p_sem->sem_count  = sem_max;
  p_sem->sem_max    = sem_max;

  p_sem->wait_list  = _BOSS_NULL;
}


/*===========================================================================
    B O S S _ S E M _ A C C E P T
---------------------------------------------------------------------------*/
boss_reg_t Boss_sem_accept(boss_sem_t *p_sem)
{  
  boss_reg_t  irq_storage;
  
  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(p_sem->sem_count != 0)
  {
    p_sem->sem_count--;                 /* 세마포어 할당 */
    BOSS_IRQ_RESTORE_SR(irq_storage);

    return _BOSS_SUCCESS;
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return (boss_reg_t)_BOSS_FAILURE;
}


/*===========================================================================
    B O S S _ S E M _ O B T A I N
---------------------------------------------------------------------------*/
boss_reg_t Boss_sem_obtain(boss_sem_t *p_sem, boss_tmr_ms_t timeout)
{
  _sem_wait_t sem_wait;
  boss_reg_t  irq_storage;
  
  Boss_sig_clear( Boss_self(), SIG_BOSS_SUCCESS );

  sem_wait.prev   = _BOSS_NULL;
  sem_wait.next   = _BOSS_NULL;
  sem_wait.p_tcb  = Boss_self();

  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(p_sem->sem_count != 0)
  {
    p_sem->sem_count--;
    BOSS_IRQ_RESTORE_SR(irq_storage);

    return _BOSS_SUCCESS;                                   /* 세마포어 획득 */
  }
  
  /* 세마포어 대기 리스트에 추가 */
  if( p_sem->wait_list != _BOSS_NULL ) {
      p_sem->wait_list->prev = &sem_wait;
      sem_wait.next = p_sem->wait_list;
  }
  p_sem->wait_list = &sem_wait;
  BOSS_IRQ_RESTORE_SR(irq_storage);

  if(WAIT_FOREVER == timeout) {
      _Boss_wait_sig_forever(SIG_BOSS_SUCCESS);                 /* 무한 대기 */
  } else {
      (void)_Boss_wait_sig_timeout(SIG_BOSS_SUCCESS, timeout);  /* 타임아웃 대기  */
  }
  
  BOSS_IRQ_DISABLE_SR(irq_storage);
  if( Boss_self()->sigs & SIG_BOSS_SUCCESS )
  {
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    return _BOSS_SUCCESS;                                   /* 세마포어 획득 */
  }

  /* 세마포어 대기 리스트에서 제거 ( 타임아웃 )*/
  if( sem_wait.prev == _BOSS_NULL ) {
      p_sem->wait_list    = sem_wait.next;
  } else {
      sem_wait.prev->next = sem_wait.next;
  }
  
  if( sem_wait.next != _BOSS_NULL ) {
      sem_wait.next->prev = sem_wait.prev;
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);

  return (boss_reg_t)_BOSS_FAILURE;               /* 세마포어 실패 (타임아웃) */
}


/*===========================================================================
    B O S S _ S E M _ R E L E A S E
---------------------------------------------------------------------------*/
void Boss_sem_release(boss_sem_t *p_sem)
{  
  boss_reg_t  irq_storage;
  
  BOSS_ASSERT(p_sem->sem_count < p_sem->sem_max);

  BOSS_IRQ_DISABLE_SR(irq_storage);
  if(p_sem->wait_list == _BOSS_NULL)
  {
    p_sem->sem_count++;
    BOSS_IRQ_RESTORE_SR(irq_storage);
    
    return;
  }
  else
  {
    _sem_wait_t *p_best = p_sem->wait_list;
    _sem_wait_t *p_find = p_best->next;

    while(p_find != _BOSS_NULL)         /* Find Best */
    {
      if( p_find->p_tcb->prio <= p_best->p_tcb->prio ) {
          p_best = p_find;
      }
      p_find = p_find->next;
    }
    
    /* 세마포어 대기 리스트에서 제거 (세마포어 전달) */
    if(p_best->prev == _BOSS_NULL) {
       p_sem->wait_list    = p_best->next;
    } else {
       p_best->prev->next  = p_best->next;
    }

    if(p_best->next != _BOSS_NULL) {
       p_best->next->prev = p_best->prev;
    }
    
    _Boss_setting_signal(p_best->p_tcb, SIG_BOSS_SUCCESS);
  }
  BOSS_IRQ_RESTORE_SR(irq_storage);

  _Boss_schedule();       /* 문맥전환 실행  */
}
