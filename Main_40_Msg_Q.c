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
BOSS_MSG_Q_ID_T   ex_msg_q_id;

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

  PRINTF("msg_q Create & Init\n");
  ex_msg_q_id = Boss_msg_q_create(10, MSG_Q_PRIORITY);  // msg_fifo = 10, �켱����

  Boss_sleep(100); // BB TASK init wait
  
  for(;;)
  {
    boss_msg_t msg;

    PRINTF("\n[%s] MSG Q Wait\n", Boss_self()->name);
    msg = Boss_msg_wait(ex_msg_q_id, 5000);   /* 5�� Ÿ�Ӿƿ� */

    switch(msg.m_cmd)
    {
      case M_CMD_EMPTY :  // Ÿ�Ӿƿ�
        PRINTF("[%s] MSG Q Wait Timeout\n", Boss_self()->name);
        break;

      case M_CMD_1 :
        PRINTF("[%s] m_cmd = M_CMD_1 : param = %d\n", Boss_self()->name, msg.param);
        break;
      
      default :
        BOSS_ASSERT(_BOSS_FALSE);   // Error : ��ȿ�������� �޽��� ��ɾ�
        break;
    }
  }
    
  return 0;   // Task Exit
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
    //Boss_sleep(7000);  // Ÿ�Ӿƿ� �׽�Ʈ
    
    Boss_sleep(2000);  /* 2�� */
    
    ++bb_count;
    
    PRINTF("[%s] Boss_msg_send( param = %d )\n", Boss_self()->name, bb_count);
    
    if( _BOSS_SUCCESS != Boss_msg_send(ex_msg_q_id, M_CMD_1, (boss_uptr_t)bb_count) )
    {
      PRINTF("FAILURE : MSG Q FIFO FULL\n");
    }
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
  
  (void)Boss_task_create(bb_task, _BOSS_NULL, bb_stk, sizeof(bb_stk),
                                                              PRIO_2, "BB");

  Boss_device_init();         /* Ÿ�̸� �ʱ�ȭ */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
        ########## ���� ��� ##########

            [AA TASK] Init 
            msg_q Create & Init
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

                ------- �߷� -------

*/
