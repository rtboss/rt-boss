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
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(;;)
  {
    boss_sigs_t sigs = Boss_wait( SIG_00_BIT  // 종료 시그널
                                | SIG_01_BIT
                                | SIG_02_BIT
                                | SIG_03_BIT
                                );

    if(sigs & SIG_01_BIT)
    {
      PRINTF(" [AA] SIG_01_BIT receive\n");
    }
    
    if(sigs & SIG_02_BIT)
    {
      PRINTF("   [AA] SIG_02_BIT receive\n");
    }
    
    if(sigs & SIG_03_BIT)
    {
      PRINTF("     [AA] SIG_03_BIT receive\n");
    }

    if(sigs & SIG_00_BIT)
    {
      PRINTF("[AA] SIG_00_BIT (Exit) receive\n");
      break;
    }
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
  return 0;
}


/*===========================================================================
    [ B B _ T A S K ]
---------------------------------------------------------------------------*/
boss_tcb_t    bb_tcb;
boss_align_t  bb_stk[ 512 / sizeof(boss_align_t) ];         /* 512 bytes */

/*===============================================
    B B _ M A I N
-----------------------------------------------*/
int bb_main(void *p_arg)
{
  int bb_count = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(;;)
  {
    Boss_sleep(3 * 1000);

    ++bb_count;

    PRINTF("\n%d (BB) AA(SIG_01_BIT) send\n", bb_count);
    Boss_send(&aa_tcb, SIG_01_BIT);

    PRINTF("  (BB) AA(SIG_02_BIT) send\n");
    Boss_send(&aa_tcb, SIG_02_BIT);

    PRINTF("    (BB) AA(SIG_03_BIT) send\n");
    Boss_send(&aa_tcb, SIG_03_BIT);
    
    if(10 <= bb_count)
    {
      break;
    }
  }

  Boss_sleep(3 * 1000);

  PRINTF("\n(BB) AA(SIG_00_BIT) send\n");
  Boss_send(&aa_tcb, SIG_00_BIT);

  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
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
                    AA_PRIO_1,            /* Priority               */
                    (boss_stk_t *)aa_stk, /* Stack Point (Base)     */
                    sizeof(aa_stk),       /* Stack Size (Bytes)     */
                    "AA"
                    );
  
  Boss_task_create( bb_main, _BOSS_NULL, &bb_tcb, BB_PRIO_2,
                    (boss_stk_t *)bb_stk, sizeof(bb_stk), "BB" );

  Boss_device_init();
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
설명 : 테스크 4개의 시그널(Signal) 통신 예제. 
     - AA 테스크는 "SIG_01_BIT, SIG_02_BIT, SIG_03_BIT" 시그널을 받으면
       메시지를 출력하고 "SIG_00_BIT"시그널을 받으면 종료함.
     - BB 테스크는 3초마다 AA 테스크로 3개의 시그널을 전송함.


### 실행결과 ###

  [AA TASK] Init 
  [BB TASK] Init 
  
  1 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  2 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  3 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  4 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  5 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  6 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  7 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  8 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  9 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  10 (BB) AA(SIG_01_BIT) send
   [AA] SIG_01_BIT receive
    (BB) AA(SIG_02_BIT) send
     [AA] SIG_02_BIT receive
      (BB) AA(SIG_03_BIT) send
       [AA] SIG_03_BIT receive
  
  (BB) AA(SIG_00_BIT) send
  [AA] SIG_00_BIT (Exit) receive
  [AA TASK] Exit 
  [BB TASK] Exit 

    
*/

