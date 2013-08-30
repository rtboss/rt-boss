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
    Boss_sleep(1000);  /* 1000ms (1��) */
    
    PRINTF(" [AA_TASK] (1�ʸ���) count = %d \n", ++aa_count);

    if(100 <= aa_count)
    {
      break;
    }
  }
  
  PRINTF("[AA_TASK] Exit \n");
  
  return 0;       // �׽�ũ ����
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
  
  return 0;       // �׽�ũ ����
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
                    AA_PRIO_1,            /* �켱����               */
                    (boss_stk_t *)aa_stk, /* ���� ������(base)      */
                    sizeof(aa_stk),       /* ���� ũ��(Bytes)       */
                    "AA"                  /* �׽�ũ �̸�            */
                    );
  
  Boss_task_create( bb_main, _BOSS_NULL, &bb_tcb, BB_PRIO_2,
                    (boss_stk_t *)bb_stk, sizeof(bb_stk), "BB" );

  Boss_device_init();         /* Ÿ�̸� �ʱ�ȭ */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
��� : AA�׽�ũ�� 1�ʸ��� �޽����� ����ϰ� 100�� ���� �� ����
       BB�׽�ũ�� 10�ʸ��� SPY ������ ����ϰ� 10�� ���� �� ����.

���� : RT-BOSS �� ����� ������ ����ϴ� SPY�� �׽�ũ ������ ��ϵǸ�
       ������ȯ�� �׽�ũ ������ ������Ʈ �Ѵ�.
       Boss_spy_report() �Լ��� ȣ�� �Ͽ� SPY ������ ����Ҽ� �ִ�.
* ���� : SPY ������ �׽�ũ�� CPU �������� Ȯ���ϴ� ������ ũ�Ⱑ
       32bit������ 70������ ��밡���ϴ� 60�и��� Boss_spy_restart() �Լ���
       ȣ���Ͽ� �ʱ�ȭ �ؾ� �������� CPU �������� Ȯ�� �Ҽ� �ִ�.


### ������ ###

  [AA_TASK] Init 
  [BB_TASK] Init 
   [AA_TASK] (1�ʸ���) count = 1 
   [AA_TASK] (1�ʸ���) count = 2 
   [AA_TASK] (1�ʸ���) count = 3 
   [AA_TASK] (1�ʸ���) count = 4 
   [AA_TASK] (1�ʸ���) count = 5 
   [AA_TASK] (1�ʸ���) count = 6 
   [AA_TASK] (1�ʸ���) count = 7 
   [AA_TASK] (1�ʸ���) count = 8 
   [AA_TASK] (1�ʸ���) count = 9 
   [AA_TASK] (1�ʸ���) count = 10 
  
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
  
   [AA_TASK] (1�ʸ���) count = 11 
   [AA_TASK] (1�ʸ���) count = 12 
   [AA_TASK] (1�ʸ���) count = 13 
  
   --- �߷� ----
   
   [AA_TASK] (1�ʸ���) count = 97 
   [AA_TASK] (1�ʸ���) count = 98 
   [AA_TASK] (1�ʸ���) count = 99 
   [AA_TASK] (1�ʸ���) count = 100 
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
