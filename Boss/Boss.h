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

#define WAIT_FOREVER        ((boss_tmr_ms_t)0)

/*---------------------------------------------------------------------------*/
/*                         RT-BOSS 시그널 비트 정의                          */
/*                                                                           */
/*    시그널 데이터형이 32Bit일 경우 "SIG_BOSS_MSB_BIT" 는 "0x8000 0000"     */
/*    시그널 데이터형이 16bit일 경우 "SIG_BOSS_MSB_BIT" 는 "0x8000"          */
/*---------------------------------------------------------------------------*/
#define SIG_BOSS_MSB_BIT  (boss_sigs_t)((boss_sigs_t)1 << ((sizeof(boss_sigs_t)*8)-1))

#define SIG_BOSS_TIMEOUT      (boss_sigs_t)(SIG_BOSS_MSB_BIT >> 0)  /* 타임아웃(슬립) */
#define SIG_BOSS_SUCCESS      (boss_sigs_t)(SIG_BOSS_MSB_BIT >> 1)  /* 완료 (성공)    */
//#define SIG_BOSS_RESERVE_MSB2   (boss_sigs_t)(SIG_BOSS_MSB_BIT >> 2)  /* 예비(2)    */
//#define SIG_BOSS_RESERVE_MSB3   (boss_sigs_t)(SIG_BOSS_MSB_BIT >> 3)  /* 예비(3)    */

/*---------------------------------------------------------------------------*/
/*                        TCB (Task Control Block)                           */
/*---------------------------------------------------------------------------*/
typedef struct boss_tcb_struct {      /* [ TCB (Task Control Block) ] */  
  struct boss_tcb_struct *run_next;       /* Schedule list link */
                                          /* NULL : Waiting / !NULL : Scheduled */
  
  boss_prio_t       prio;                 /* Task Priority  */

  #ifdef _BOSS_RR_QUANTUM_MS
  boss_u16_t        quantum_ms;           /* Round Robin    */
  #endif
  
  boss_sigs_t       sigs;                 /* Current Signal */
  boss_sigs_t       wait;                 /* Waiting Signal */
  
  boss_stk_t        *sp;                  /* Task Stack Point */
  
  #ifdef _BOSS_TCB_NAME_SIZE
  char        name[_BOSS_TCB_NAME_SIZE];  /* TCB Name */
  #endif

  #ifdef _BOSS_TCB_EXTEND_
  _boss_tcb_ex_t          ex;             /* TCB Extend     */
  struct boss_tcb_struct *ex_task_link;   /* Task list link */
  #endif
} boss_tcb_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void        Boss_init(int (*idle_task)(void *), boss_tcb_t *idle_tcb,
                                  boss_stk_t *sp_base, boss_uptr_t stk_bytes);
void        Boss_start(void);
boss_tcb_t *Boss_self(void);

void        Boss_sleep(boss_tmr_ms_t wait_ms);

void        Boss_sig_send(boss_tcb_t *p_tcb, boss_sigs_t sigs);
void        Boss_sig_clear(boss_tcb_t *p_tcb, boss_sigs_t sigs);
boss_sigs_t Boss_sig_receive(boss_sigs_t wait_sigs);
boss_sigs_t Boss_sig_wait(boss_sigs_t wait_sigs, boss_tmr_ms_t timeout);

void Boss_task_create( int (*task)(void *p_arg), void *p_arg, 
                        boss_tcb_t *p_tcb, boss_prio_t prio, 
                        boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                        const char *name );
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
