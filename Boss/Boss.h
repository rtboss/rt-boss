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

/*---------------------------------------------------------------------------*/
/*                         RT-BOSS 시그널 비트 정의                          */
/*                                                                           */
/*    시그널 데이터형이 32Bit일 경우 "BOSS_SIG_MSB_BIT" 는 "0x8000 0000"     */
/*    시그널 데이터형이 16bit일 경우 "BOSS_SIG_MSB_BIT" 는 "0x8000"          */
/*---------------------------------------------------------------------------*/
#define BOSS_SIG_MSB_BIT  (boss_sigs_t)((boss_sigs_t)1 << ((sizeof(boss_sigs_t)*8)-1))

#define BOSS_SIG_SLEEP            (boss_sigs_t)(BOSS_SIG_MSB_BIT >> 0)  /* 슬립     */
#define BOSS_SIG_SEM_OBTAIN       (boss_sigs_t)(BOSS_SIG_MSB_BIT >> 1)  /* 세마포어 */
#define BOSS_SIG_MBOX_PEND_DONE   (boss_sigs_t)(BOSS_SIG_MSB_BIT >> 2)  /* MBox 완료 */
// 예비(Reserve)    BOSS_SIG_RESERVE    (BOSS_SIG_MSB_BIT >> 3)   /* 예비     */

/*---------------------------------------------------------------------------*/
/*                        TCB (Task Control Block)                           */
/*---------------------------------------------------------------------------*/
typedef enum {
  _TCB_WAITING  =  _BOSS_FALSE,     /* 시그널 대기 상태 */
  _TCB_LISTING  =  _BOSS_TRUE,      /* 스케줄 리스팅    */
} _tcb_state_t;

typedef struct boss_tcb_struct {  /* [ TCB (Task Control Block) 구조체 ] */  
  struct boss_tcb_struct *next;           /* 스케줄러 리스트 링크 */
  _tcb_state_t      state;                /* 태스크 상태 (Waiting or Listing) */
  
  boss_prio_t       prio;                 /* 우선순위     */
  
  boss_sigs_t       sigs;                 /* 현재 시그널  */
  boss_sigs_t       wait;                 /* 대기 시그널  */
  
  boss_stk_t        *sp;                  /* 스택 포인터  */
  
  #ifdef _BOSS_SPY_
  boss_stk_t    *sp_base;
  boss_stk_t    *sp_peak;     
  boss_stk_t    *sp_limit;
  
  boss_u32_t    cpu_ent_us;   /* Task enter time        */
  boss_u32_t    cpu_sum_us;   /* Task run-time sum      */
  
  boss_uptr_t   context;      /* Context Switch Number  */
  #endif
  
  #ifdef _BOSS_TCB_NAME_SIZE
  char          name[_BOSS_TCB_NAME_SIZE];  /* TCB Name */
  #endif
} boss_tcb_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_init(void (*idle_task)(void *), boss_tcb_t *idle_tcb,
                                  boss_stk_t *sp_base, boss_uptr_t stk_bytes);

void Boss_start(void);

boss_tcb_t *Boss_self(void);

boss_sigs_t Boss_wait(boss_sigs_t wait_sigs);
void Boss_send(boss_tcb_t *p_tcb, boss_sigs_t sigs);
void Boss_sigs_clear(boss_tcb_t *p_tcb, boss_sigs_t sigs);
boss_sigs_t Boss_sigs_receive(void);

void Boss_task_create( void (*task)(void *p_arg), void *p_arg, 
                        boss_tcb_t *p_tcb, boss_prio_t prio, 
                        boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                        const char *name );


void Boss_task_delete(void);
void Boss_task_priority(boss_tcb_t *p_tcb, boss_prio_t new_prio);

void _Boss_sched_lock(void);
void _Boss_sched_free(void);
boss_reg_t Boss_sched_locking(void);

#endif  /* _BOSS_KERNEL_H_ */
