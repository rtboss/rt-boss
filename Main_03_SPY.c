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
  
  PRINTF("[AA_TASK] Init \n");
  
  for(;;)
  {    
    Boss_sleep(1000);  /* 1000ms (1초) */
    
    PRINTF(" [AA_TASK] (1초마다) count = %d \n", ++aa_count);

    if(100 <= aa_count)
    {
      break;
    }
  }
  
  PRINTF("[AA_TASK] Exit \n");
  
  return 0;       // 테스크 종료
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
  
  PRINTF("[BB_TASK] Init \n");
  
  for(;;)
  {
    Boss_sleep(10 * 1000);   /* 10 Sec */

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

  PRINTF("[BB_TASK] Exit \n");
  
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
  
  Boss_task_create( bb_main, _BOSS_NULL, &bb_tcb, BB_PRIO_2,
                    (boss_stk_t *)bb_stk, sizeof(bb_stk), "BB" );

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


### 실행결과 ###

  [AA_TASK] Init 
  [BB_TASK] Init 
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
  
  [ M S P ] %(u/t) :  10% (112/1024)
  
  [TASK]   STACK %(u/t)    C P U    Context
  ------------------------------------------
     BB    31%(160/512)    0.002%         1
     AA    32%(168/512)    0.031%        11
   Idle    56%( 72/128)   99.966%        10
  [TOTAL] :               99.999%        22
  
     total_us = 10000403
     SysTick->LOAD = 11999
  
  [Mmory]  Peak byte  Used byte  Total  Block  first
  [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
  
   [AA_TASK] (1초마다) count = 11 
   [AA_TASK] (1초마다) count = 12 
   [AA_TASK] (1초마다) count = 13 
  
   --- 중략 ----
   
   [AA_TASK] (1초마다) count = 97 
   [AA_TASK] (1초마다) count = 98 
   [AA_TASK] (1초마다) count = 99 
   [AA_TASK] (1초마다) count = 100 
  [AA_TASK] Exit 
  
  [ M S P ] %(u/t) :  10% (112/1024)
  
  [TASK]   STACK %(u/t)    C P U    Context
  ------------------------------------------
     BB    48%(248/512)    0.055%        10
   Idle    56%( 72/128)   99.911%       109
  [TOTAL] :               99.966%       119
  
     total_us = 100054052
     SysTick->LOAD = 11999
  
  [Mmory]  Peak byte  Used byte  Total  Block  first
  [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
  
  
  [ M S P ] %(u/t) :  10% (112/1024)
  
  [TASK]   STACK %(u/t)    C P U    Context
  ------------------------------------------
     BB    48%(248/512)    0.055%        11
   Idle    56%( 72/128)   99.914%       110
  [TOTAL] :               99.969%       121
  
     total_us = 110059052
     SysTick->LOAD = 11999
  
  [Mmory]  Peak byte  Used byte  Total  Block  first
  [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
  
  [BB_TASK] Exit 

*/
