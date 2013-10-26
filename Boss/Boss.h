/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Kernel)                               *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss_Conf.h"

#ifndef _BOSS_KERNEL_H_
#define _BOSS_KERNEL_H_

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
#define _BOSS_NULL          ((void *) 0)

#define _BOSS_FALSE         ( 0)
#define _BOSS_TRUE          (!_BOSS_FALSE)

#define _BOSS_SUCCESS       ( 0)
#define _BOSS_FAILURE       (-1)    /* (32bit=0xFFFFFFFF) / (16bit=0xFFFF) */

#define NO_WAIT             ( (boss_tmr_ms_t)0) // 32bit = 0x00000000
#define WAIT_FOREVER        (~(boss_tmr_ms_t)0) // 32bit = 0xFFFFFFFF


#define BOSS_INDICATE_TIMEOUT       ((boss_u08_t)(0x80))    /* 타임아웃(슬립) */
#define BOSS_INDICATE_SUCCESS       ((boss_u08_t)(0x01))    /* 성공 / 완료    */
#define BOSS_INDICATE_CLEAR         ((boss_u08_t)(0x00))

/*---------------------------------------------------------------------------*/
/*                        TCB (Task Control Block)                           */
/*---------------------------------------------------------------------------*/
typedef struct boss_tcb_struct {      /* [ TCB (Task Control Block) ] */  
  struct boss_tcb_struct *sched_next;     /* Schedule list link */
                                          /* NULL : Waiting / !NULL : Scheduled */
  
  boss_prio_t       prio;                 /* Task Priority  */
                                          
  boss_u08_t        indicate;             /* Indicate       */

  #ifdef _BOSS_RR_QUANTUM_MS
  boss_u16_t        quantum_ms;           /* Round Robin    */
  #endif
  
  
  boss_stk_t        *sp;                  /* Task Stack Point */
  
  #ifdef _BOSS_TCB_NAME_SIZE
  char        name[_BOSS_TCB_NAME_SIZE];  /* TCB Name */
  #endif

  #ifdef _BOSS_TCB_EXTEND_
  _boss_tcb_ex_t          ex;             /* TCB Extend     */
  struct boss_tcb_struct *ex_task_link;   /* Task list link */
  #endif
} boss_tcb_t;

typedef boss_tcb_t *    BOSS_TID_T;       /* TASK ID TYPE (TCB Point) */

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
boss_tcb_t *Boss_init(int (*idle_task_entry)(void *p_arg), void *p_arg,
                                    boss_stk_t *p_stack, boss_uptr_t stk_bytes);

void        Boss_start(void);
boss_tcb_t *Boss_self(void);

void        Boss_sleep(boss_tmr_ms_t timeout);

boss_tcb_t *Boss_task_create(int (*task_entry)(void *p_arg), void *p_arg,
                                  boss_stk_t *p_stack, boss_uptr_t stk_bytes,
                                            boss_prio_t prio, const char *name);
void Boss_task_priority(boss_tcb_t *p_tcb, boss_prio_t new_prio);
void _Boss_task_exit(int exit_code);


#ifdef _BOSS_TCB_EXTEND_
unsigned int Boss_ex_task_count(void);
boss_tcb_t  *Boss_ex_task_list(unsigned int i);
#endif

void      _Boss_sched_lock(void);
void      _Boss_sched_free(void);
boss_reg_t Boss_sched_locking(void);

#endif  /* _BOSS_KERNEL_H_ */
