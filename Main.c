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
    Boss_sleep(500);  /* 500ms */
    PRINTF(" AA_TASK count = %d \n", ++aa_count);

    if(100 < aa_count)
    {
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
    Boss_sleep(10 * 1000);  /* 10 Sec */
        
    #ifdef _BOSS_SPY_
    Boss_spy_report();
    #endif
    
    #ifdef _BOSS_MEM_INFO_
    Boss_mem_info_report();
    #endif

    PRINTF("BB_TASK count = %d \n", ++bb_count);
    
    if(10 <= bb_count)
    {
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

            [ M S P ] %(u/t) :  10% (112/1024)

            [TASK]    STACK %(u/t)    C P U    Context
            ------------------------------------------
               BB     32%(168/512)    0.002%         1
               AA     32%(168/512)    0.051%        21
             Idle     56%( 72/128)   99.946%        20
            [TOTAL] :                99.999%        42

               total_us = 10000344
               SysTick->LOAD = 11999

            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0

            BB_TASK count = 1 
             AA_TASK count = 21 
             AA_TASK count = 22 
             AA_TASK count = 23 
             AA_TASK count = 24 
             AA_TASK count = 25 

             ------- 중략 -------

             AA_TASK count = 96 
             AA_TASK count = 97 
             AA_TASK count = 98 
             AA_TASK count = 99 
             AA_TASK count = 100 

            [ M S P ] %(u/t) :  10% (112/1024)

            [TASK]    STACK %(u/t)    C P U    Context
            ------------------------------------------
               BB     48%(248/512)    0.051%         5
               AA     32%(168/512)    0.052%       101
             Idle     56%( 72/128)   99.896%       104
            [TOTAL] :                99.999%       210

               total_us = 50024063
               SysTick->LOAD = 11999

            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0

            BB_TASK count = 5 
             AA_TASK count = 101 
            [AA TASK] Exit 

             ------- 중략 -------
             
            BB_TASK count = 9 

            [ M S P ] %(u/t) :  10% (112/1024)

            [TASK]    STACK %(u/t)    C P U    Context
            ------------------------------------------
               BB     48%(248/512)    0.053%        10
             Idle     56%( 72/128)   99.919%       110
            [TOTAL] :                99.972%       120

               total_us = 100050061
               SysTick->LOAD = 11999

            [Mmory]  Peak byte  Used byte  Total  Block  first
            [Info]     0 ( 0%)    0 ( 0%)  1024     0       0

            BB_TASK count = 10 
            [BB TASK] Exit 

*/
