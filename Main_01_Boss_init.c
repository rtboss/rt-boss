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
    
    PRINTF(" AA_TASK count = %d \n", ++aa_count);
  }
  
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
��� : �׽�ũ(AA_Task) 1���� �����Ͽ� 1�ʸ��� �޽����� �����.

���� : RT-BOSS�� �ʱ� ���� ����. 
      1. Boss_init()        : Idle_Task ���� �� �ʱ�ȭ.
      2. Boss_task_create() : 1���̻��� �׽�ũ ����.
      3. Boss_device_init() : Timer �� CPU �� ����.
      4. Boss_start()       : �����ٷ� ����.


        ########## ���� ��� ##########

              [AA TASK] Init 
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
               ...
    
*/
