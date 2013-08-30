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

    PRINTF("----- [BB_TASK (10�ʸ���)] ----- count = %d \n", ++bb_count);
    
    if(10 <= bb_count)
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
��� : 2���� �׽�ũ AA�׽�ũ�� BB�׽�ũ�� �����Ͽ� 
       AA�׽�ũ�� 1�ʸ��� �޽����� ����ϰ� 100�� ���� �� ����
       BB�׽�ũ�� 10�ʸ��� �޽����� ����ϰ� 10�� ���� �� ����.

���� : �׽�ũ ������ Boss_task_create()�� �����ϸ�
       �׽�ũ ����� �׽�ũ ������ "0"�� �����ϸ� �����Ѵ�.(rtetur 0)

       #### 1. �׽�ũ ���� �Լ� ####
          void Boss_task_create(  int (*task)(void *p_arg), void *p_arg, 
                          boss_tcb_t *p_tcb, boss_prio_t prio, 
                          boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                          const char *name )

            - *task  : �׽�ũ �Լ� ������.
            - *p_arg : �׽�ũ ����� ���ް�.
            - *p_tcb : �׽�ũ�� ������ ������ �ִ� TCB
            - prio   : �켱���� (1~254 ���� ����)
                        0 - ����(���� �̻��)
                        1 - ���� ���� �켱����
                      254 - ���� ���� �켱����  
                      255 - Idle�׽�ũ �켱���� (Idle �׽�ũ�� ���Ұ�)
            - sp_base   : ���� �޸���(base �ּ�)
            - stk_bytes : ���� ũ��
            - name      : �׽�ũ �̸�

      #### 2. �׽�ũ ���� ####
        int aa_main(void *p_arg)
        {
          // �׽�ũ �ʱ�ȭ
          
          for(;;) {
           // �׽�ũ ���� �ڵ�
          }

          // �׽�ũ ���� �ڵ�
          return 0;       // �׽�ũ ����
        }





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
  ----- [BB_TASK (10�ʸ���)] ----- count = 1 
   [AA_TASK] (1�ʸ���) count = 11 
   [AA_TASK] (1�ʸ���) count = 12 
   [AA_TASK] (1�ʸ���) count = 13 
   [AA_TASK] (1�ʸ���) count = 14 
   [AA_TASK] (1�ʸ���) count = 15 
   [AA_TASK] (1�ʸ���) count = 16 
   [AA_TASK] (1�ʸ���) count = 17 
   [AA_TASK] (1�ʸ���) count = 18 
   [AA_TASK] (1�ʸ���) count = 19 
   [AA_TASK] (1�ʸ���) count = 20 
  ----- [BB_TASK (10�ʸ���)] ----- count = 2 
   [AA_TASK] (1�ʸ���) count = 21 
   [AA_TASK] (1�ʸ���) count = 22 
   [AA_TASK] (1�ʸ���) count = 23 
  
   --- �߷� ----
    
   [AA_TASK] (1�ʸ���) count = 87 
   [AA_TASK] (1�ʸ���) count = 88 
   [AA_TASK] (1�ʸ���) count = 89 
   [AA_TASK] (1�ʸ���) count = 90 
  ----- [BB_TASK (10�ʸ���)] ----- count = 9 
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
  [AA_TASK] Exit 
  ----- [BB_TASK (10�ʸ���)] ----- count = 10 
  [BB_TASK] Exit 

    
*/
