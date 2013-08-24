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

boss_mbox_q_t aa_mbox_q;
/*===============================================
    A A _ M A I N
-----------------------------------------------*/
int aa_main(void *p_arg)
{  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  Boss_mbox_q_init(&aa_mbox_q, &aa_tcb, SIG_00_BIT);
  
  for(;;)
  {
    boss_sigs_t sigs = Boss_wait(SIG_00_BIT);

    if(sigs & SIG_00_BIT)
    {
      int *mbox_add_arg = Boss_mbox_receive(&aa_mbox_q);
      int result;

      //Boss_sleep(10);     // 메일박스 타임아웃 테스트

      result = mbox_add_arg[0] + mbox_add_arg[1];

      Boss_mbox_done(mbox_add_arg, result);
    }
  }
  
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
    int         *mbox_add_arg;
    boss_uptr_t rsp;
  
    ++bb_count;
    
    Boss_sleep(1 * 1000);  /* 1 Sec */

    mbox_add_arg = Boss_mbox_alloc(sizeof(int) * 2);

    mbox_add_arg[0] = 1000;
    mbox_add_arg[1] = bb_count;

    if(_BOSS_SUCCESS == Boss_mbox_pend(&aa_mbox_q, mbox_add_arg, &rsp, 5/*ms*/))
    {
      PRINTF(" rsp = %d  \n", rsp);
    }
    else
    {
      PRINTF("메일박스 [타임 아웃 : ERROR] \n");
    }
  }
  
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

요약 : BB_Task 에서 값을 MailBox로 보내면 AA_Task에서 더해서 응답하는 예제


        ########## 실행 결과 ##########

            [AA TASK] Init 
            [BB TASK] Init 
             rsp = 1001  
             rsp = 1002  
             rsp = 1003  
             rsp = 1004  
             rsp = 1005  
             rsp = 1006  
             rsp = 1007  
             rsp = 1008  
             rsp = 1009  
             rsp = 1010  
             rsp = 1011  
             rsp = 1012  
             rsp = 1013  
             rsp = 1014  
             rsp = 1015  
             rsp = 1016  
             rsp = 1017  
             rsp = 1018  
             rsp = 1019  
             rsp = 1020  
             rsp = 1021  
             rsp = 1022  
             rsp = 1023  
             rsp = 1024  
             rsp = 1025  
             rsp = 1026  
             rsp = 1027  
             rsp = 1028  
             rsp = 1029  
             rsp = 1030  
             rsp = 1031  
             rsp = 1032  
             rsp = 1033  
             rsp = 1034  
             rsp = 1035  
             rsp = 1036  
             rsp = 1037

            --- 중략 ---
*/
