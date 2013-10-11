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
    Boss_sleep(1000);  /* 1000ms (1��) */
    
    PRINTF(" [AA_TASK] (1�ʸ���) count = %d \n", ++aa_count);

    if(100 <= aa_count)
    {
      break;
    }
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
  return 0;       // �׽�ũ ����
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
  
  (void)Boss_task_create(bb_task, _BOSS_NULL, bb_stk, sizeof(bb_stk),
                                                              PRIO_2, "BB");

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



      ########## ���� ��� ##########
      
        [AA TASK] Init 
        [BB TASK] Init 
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
        
         [AA_TASK] (1�ʸ���) count = 11 
         [AA_TASK] (1�ʸ���) count = 12 
         [AA_TASK] (1�ʸ���) count = 13 
         [AA_TASK] (1�ʸ���) count = 14 
         [AA_TASK] (1�ʸ���) count = 15 

         ------------ �� �� ------------

         [AA_TASK] (1�ʸ���) count = 86 
         [AA_TASK] (1�ʸ���) count = 87 
         [AA_TASK] (1�ʸ���) count = 88 
         [AA_TASK] (1�ʸ���) count = 89 
         [AA_TASK] (1�ʸ���) count = 90 
        
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
        
         [AA_TASK] (1�ʸ���) count = 91 
         [AA_TASK] (1�ʸ���) count = 92 
         [AA_TASK] (1�ʸ���) count = 93 
         [AA_TASK] (1�ʸ���) count = 94 
         [AA_TASK] (1�ʸ���) count = 95 
         [AA_TASK] (1�ʸ���) count = 96 
         [AA_TASK] (1�ʸ���) count = 97 
         [AA_TASK] (1�ʸ���) count = 98 
         [AA_TASK] (1�ʸ���) count = 99 
         [AA_TASK] (1�ʸ���) count = 100 
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
