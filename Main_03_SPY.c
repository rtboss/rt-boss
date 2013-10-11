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
    Boss_sleep(1000);  /* 1000ms (1초) */
    
    PRINTF(" [AA_TASK] (1초마다) count = %d \n", ++aa_count);

    if(100 <= aa_count)
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

    if(10 <= bb_count++)
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
요약 : AA테스크는 1초마다 메시지를 출력하고 100번 실행 후 종료
       BB테스크는 10초마다 SPY 정보를 출력하고 10번 실행 후 종료.

설명 : RT-BOSS 의 디버깅 정보를 출력하는 SPY는 테스크 생성시 등록되며
       문맥전환시 테스크 정보를 업데이트 한다.
       Boss_spy_report() 함수를 호출 하여 SPY 정보를 출력할수 있다.
* 주의 : SPY 정보중 테스크의 CPU 점유률을 확인하는 변수의 크기가
       32bit형으로 70분정도 사용가능하다 60분마다 Boss_spy_restart() 함수를
       호출하여 초기화 해야 정상적인 CPU 점유율을 확인 할수 있다.



      ########## 실행 결과 ##########
      
        [AA TASK] Init 
        [BB TASK] Init 
         [AA_TASK] (1초마다) count = 1 
         [AA_TASK] (1초마다) count = 2 
         [AA_TASK] (1초마다) count = 3 
         [AA_TASK] (1초마다) count = 4 
         [AA_TASK] (1초마다) count = 5 
         [AA_TASK] (1초마다) count = 6 
         [AA_TASK] (1초마다) count = 7 
         [AA_TASK] (1초마다) count = 8 
         [AA_TASK] (1초마다) count = 9 
         [AA_TASK] (1초마다) count = 10 
        
        ==================================================
        PRI  NAME  STACK %(u/t)   C P U   Status  Context
        --------------------------------------------------
          2    BB  36%(168/464)   0.002%   RUN         1
          1    AA  36%(168/464)   0.031%   Wait       11
        255  Idle  64%( 72/112)  99.965%   Pend       10
        
        ---[TOTAL]-------------  99.998%  ------      22
        
           total_us = 10000410
           SysTick->LOAD = 11999
        
        [ M S P ] %(u/t) :  10% (104/1024)
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
        
         [AA_TASK] (1초마다) count = 11 
         [AA_TASK] (1초마다) count = 12 
         [AA_TASK] (1초마다) count = 13 
         [AA_TASK] (1초마다) count = 14 
         [AA_TASK] (1초마다) count = 15 

         ------------ 중 략 ------------

         [AA_TASK] (1초마다) count = 86 
         [AA_TASK] (1초마다) count = 87 
         [AA_TASK] (1초마다) count = 88 
         [AA_TASK] (1초마다) count = 89 
         [AA_TASK] (1초마다) count = 90 
        
        ==================================================
        PRI  NAME  STACK %(u/t)   C P U   Status  Context
        --------------------------------------------------
          2    BB  55%(256/464)   0.063%   RUN         9
          1    AA  36%(168/464)   0.032%   Wait       91
        255  Idle  64%( 72/112)  99.903%   Pend       98
        
        ---[TOTAL]-------------  99.998%  ------     198
        
           total_us = 90056063
           SysTick->LOAD = 11999
        
        [ M S P ] %(u/t) :  10% (104/1024)
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
        
         [AA_TASK] (1초마다) count = 91 
         [AA_TASK] (1초마다) count = 92 
         [AA_TASK] (1초마다) count = 93 
         [AA_TASK] (1초마다) count = 94 
         [AA_TASK] (1초마다) count = 95 
         [AA_TASK] (1초마다) count = 96 
         [AA_TASK] (1초마다) count = 97 
         [AA_TASK] (1초마다) count = 98 
         [AA_TASK] (1초마다) count = 99 
         [AA_TASK] (1초마다) count = 100 
        [AA TASK] Exit 
        
        ==================================================
        PRI  NAME  STACK %(u/t)   C P U   Status  Context
        --------------------------------------------------
          2    BB  55%(256/464)   0.064%   RUN        10
        255  Idle  64%( 72/112)  99.902%   Pend      109
        
        ---[TOTAL]-------------  99.966%  ------     119
        
           total_us = 100063062
           SysTick->LOAD = 11999
        
        [ M S P ] %(u/t) :  10% (104/1024)
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
        
        
        ==================================================
        PRI  NAME  STACK %(u/t)   C P U   Status  Context
        --------------------------------------------------
          2    BB  55%(256/464)   0.064%   RUN        11
        255  Idle  64%( 72/112)  99.905%   Pend      110
        
        ---[TOTAL]-------------  99.969%  ------     121
        
           total_us = 110069062
           SysTick->LOAD = 11999
        
        [ M S P ] %(u/t) :  10% (104/1024)
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
        
        [BB TASK] Exit 
*/
