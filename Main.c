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
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{  
  int aa_count = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(;;)
  {
    Boss_sleep(500);  /* 500ms */
    PRINTF("    AA_TASK count = %d \n", ++aa_count);

    if(100 < aa_count)
    {
      break;
    }
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
  return 0;       // 테스크 종료
}


/*===========================================================================
    B B _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t bb_stk[ 512 / sizeof(boss_stk_t)];

int bb_task(void *p_arg)
{
  int bb_count = 0;
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(;;)
  {
    Boss_sleep(10 * 1000);  /* 10 Sec */
    
    #ifdef _BOSS_SPY_
    Boss_spy_report();
    #endif
    
    #ifdef _BOSS_MEM_INFO_
    Boss_mem_info_report();
    #endif

    PRINTF("*BB_TASK count = %d \n", ++bb_count);
    
    if(10 <= bb_count)
    {
      break;
    }
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
  
  (void)Boss_task_create(bb_task, _BOSS_NULL, bb_stk, sizeof(bb_stk),
                                                              PRIO_2, "BB");

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
        ########## 실행 결과 ##########

            [AA TASK] Init 
            [BB TASK] Init 
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
                AA_TASK count = 15 
                AA_TASK count = 16 
                AA_TASK count = 17 
                AA_TASK count = 18 
                AA_TASK count = 19 
                AA_TASK count = 20 
            
            ==================================================
            PRI  NAME  STACK %(u/t)   C P U   Status  Context
            --------------------------------------------------
              2    BB  36%(168/464)   0.002%   RUN         1
              1    AA  36%(168/464)   0.054%   Wait       21
            255  Idle  64%( 72/112)  99.943%   Pend       20
            
            ---[TOTAL]-------------  99.999%  ------      42
            
               total_us = 10000358
               SysTick->LOAD = 11999
            
            [ M S P ] %(u/t) :  10% (104/1024)
            
            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
            
            *BB_TASK count = 1 
                AA_TASK count = 21 
                AA_TASK count = 22 
                AA_TASK count = 23 
                AA_TASK count = 24 
                AA_TASK count = 25 
                AA_TASK count = 26 
                AA_TASK count = 27 
                AA_TASK count = 28 
                AA_TASK count = 29 
                
               ------ 중  략 ------
                
                AA_TASK count = 90 
                AA_TASK count = 91 
                AA_TASK count = 92 
                AA_TASK count = 93 
                AA_TASK count = 94 
                AA_TASK count = 95 
                AA_TASK count = 96 
                AA_TASK count = 97 
                AA_TASK count = 98 
                AA_TASK count = 99 
                AA_TASK count = 100 
            
            ==================================================
            PRI  NAME  STACK %(u/t)   C P U   Status  Context
            --------------------------------------------------
              2    BB  53%(248/464)   0.058%   RUN         5
              1    AA  36%(168/464)   0.055%   Wait      101
            255  Idle  64%( 72/112)  99.885%   Pend      104
            
            ---[TOTAL]-------------  99.998%  ------     210
            
               total_us = 50028063
               SysTick->LOAD = 11999
            
            [ M S P ] %(u/t) :  10% (104/1024)
            
            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
            
            *BB_TASK count = 5 
                AA_TASK count = 101 
            [AA TASK] Exit 
            
            ------ 중  략 ------
            
            *BB_TASK count = 9 
            
            ==================================================
            PRI  NAME  STACK %(u/t)   C P U   Status  Context
            --------------------------------------------------
              2    BB  53%(248/464)   0.061%   RUN        10
            255  Idle  64%( 72/112)  99.909%   Pend      110
            
            ---[TOTAL]-------------  99.970%  ------     120
            
               total_us = 100059062
               SysTick->LOAD = 11999
            
            [ M S P ] %(u/t) :  10% (104/1024)
            
            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
            
            *BB_TASK count = 10 
            [BB TASK] Exit 

*/
