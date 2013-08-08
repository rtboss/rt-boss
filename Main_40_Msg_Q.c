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

boss_msg_q_t  aa_msg_q;
_msg_fifo_t   aa_msg_fifo[10];


/*===============================================
    A A _ M A I N
-----------------------------------------------*/
int aa_main(void *p_arg)
{  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  PRINTF("aa_msg_q Init\n");
  Boss_msg_q_init(&aa_msg_q, aa_msg_fifo, sizeof(aa_msg_fifo), Boss_self(), SIG_00_BIT);
  
  for(;;)
  {
    boss_sigs_t sigs = Boss_wait(SIG_00_BIT);

    if(sigs & SIG_00_BIT)
    {
      msg_cmd_t   m_cmd;
      boss_uptr_t param;
      
      while( M_CMD_EMPTY != (m_cmd = Boss_msg_receive(&aa_msg_q, &param)) )
      {
        switch(m_cmd)
        {
          case M_CMD_1 :
            PRINTF("M_CMD_1 : param (bb_count = %d)\n", param);
            break;
            
          default :
            BOSS_ASSERT(_BOSS_FALSE);
            break;
        }
      }
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
    Boss_sleep(1000);  /* 1 Sec */
    
    ++bb_count;

    if( _BOSS_SUCCESS != Boss_msg_send(&aa_msg_q, M_CMD_1, (boss_uptr_t)bb_count) )
    {
      PRINTF("FAILURE : 메시지 FULL\n");
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
            aa_msg_q Init
            [BB TASK] Init 
            M_CMD_1 : param (bb_count = 1)
            M_CMD_1 : param (bb_count = 2)
            M_CMD_1 : param (bb_count = 3)
            M_CMD_1 : param (bb_count = 4)
            M_CMD_1 : param (bb_count = 5)
            M_CMD_1 : param (bb_count = 6)
            M_CMD_1 : param (bb_count = 7)
            M_CMD_1 : param (bb_count = 8)
            M_CMD_1 : param (bb_count = 9)
            M_CMD_1 : param (bb_count = 10)
            M_CMD_1 : param (bb_count = 11)
            M_CMD_1 : param (bb_count = 12)
            M_CMD_1 : param (bb_count = 13)
            M_CMD_1 : param (bb_count = 14)
            M_CMD_1 : param (bb_count = 15)
            M_CMD_1 : param (bb_count = 16)
            M_CMD_1 : param (bb_count = 17)
            M_CMD_1 : param (bb_count = 18)
            M_CMD_1 : param (bb_count = 19)
            M_CMD_1 : param (bb_count = 20)
            M_CMD_1 : param (bb_count = 21)
            M_CMD_1 : param (bb_count = 22)
            M_CMD_1 : param (bb_count = 23)
            M_CMD_1 : param (bb_count = 24)
            M_CMD_1 : param (bb_count = 25)
            M_CMD_1 : param (bb_count = 26)
            M_CMD_1 : param (bb_count = 27)
            M_CMD_1 : param (bb_count = 28)
            M_CMD_1 : param (bb_count = 29)
            M_CMD_1 : param (bb_count = 30)
            M_CMD_1 : param (bb_count = 31)
                ------- 중략 -------

*/
