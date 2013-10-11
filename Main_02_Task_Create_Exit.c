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
    Boss_sleep(10 * 1000);   /* 10 Sec */

    PRINTF("----- [BB_TASK (10�ʸ���)] ----- count = %d \n", ++bb_count);
    
    if(10 <= bb_count)
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
��� : 2���� �׽�ũ AA�׽�ũ�� BB�׽�ũ�� �����Ͽ� 
       AA�׽�ũ�� 1�ʸ��� �޽����� ����ϰ� 100�� ���� �� ����
       BB�׽�ũ�� 10�ʸ��� �޽����� ����ϰ� 10�� ���� �� ����.

���� : �׽�ũ ������ Boss_task_create()�� �����ϸ�
       �׽�ũ ����� �׽�ũ ������ "0"�� �����ϸ� �����Ѵ�.(return 0)

       #### 1. �׽�ũ ���� �Լ� ####
       BOSS_TID_T Boss_task_create(int (*task_entry)(void *p_arg), void *p_arg,
                                  boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                                            boss_prio_t prio, const char *name)

            - *task_entry : �׽�ũ �Լ� ������.
            - *p_arg      : �׽�ũ ����� ���ް�.
            - sp_base     : ���� �޸���(base �ּ�)
            - stk_bytes   : ���� ũ��
            - prio   : �켱���� (1~254 ���� ����)
                        0 - ����(���� �̻��)
                        1 - ���� ���� �켱����
                      254 - ���� ���� �켱����  
                      255 - Idle�׽�ũ �켱���� (Idle �׽�ũ �� ���Ұ�)
            - name      : �׽�ũ �̸�

            *��ȯ�� : Task ID �� ��ȯ��.( TCB Point )

      #### 2. �׽�ũ ���� ####
        int aa_task(void *p_arg)
        {
          // �׽�ũ �ʱ�ȭ
          
          for(;;) {
           // �׽�ũ ���� �ڵ�
          }

          // �׽�ũ ���� �ڵ�
          return 0;       // �׽�ũ ����
        }





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

         ------------ �� �� ------------

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
        [AA TASK] Exit 
        ----- [BB_TASK (10�ʸ���)] ----- count = 10 
        [BB TASK] Exit 
    
*/
