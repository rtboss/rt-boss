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
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  PRINTF("test_msg_q Init\n");
  Boss_msg_q_init(&test_msg_q, test_msg_fifo, sizeof(test_msg_fifo), MSG_Q_FIFO);
  //Boss_msg_q_init(&test_msg_q, test_msg_fifo, sizeof(test_msg_fifo), MSG_Q_PRIORITY);
  
  Boss_sleep(100); // TASK init wait
  
  for(;;)
  {
    static int param_count = 0;

    Boss_sleep(2000);    
    param_count++;

    if(_BOSS_SUCCESS != Boss_msg_send(&test_msg_q, M_CMD_1, (boss_uptr_t)param_count))
    {
      PRINTF("FAILURE : MSG Q FIFO FULL\n");
    }
    else
    {      
      PRINTF("\n[%s] Boss_msg_send( param = %d )\n", Boss_self()->name, param_count);
    }
  }
  
  return 0;       // 테스크 종료
}


/*===========================================================================
    C X _ T A S K
---------------------------------------------------------------------------*/
#define CX_TASK_MAX   5
    
boss_stk_t    cx_stk[CX_TASK_MAX][ 512 / sizeof(boss_stk_t) ];

int cx_task(void *p_arg)
{
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  Boss_sleep(100); // TASK init wait

  for(;;)
  {
    boss_msg_t msg = Boss_msg_wait(&test_msg_q, 10*1000/*10초*/);

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
  
  return 0;       // 테스크 종료
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
                          aa_stk,               /* 스택 포인터(base)      */
                          sizeof(aa_stk),       /* 스택 크기(Bytes)       */
                          PRIO_1,               /* 우선순위               */
                          "AA"                  /* 테스크 이름            */
                        );

  {
    int idx = 0;
    boss_prio_t prio = PRIO_3;
  
    for(idx=0; idx < CX_TASK_MAX; idx++)
    {
      char name[10];
      sprintf(name, "C%02d", idx + 1);
      (void)Boss_task_create( cx_task,              /* Task Entry Point     */
                              (void *)idx,          /* Task Argument        */
                              cx_stk[idx],          /* Stack Point (Base)   */
                              sizeof(cx_stk[idx]),  /* Stack Size (Bytes)   */
                              prio++,               /* Priority             */
                              name                  /* Task Name String     */
                            );
    }
  }

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
        ########## 실행 결과 ##########
        
            [AA TASK] Init 
            test_msg_q Init
            [C01 TASK] Init 
            [C02 TASK] Init 
            [C03 TASK] Init 
            [C04 TASK] Init 
            [C05 TASK] Init 
            
            [AA] Boss_msg_send( param = 1 )
            [C01] m_cmd = M_CMD_1 : param = 1
            
            [AA] Boss_msg_send( param = 2 )
            [C02] m_cmd = M_CMD_1 : param = 2
            
            [AA] Boss_msg_send( param = 3 )
            [C03] m_cmd = M_CMD_1 : param = 3
            
            [AA] Boss_msg_send( param = 4 )
            [C04] m_cmd = M_CMD_1 : param = 4
            
            [AA] Boss_msg_send( param = 5 )
            [C05] m_cmd = M_CMD_1 : param = 5
            
            [AA] Boss_msg_send( param = 6 )
            [C01] m_cmd = M_CMD_1 : param = 6
            
            [AA] Boss_msg_send( param = 7 )
            [C02] m_cmd = M_CMD_1 : param = 7
            
            [AA] Boss_msg_send( param = 8 )
            [C03] m_cmd = M_CMD_1 : param = 8
            
            [AA] Boss_msg_send( param = 9 )
            [C04] m_cmd = M_CMD_1 : param = 9
            
            [AA] Boss_msg_send( param = 10 )
            [C05] m_cmd = M_CMD_1 : param = 10
            
            [AA] Boss_msg_send( param = 11 )
            [C01] m_cmd = M_CMD_1 : param = 11
            
            [AA] Boss_msg_send( param = 12 )
            [C02] m_cmd = M_CMD_1 : param = 12

            ------- 중략 -------

*/
