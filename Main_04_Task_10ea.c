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
    [ C x _ T A S K ]
---------------------------------------------------------------------------*/
#define CX_TASK_MAX   10

boss_tcb_t    cx_tcb[CX_TASK_MAX];
boss_stk_t    cx_stk[CX_TASK_MAX][ 256 / sizeof(boss_stk_t) ];   /* 256 bytes */

int cx_task(void *p_arg)
{
  int cx_count = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  for(;;)
  {
    Boss_sleep(1000);
    
    PRINTF("[%s TASK] cx_count = %d\n", Boss_self()->name, ++cx_count);

    if(cx_count == 100)
    {
      break;
    }
  }
  
  PRINTF("[%s TASK ] Delete \n", Boss_self()->name);
  return 0;   // Task Exit
}


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
  int idx = 0;
  PRINTF("[AA_TASK] Init \n");
  
  for(idx=0; idx < CX_TASK_MAX; idx++)
  {
    char name[10];
    sprintf(name, "C%02d", idx);    
    Boss_task_create( cx_task,              /* Task Entry Point       */
                    (void *)idx,            /* Task Argument          */
                    &cx_tcb[idx],           /* TCB(Task Control Block)*/
                    Cx_PRIO_3,              /* Priority               */
                    cx_stk[idx],            /* Stack Point (Base)     */
                    sizeof(cx_stk[idx]),    /* Stack Size (Bytes)     */
                    name                    /* Task Name String       */
                    );
  }


  for(;;)
  {    
    Boss_sleep(10 * 1000);   /* 10 Sec */

    #ifdef _BOSS_SPY_
    Boss_spy_report();
    #endif
    
    #ifdef _BOSS_MEM_INFO_
    Boss_mem_info_report();
    #endif

  }
  
  PRINTF("[AA_TASK] Exit \n");
  
  return 0;       // 테스크 종료
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
주의 : 테스크 10생성시 3Kbyte 정도의 메모리를 사용함.

설명 : main()함수에서 AA테스크를 생성하고 AA테스크에서 10개의 테스크를 생성 한후
       10초마다 SPY정보를 출력한다.
       AA테스크에서 생성된 Cx테스크들은 1초마다 메시지를 출력.
       

### 실행결과 ###

  [AA_TASK] Init 
   [C00 TASK] Init 
   [C01 TASK] Init 
   [C02 TASK] Init 
   [C03 TASK] Init 
   [C04 TASK] Init 
   [C05 TASK] Init 
   [C06 TASK] Init 
   [C07 TASK] Init 
   [C08 TASK] Init 
   [C09 TASK] Init 
   [C00 TASK] cx_count = 1
   [C01 TASK] cx_count = 1
   [C02 TASK] cx_count = 1
   [C03 TASK] cx_count = 1
   [C04 TASK] cx_count = 1
   [C05 TASK] cx_count = 1
   [C06 TASK] cx_count = 1
   [C07 TASK] cx_count = 1
   [C08 TASK] cx_count = 1
   [C09 TASK] cx_count = 1
   [C00 TASK] cx_count = 2
   [C01 TASK] cx_count = 2
   [C02 TASK] cx_count = 2
   [C03 TASK] cx_count = 2
   [C04 TASK] cx_count = 2
   [C05 TASK] cx_count = 2
   [C06 TASK] cx_count = 2
   [C07 TASK] cx_count = 2
   [C08 TASK] cx_count = 2
   [C09 TASK] cx_count = 2
   [C00 TASK] cx_count = 3
   [C01 TASK] cx_count = 3
   [C02 TASK] cx_count = 3
   [C03 TASK] cx_count = 3
   [C04 TASK] cx_count = 3
   [C05 TASK] cx_count = 3
   [C06 TASK] cx_count = 3
   [C07 TASK] cx_count = 3
   [C08 TASK] cx_count = 3
   [C09 TASK] cx_count = 3
   [C00 TASK] cx_count = 4
   [C01 TASK] cx_count = 4
   [C02 TASK] cx_count = 4
   [C03 TASK] cx_count = 4
   [C04 TASK] cx_count = 4
   [C05 TASK] cx_count = 4
   [C06 TASK] cx_count = 4
   [C07 TASK] cx_count = 4
   [C08 TASK] cx_count = 4
   [C09 TASK] cx_count = 4
   [C00 TASK] cx_count = 5
   [C01 TASK] cx_count = 5
   [C02 TASK] cx_count = 5
   [C03 TASK] cx_count = 5
   [C04 TASK] cx_count = 5
   [C05 TASK] cx_count = 5
   [C06 TASK] cx_count = 5
   [C07 TASK] cx_count = 5
   [C08 TASK] cx_count = 5
   [C09 TASK] cx_count = 5
   [C00 TASK] cx_count = 6
   [C01 TASK] cx_count = 6
   [C02 TASK] cx_count = 6
   [C03 TASK] cx_count = 6
   [C04 TASK] cx_count = 6
   [C05 TASK] cx_count = 6
   [C06 TASK] cx_count = 6
   [C07 TASK] cx_count = 6
   [C08 TASK] cx_count = 6
   [C09 TASK] cx_count = 6
   [C00 TASK] cx_count = 7
   [C01 TASK] cx_count = 7
   [C02 TASK] cx_count = 7
   [C03 TASK] cx_count = 7
   [C04 TASK] cx_count = 7
   [C05 TASK] cx_count = 7
   [C06 TASK] cx_count = 7
   [C07 TASK] cx_count = 7
   [C08 TASK] cx_count = 7
   [C09 TASK] cx_count = 7
   [C00 TASK] cx_count = 8
   [C01 TASK] cx_count = 8
   [C02 TASK] cx_count = 8
   [C03 TASK] cx_count = 8
   [C04 TASK] cx_count = 8
   [C05 TASK] cx_count = 8
   [C06 TASK] cx_count = 8
   [C07 TASK] cx_count = 8
   [C08 TASK] cx_count = 8
   [C09 TASK] cx_count = 8
   [C00 TASK] cx_count = 9
   [C01 TASK] cx_count = 9
   [C02 TASK] cx_count = 9
   [C03 TASK] cx_count = 9
   [C04 TASK] cx_count = 9
   [C05 TASK] cx_count = 9
   [C06 TASK] cx_count = 9
   [C07 TASK] cx_count = 9
   [C08 TASK] cx_count = 9
   [C09 TASK] cx_count = 9
   
   [ M S P ] %(u/t) :  10% (112/1024)
   
   [TASK]   STACK %(u/t)    C P U    Context
   ------------------------------------------
     C09    75%(192/256)    0.026%        10
     C08    65%(168/256)    0.026%        10
     C07    75%(192/256)    0.026%        10
     C06    65%(168/256)    0.026%        10
     C05    68%(176/256)    0.026%        10
     C04    75%(192/256)    0.027%        10
     C03    65%(168/256)    0.026%        10
     C02    65%(168/256)    0.026%        10
     C01    68%(176/256)    0.026%        10
     C00    65%(168/256)    0.026%        10
      AA    35%(184/512)    0.016%         1
    Idle    56%( 72/128)   99.716%        35
   [TOTAL] :               99.993%       136
   
      total_us = 10001087
      SysTick->LOAD = 11999
   
   [Mmory]  Peak byte  Used byte  Total  Block  first
   [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
   
   [C00 TASK] cx_count = 10
   [C01 TASK] cx_count = 10
   [C02 TASK] cx_count = 10
   [C03 TASK] cx_count = 10
   [C04 TASK] cx_count = 10
   [C05 TASK] cx_count = 10
   [C06 TASK] cx_count = 10
   [C07 TASK] cx_count = 10
   [C08 TASK] cx_count = 10
   [C09 TASK] cx_count = 10
  
   ------ 중략 ---------
  
   [C00 TASK] cx_count = 98
   [C01 TASK] cx_count = 98
   [C02 TASK] cx_count = 98
   [C03 TASK] cx_count = 98
   [C04 TASK] cx_count = 98
   [C05 TASK] cx_count = 98
   [C06 TASK] cx_count = 98
   [C07 TASK] cx_count = 98
   [C08 TASK] cx_count = 98
   [C09 TASK] cx_count = 98
   [C00 TASK] cx_count = 99
   [C01 TASK] cx_count = 99
   [C02 TASK] cx_count = 99
   [C03 TASK] cx_count = 99
   [C04 TASK] cx_count = 99
   [C05 TASK] cx_count = 99
   [C06 TASK] cx_count = 99
   [C07 TASK] cx_count = 99
   [C08 TASK] cx_count = 99
   [C09 TASK] cx_count = 99
   
   [ M S P ] %(u/t) :  10% (112/1024)
   
   [TASK]   STACK %(u/t)    C P U    Context
   ------------------------------------------
     C09    75%(192/256)    0.030%       100
     C08    68%(176/256)    0.030%       100
     C07    75%(192/256)    0.030%       100
     C06    78%(200/256)    0.030%       100
     C05    68%(176/256)    0.035%       100
     C04    75%(192/256)    0.029%       100
     C03    75%(192/256)    0.029%       100
     C02    65%(168/256)    0.037%       100
     C01    68%(176/256)    0.029%       100
     C00    75%(192/256)    0.029%       100
      AA    50%(256/512)    0.135%        11
    Idle    56%( 72/128)   99.549%       189
   [TOTAL] :               99.992%      1200
   
      total_us = 100130130
      SysTick->LOAD = 11999
   
   [Mmory]  Peak byte  Used byte  Total  Block  first
   [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
   
   [C00 TASK] cx_count = 100
   [C00 TASK ] Delete 
   [C01 TASK] cx_count = 100
   [C01 TASK ] Delete 
   [C02 TASK] cx_count = 100
   [C02 TASK ] Delete 
   [C03 TASK] cx_count = 100
   [C03 TASK ] Delete 
   [C04 TASK] cx_count = 100
   [C04 TASK ] Delete 
   [C05 TASK] cx_count = 100
   [C05 TASK ] Delete 
   [C06 TASK] cx_count = 100
   [C06 TASK ] Delete 
   [C07 TASK] cx_count = 100
   [C07 TASK ] Delete 
   [C08 TASK] cx_count = 100
   [C08 TASK ] Delete 
   [C09 TASK] cx_count = 100
   [C09 TASK ] Delete 

*/
