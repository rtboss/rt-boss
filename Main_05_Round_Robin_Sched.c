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
    volatile boss_u32_t delay = 0;
    
    while(delay++ < 1000000)
    {}
    
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
  
  return 0;       // �׽�ũ ����
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
                          aa_stk,               /* ���� ������(base)      */
                          sizeof(aa_stk),       /* ���� ũ��(Bytes)       */
                          PRIO_1,               /* �켱����               */
                          "AA"                  /* �׽�ũ �̸�            */
                        );

  Boss_device_init();         /* Ÿ�̸� �ʱ�ȭ */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*

RT-BOSS�� ���� �κ��� ������ �켱������ �׽�ũ�� ����ϰ� �������� �����մϴ�.
"Boss_Conf.h" ������ _BOSS_RR_QUANTUM_MS ���� �����ϸ� ���� �κ� �ð��� ����˴ϴ�. 


���� : main()�Լ����� AA�׽�ũ�� �����ϰ� AA�׽�ũ���� 10���� Cx�׽�ũ�� ���� ����
       5�ʸ��� SPY�� ����Ѵ�.
       

    ########## ���� ��� ##########

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
      
      ==================================================
      PRI  NAME  STACK %(u/t)   C P U   Status  Context
      --------------------------------------------------
        3   C10  43%(200/464)   9.900%   Pend       21
        3   C09  43%(200/464)  10.000%   Pend       21
        3   C08  43%(200/464)  10.000%   Pend       21
        3   C07  43%(200/464)  10.000%   Pend       21
        3   C06  43%(200/464)  10.000%   Pend       21
        3   C05  43%(200/464)  10.000%   Pend       21
        3   C04  43%(200/464)  10.000%   Pend       21
        3   C03  43%(200/464)  10.000%   Pend       21
        3   C02  43%(200/464)  10.000%   Pend       21
        3   C01  37%(176/464)   9.999%   Pend       21
        1    AA  41%(192/464)   0.020%   RUN         1
      255  Idle  64%( 72/112)   0.074%   Pend        1
      
      ---[TOTAL]-------------  99.993%  ------     212
      
         total_us = 10002063
         SysTick->LOAD = 11999
      
      [ M S P ] %(u/t) :  10% (104/1024)
      
      [Mmory]  Peak byte  Used byte  Total  Block  first
      [Info]     0 ( 0%)    0 ( 0%)  1024     0       0
      
      [C01 TASK] cx_count = 2
      [C02 TASK] cx_count = 2
      [C03 TASK] cx_count = 2
      [C04 TASK] cx_count = 2
      [C05 TASK] cx_count = 2
      [C06 TASK] cx_count = 2

      -------- �� �� --------
*/
