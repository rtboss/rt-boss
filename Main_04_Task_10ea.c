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

boss_stk_t cx_stk[CX_TASK_MAX][ 512 / sizeof(boss_stk_t) ];

int cx_task(void *p_arg)
{
  int cx_count = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  Boss_sleep(10);

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
boss_stk_t  aa_stk[ 512 / sizeof(boss_stk_t) ];

int aa_task(void *p_arg)
{  
  int idx = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(idx=0; idx < CX_TASK_MAX; idx++)
  {
    char name[10];
    sprintf(name, "C%02d", idx+1);
    (void)Boss_task_create( cx_task,              /* Task Entry Point       */
                            (void *)idx,          /* Task Argument          */
                            cx_stk[idx],          /* Stack Point (Base)     */
                            sizeof(cx_stk[idx]),  /* Stack Size (Bytes)     */
                            PRIO_3,               /* Priority               */
                            name                  /* Task Name String       */
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
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
  return 0;       // 테스크 종료
}


/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                         RT-BOSS ( IDLE TASK )                               *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
boss_stk_t idle_stack[ 160 / sizeof(boss_stk_t)];

int idle_task(void *p_arg)
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
  (void)Boss_init(idle_task, _BOSS_NULL, idle_stack, sizeof(idle_stack));
  
  (void)Boss_task_create( aa_task,              /* Task Entry Point       */
                          _BOSS_NULL,           /* Task Argument          */
                          aa_stk,               /* 스택 포인터(base)      */
                          sizeof(aa_stk),       /* 스택 크기(Bytes)       */
                          PRIO_1,               /* 우선순위               */
                          "AA"                  /* 테스크 이름            */
                        );

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*

설명 : main()함수에서 AA테스크를 생성하고 AA테스크에서 10개의 테스크를 생성 한후
       10초마다 SPY정보를 출력한다.
       AA테스크에서 생성된 Cx테스크들은 1초마다 메시지를 출력.
       

    ########## 실행 결과 ##########

      [AA TASK] Init 
      [C01 TASK] Init 
      [C02 TASK] Init 
      [C03 TASK] Init 
      [C04 TASK] Init 
      [C05 TASK] Init 
      [C06 TASK] Init 
      [C07 TASK] Init 
      [C08 TASK] Init 
      [C09 TASK] Init 
      [C10 TASK] Init 
      [C01 TASK] cx_count = 1
      [C02 TASK] cx_count = 1
      [C03 TASK] cx_count = 1
      [C04 TASK] cx_count = 1
      [C05 TASK] cx_count = 1
      [C06 TASK] cx_count = 1
      [C07 TASK] cx_count = 1
      [C08 TASK] cx_count = 1
      [C09 TASK] cx_count = 1
      [C10 TASK] cx_count = 1
      [C01 TASK] cx_count = 2
      [C02 TASK] cx_count = 2
      [C03 TASK] cx_count = 2
      [C04 TASK] cx_count = 2
      [C05 TASK] cx_count = 2
      [C06 TASK] cx_count = 2
      [C07 TASK] cx_count = 2
      [C08 TASK] cx_count = 2
      [C09 TASK] cx_count = 2
      [C10 TASK] cx_count = 2
      [C01 TASK] cx_count = 3
      [C02 TASK] cx_count = 3
      [C03 TASK] cx_count = 3
      [C04 TASK] cx_count = 3
      [C05 TASK] cx_count = 3
      [C06 TASK] cx_count = 3
      [C07 TASK] cx_count = 3
      [C08 TASK] cx_count = 3
      [C09 TASK] cx_count = 3
      [C10 TASK] cx_count = 3
      [C01 TASK] cx_count = 4
      [C02 TASK] cx_count = 4
      [C03 TASK] cx_count = 4
      [C04 TASK] cx_count = 4
      [C05 TASK] cx_count = 4
      [C06 TASK] cx_count = 4
      [C07 TASK] cx_count = 4
      [C08 TASK] cx_count = 4
      [C09 TASK] cx_count = 4
      [C10 TASK] cx_count = 4
      [C01 TASK] cx_count = 5
      [C02 TASK] cx_count = 5
      [C03 TASK] cx_count = 5
      [C04 TASK] cx_count = 5
      [C05 TASK] cx_count = 5
      [C06 TASK] cx_count = 5
      [C07 TASK] cx_count = 5
      [C08 TASK] cx_count = 5
      [C09 TASK] cx_count = 5
      [C10 TASK] cx_count = 5
      [C01 TASK] cx_count = 6
      [C02 TASK] cx_count = 6
      [C03 TASK] cx_count = 6
      [C04 TASK] cx_count = 6
      [C05 TASK] cx_count = 6
      [C06 TASK] cx_count = 6
      [C07 TASK] cx_count = 6
      [C08 TASK] cx_count = 6
      [C09 TASK] cx_count = 6
      [C10 TASK] cx_count = 6
      [C01 TASK] cx_count = 7
      [C02 TASK] cx_count = 7
      [C03 TASK] cx_count = 7
      [C04 TASK] cx_count = 7
      [C05 TASK] cx_count = 7
      [C06 TASK] cx_count = 7
      [C07 TASK] cx_count = 7
      [C08 TASK] cx_count = 7
      [C09 TASK] cx_count = 7
      [C10 TASK] cx_count = 7
      [C01 TASK] cx_count = 8
      [C02 TASK] cx_count = 8
      [C03 TASK] cx_count = 8
      [C04 TASK] cx_count = 8
      [C05 TASK] cx_count = 8
      [C06 TASK] cx_count = 8
      [C07 TASK] cx_count = 8
      [C08 TASK] cx_count = 8
      [C09 TASK] cx_count = 8
      [C10 TASK] cx_count = 8
      [C01 TASK] cx_count = 9
      [C02 TASK] cx_count = 9
      [C03 TASK] cx_count = 9
      [C04 TASK] cx_count = 9
      [C05 TASK] cx_count = 9
      [C06 TASK] cx_count = 9
      [C07 TASK] cx_count = 9
      [C08 TASK] cx_count = 9
      [C09 TASK] cx_count = 9
      [C10 TASK] cx_count = 9
      
      ==================================================
      PRI  NAME  STACK %(u/t)   C P U   Status  Context
      --------------------------------------------------
        3   C10  41%(192/464)   0.027%   Wait       11
        3   C09  36%(168/464)   0.026%   Wait       11
        3   C08  41%(192/464)   0.027%   Wait       11
        3   C07  41%(192/464)   0.027%   Wait       11
        3   C06  36%(168/464)   0.026%   Wait       11
        3   C05  36%(168/464)   0.026%   Wait       11
        3   C04  41%(192/464)   0.027%   Wait       11
        3   C03  36%(168/464)   0.026%   Wait       11
        3   C02  36%(168/464)   0.026%   Pend       11
        3   C01  36%(168/464)   0.027%   Pend       11
        1    AA  41%(192/464)   0.020%   RUN         1
      255  Idle  64%( 72/112)  99.708%   Pend       38
      
      ---[TOTAL]-------------  99.993%  ------     149
      
         total_us = 10002077
         SysTick->LOAD = 11999
      
      [ M S P ] %(u/t) :  10% (104/1024)
      
      [Mmory]  Peak byte  Used byte  Total  Block  first
      [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
      
      [C01 TASK] cx_count = 10
      [C02 TASK] cx_count = 10
      [C03 TASK] cx_count = 10
      [C04 TASK] cx_count = 10
      [C05 TASK] cx_count = 10
      [C06 TASK] cx_count = 10
      [C07 TASK] cx_count = 10
      [C08 TASK] cx_count = 10
      [C09 TASK] cx_count = 10
      [C10 TASK] cx_count = 10

      --------- 중 략 ---------
*/
