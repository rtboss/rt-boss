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
    int delay = 0;

    while(delay++ < 1000000)
    {}
    
    PRINTF("[%s TASK] cx_count = %d\n", Boss_self()->name, cx_count++);

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
    Boss_sleep(5 * 1000);   /* 5 Sec */

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

RT-BOSS의 라운드 로빈은 동일한 우선순위의 테스크가 대기하고 있을때만 동작합니다.
"Boss_Conf.h" 파일의 _BOSS_RR_QUANTUM_MS 값을 변경하면 라운드 로빈 시간이 변경됩니다. 

주의 : 테스크 10생성시 3Kbyte 정도의 메모리를 사용함.

설명 : main()함수에서 AA테스크를 생성하고 AA테스크에서 10개의 Cx테스크를 생성 한후
       5초마다 SPY를 출력한다.
       

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
       
       [ M S P ] %(u/t) :  10% (112/1024)
       
       [TASK]   STACK %(u/t)    C P U    Context
       ------------------------------------------
         C09    42%(108/256)    9.998%        10
         C08    42%(108/256)    9.997%        10
         C07    42%(108/256)    9.997%        10
         C06    42%(108/256)    9.997%        10
         C05    42%(108/256)    9.997%        10
         C04    42%(108/256)    9.997%        10
         C03    42%(108/256)    9.997%        10
         C02    42%(108/256)    9.997%        10
         C01    42%(108/256)    9.997%        10
         C00    42%(108/256)    9.985%        10
          AA    35%(184/512)    0.033%         1
        Idle     3%(  4/128)    0.000%         0
       [TOTAL] :               99.992%       101
       
          total_us = 5001069
          SysTick->LOAD = 11999
       
       [Mmory]  Peak byte  Used byte  Total  Block  first
       [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
       
       [C00 TASK] cx_count = 0
       [C01 TASK] cx_count = 0
       [C02 TASK] cx_count = 0
       [C03 TASK] cx_count = 0
       [C04 TASK] cx_count = 0
       [C05 TASK] cx_count = 0
       [C06 TASK] cx_count = 0
       [C07 TASK] cx_count = 0
       [C08 TASK] cx_count = 0
       [C09 TASK] cx_count = 0
       
       [ M S P ] %(u/t) :  10% (112/1024)
       
       [TASK]   STACK %(u/t)    C P U    Context
       ------------------------------------------
         C09    46%(120/256)    9.984%        20
         C08    46%(120/256)    9.983%        20
         C07    46%(120/256)    9.983%        20
         C06    46%(120/256)    9.983%        20
         C05    46%(120/256)    9.983%        20
         C04    46%(120/256)    9.983%        20
         C03    46%(120/256)    9.983%        20
         C02    46%(120/256)    9.983%        20
         C01    46%(120/256)    9.983%        20
         C00    46%(120/256)    9.977%        20
          AA    46%(240/512)    0.166%         2
        Idle     3%(  4/128)    0.000%         0
       [TOTAL] :               99.991%       202
       
          total_us = 10016069
          SysTick->LOAD = 11999
       
       [Mmory]  Peak byte  Used byte  Total  Block  first
       [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
       
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
       
       [ M S P ] %(u/t) :  10% (112/1024)
       
       [TASK]   STACK %(u/t)    C P U    Context
       ------------------------------------------
         C09    48%(124/256)    9.979%        30
         C08    48%(124/256)    9.979%        30
         C07    48%(124/256)    9.979%        30
         C06    48%(124/256)    9.979%        30
         C05    48%(124/256)    9.979%        30
         C04    48%(124/256)    9.979%        30
         C03    48%(124/256)    9.979%        30
         C02    48%(124/256)    9.979%        30
         C01    48%(124/256)    9.979%        30
         C00    48%(124/256)    9.973%        30
          AA    50%(256/512)    0.211%         3
        Idle     3%(  4/128)    0.000%         0
       [TOTAL] :               99.995%       303
       
          total_us = 15031069
          SysTick->LOAD = 11999
       
       [Mmory]  Peak byte  Used byte  Total  Block  first
       [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
       
       --- 계속 ---

*/
