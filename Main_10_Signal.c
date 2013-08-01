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
    boss_sigs_t sigs = Boss_wait(SIG_01_BIT);

    if(sigs & SIG_01_BIT)
    {
      PRINTF(" [AA] SIG_01_BIT ����\n");
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
    Boss_sleep(3 * 1000);

    PRINTF("\n(BB : %d) aa_tcb - SIG_01_BIT ����(��)\n", ++bb_count);
    Boss_send(&aa_tcb, SIG_01_BIT);
    PRINTF("(BB) aa_tcb - SIG_01_BIT ����(��)\n");
    
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
���� : �׽�ũ �ñ׳�(Signal) ��� ����. 
       AA �׽�ũ�� "SIG_01_BIT" �ñ׳��� ������ �޽����� �����.
       BB �׽�ũ�� 3�ʸ��� AA �׽�ũ�� "SIG_01_BIT" �ñ׳��� ������.


### ������ ###

  [AA TASK] Init 
  [BB TASK] Init 
  
  (BB : 1) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 2) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 3) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 4) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 5) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 6) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 7) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 8) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 9) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  
  (BB : 10) aa_tcb - SIG_01_BIT ����(��)
   [AA] SIG_01_BIT ����
  (BB) aa_tcb - SIG_01_BIT ����(��)
  [BB TASK] Exit 
    
*/

