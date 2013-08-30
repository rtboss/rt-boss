#ifndef _BOSS_SEM_H_
#define _BOSS_SEM_H_
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
#include "Boss.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
typedef struct _sem_link_struct {
  boss_tcb_t                *p_tcb;
  struct _sem_link_struct   *next;
} _sem_link_t;


typedef struct {
  boss_reg_t        busy;           /* 0 = free / 0 != busy */
  
  boss_tcb_t        *owner_tcb;
  _sem_link_t       *wait_list;
} boss_sem_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_sem_init(boss_sem_t *p_sem);
boss_reg_t Boss_sem_accept(boss_sem_t *p_sem);
boss_reg_t Boss_sem_obtain(boss_sem_t *p_sem, boss_tmr_ms_t timeout);
void Boss_sem_release(boss_sem_t *p_sem);

#endif  /* _BOSS_SEM_H_ */
