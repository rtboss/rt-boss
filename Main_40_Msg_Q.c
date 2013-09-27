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
boss_msg_q_t  test_msg_q;
boss_msg_t    test_msg_fifo[10];

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
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  PRINTF("test_msg_q Init\n");
  Boss_msg_q_init(&test_msg_q, test_msg_fifo, sizeof(test_msg_fifo), MSG_Q_PRIORITY);

  Boss_sleep(100); // BB TASK init wait
  
  for(;;)
  {
    boss_msg_t msg;

    PRINTF("\n[%s] MSG Q Wait\n", Boss_self()->name);
    msg = Boss_msg_wait(&test_msg_q, 5000);   /* 5초 타임아웃 */

    switch(msg.m_cmd)
    {
      case M_CMD_EMPTY :  // 타임아웃
        PRINTF("[%s] MSG Q Wait Timeout\n", Boss_self()->name);
        break;

      case M_CMD_1 :
        PRINTF("[%s] m_cmd = M_CMD_1 : param = %d\n", Boss_self()->name, msg.param);
        break;
      
      default :
        BOSS_ASSERT(_BOSS_FALSE);   // Error : 유효하지않은 메시지 명령어
        break;
    }
  }
    
  return 0;   // Task Exit
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
    //Boss_sleep(7000);  // 타임아웃 테스트
    
    Boss_sleep(2000);  /* 2초 */
    
    ++bb_count;
    
    PRINTF("[%s] Boss_msg_send( param = %d )\n", Boss_self()->name, bb_count);
    
    if( _BOSS_SUCCESS != Boss_msg_send(&test_msg_q, M_CMD_1, (boss_uptr_t)bb_count) )
    {
      PRINTF("FAILURE : MSG Q FIFO FULL\n");
    }
  }
  
  return 0;   // Task Exit
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
            test_msg_q Init
            [BB TASK] Init 
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 1 )
            [AA] m_cmd = M_CMD_1 : param = 1
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 2 )
            [AA] m_cmd = M_CMD_1 : param = 2
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 3 )
            [AA] m_cmd = M_CMD_1 : param = 3
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 4 )
            [AA] m_cmd = M_CMD_1 : param = 4
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 5 )
            [AA] m_cmd = M_CMD_1 : param = 5
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 6 )
            [AA] m_cmd = M_CMD_1 : param = 6
            
            [AA] MSG Q Wait
            [BB] Boss_msg_send( param = 7 )
            [AA] m_cmd = M_CMD_1 : param = 7
            
            [AA] MSG Q Wait

                ------- 중략 -------

*/
