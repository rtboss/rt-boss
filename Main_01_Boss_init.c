/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                                M  A  I  N                                   *
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

/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_device_init(void);

/*===========================================================================
    [ A A _ T A S K ]
---------------------------------------------------------------------------*/
boss_tcb_t    aa_tcb;
boss_align_t  aa_stk[ 512 / sizeof(boss_align_t) ];         /* 512 bytes */

/*===============================================
    A A _ M A I N
-----------------------------------------------*/
int aa_main(void *p_arg)
{  
  int aa_count = 0;
  
  PRINTF("[%s TASK] 시작 \n", Boss_self()->name);
  
  for(;;)
  {    
    Boss_sleep(1000);  /* 1000ms (1초) */
    
    PRINTF(" AA_TASK count = %d \n", ++aa_count);
  }
  
  return 0;
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                         RT-BOSS ( IDLE TASK )                               *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
boss_tcb_t    idle_tcb;
boss_align_t  idle_stack[ 128 / sizeof(boss_align_t) ];     /* 128 bytes */

/*===========================================================================
    I D L E _ M A I N
---------------------------------------------------------------------------*/
int idle_main(void *p_arg)
{
  for(;;)
  {
  }
}


/*===========================================================================
    M A I N
---------------------------------------------------------------------------*/
int main(void)
{
  Boss_init(idle_main, &idle_tcb, (boss_stk_t *)idle_stack, sizeof(idle_stack));
  
  Boss_task_create( aa_main,              /* Task Entry Point       */
                    _BOSS_NULL,           /* Task Argument          */
                    &aa_tcb,              /* TCB(Task Control Block)*/
                    AA_PRIO_1,            /* 우선순위               */
                    (boss_stk_t *)aa_stk, /* 스택 포인터(base)      */
                    sizeof(aa_stk),       /* 스택 크기(Bytes)       */
                    "AA"                  /* 테스크 이름            */
                    );

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
요약 : 테스크(AA_Task) 1개를 생성하여 1초마다 메시지를 출력함.

설명 : RT-BOSS의 초기 실행 과정. 
      1. Boss_init()        : Idle_Task 생성 및 초기화.
      2. Boss_task_create() : 1개이상의 테스크 생성.
      3. Boss_device_init() : Timer 및 CPU 별 설정.
      4. Boss_start()       : 스케줄러 시작.


### 실행결과 ###

  [AA TASK] 시작 
    AA_TASK count = 1 
    AA_TASK count = 2 
    AA_TASK count = 3 
    AA_TASK count = 4 
    AA_TASK count = 5 
    AA_TASK count = 6 
    AA_TASK count = 7 
    AA_TASK count = 8 
    AA_TASK count = 9 
    AA_TASK count = 10 
    AA_TASK count = 11 
    AA_TASK count = 12 
    AA_TASK count = 13 
    AA_TASK count = 14 
    ...
    
*/
