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

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
typedef struct _sem_link_struct {
  struct _sem_link_struct   *prev;      /* Semaphore link */
  struct _sem_link_struct   *next;
  
  boss_tcb_t                *p_tcb;
} _sem_link_t;


typedef struct {
  boss_u08_t        sem_count;
  boss_u08_t        sem_max;
  
  _sem_link_t       *wait_list;
} boss_sem_t;

typedef boss_sem_t *    BOSS_SEM_ID_T;    /* Semaphore ID TYPE */


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
#ifdef _BOSS_MEMORY_H_
BOSS_SEM_ID_T Boss_sem_create(boss_reg_t sem_max);

#define Boss_sem_del(sem_id)      do {                            \
                                      Boss_sem_mfree_del(sem_id); \
                                      sem_id = _BOSS_NULL;        \
                                    } while(0)

void Boss_sem_mfree_del(boss_sem_t *p_sem);
#endif /* _BOSS_MEMORY_H_ */


void        Boss_sem_init(boss_sem_t *p_sem, boss_reg_t sem_max);

boss_reg_t  Boss_sem_obtain(boss_sem_t *p_sem, boss_tmr_ms_t timeout);
void        Boss_sem_release(boss_sem_t *p_sem);

#endif  /* _BOSS_SEM_H_ */
