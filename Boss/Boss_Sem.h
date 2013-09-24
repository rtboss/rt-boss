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
typedef struct _sem_wait_struct {
  struct _sem_wait_struct   *prev;      /* Semaphore wait list link */
  struct _sem_wait_struct   *next;
  
  boss_tcb_t                *p_tcb;
} _sem_wait_t;


typedef struct {
  boss_u08_t        sem_count;
  boss_u08_t        sem_max;
  
  _sem_wait_t       *wait_list;
} boss_sem_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void        Boss_sem_init(boss_sem_t *p_sem, boss_reg_t sem_max);
boss_reg_t  Boss_sem_accept(boss_sem_t *p_sem);
boss_reg_t  Boss_sem_obtain(boss_sem_t *p_sem, boss_tmr_ms_t timeout);
void        Boss_sem_release(boss_sem_t *p_sem);

#endif  /* _BOSS_SEM_H_ */
