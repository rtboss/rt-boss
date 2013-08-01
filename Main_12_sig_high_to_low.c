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
extern boss_tcb_t    bb_tcb;

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
    Boss_sleep(3 * 1000);
    PRINTF("\n%d AA sleep 후\n", ++aa_count);

    PRINTF("  [AA] BB SIG_01_BIT send\n");
    Boss_send(&bb_tcb, SIG_01_BIT);

    PRINTF("    [AA] BB SIG_02_BIT send\n");
    Boss_send(&bb_tcb, SIG_02_BIT);

    PRINTF("      [AA] BB SIG_03_BIT send\n");
    Boss_send(&bb_tcb, SIG_03_BIT);

    if(10 <= aa_count)
    {
      break;
    }
  }
  Boss_send(&bb_tcb, SIG_00_BIT);
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
    boss_sigs_t sigs = Boss_wait( SIG_00_BIT  // 종료 시그널
                                | SIG_01_BIT
                                | SIG_02_BIT
                                | SIG_03_BIT
                                );

    if(sigs & SIG_01_BIT)
    {
      PRINTF(" (BB) SIG_01_BIT receive\n");
    }
    
    if(sigs & SIG_02_BIT)
    {
      PRINTF("   (BB) SIG_02_BIT receive\n");
    }
    
    if(sigs & SIG_03_BIT)
    {
      PRINTF("     (BB) SIG_03_BIT receive\n");
    }

    if(sigs & SIG_00_BIT)
    {
      PRINTF("\n(BB) SIG_00_BIT (Exit) receive\n");
      break;
    }
  }

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
설명 : 우선순위가 높은 AA테스크에서 BB테스크로 시그널(Signal) 통신 예제.
     - 우선순위가 높은 AA 테스크는 3초마다 BB 테스크로 3개의 시그널을 전송함.

     - BB 테스크는 "SIG_01_BIT, SIG_02_BIT, SIG_03_BIT" 시그널을 받으면
       메시지를 출력하고 "SIG_00_BIT"시그널을 받으면 종료함.
       

### 실행결과 ###

  [AA TASK] Init 
  [BB TASK] Init 
  
  1 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  2 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  3 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  4 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  5 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  6 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  7 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  8 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  9 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  10 AA sleep 후
    [AA] BB SIG_01_BIT send
      [AA] BB SIG_02_BIT send
        [AA] BB SIG_03_BIT send
  [AA TASK] Exit 
   (BB) SIG_01_BIT receive
     (BB) SIG_02_BIT receive
       (BB) SIG_03_BIT receive
  
  (BB) SIG_00_BIT (Exit) receive
  [BB TASK] Exit 
    
*/

