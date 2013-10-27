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

BOSS_FLAG_ID_T ex_flag_id;


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

  PRINTF("flag grp Create & Init\n");
  ex_flag_id = Boss_flag_grp_create();
  
  Boss_sleep(100); // TASK init wait
  
  for(;;)
  {
    Boss_sleep(2000);
    
    PRINTF("\n[%s] (%d) Boss_flag_send()\n", Boss_self()->name, ++aa_count);
    Boss_flag_send(ex_flag_id, 0x0001);
  }

  Boss_flag_grp_del(ex_flag_id);  // �÷��� �׷� ����
  
  return 0;       // �׽�ũ ����
}


/*===========================================================================
    B B _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t bb_stk[ 512 / sizeof(boss_stk_t)];

int bb_task(void *p_arg)
{
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  Boss_sleep(100); // TASK init wait
  
  for(;;)
  {
    boss_flags_t flags;
    
    flags = Boss_flag_wait(ex_flag_id, 0x0001, _FLAG_OPT_OR, 20*1000/*20��*/);
    if(flags != 0)
    {
      PRINTF("[%s] Boss_flag_wait(OR) flags = 0x%04x\n", Boss_self()->name, flags);
      Boss_flag_clear(ex_flag_id, 0x0001);  // ������ flag ���� Ŭ����
    }
    else
    {
      PRINTF("[%s] Timeout Flag Wait\n", Boss_self()->name);
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
            flag grp Create & Init
            [BB TASK] Init 
            
            [AA] (1) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (2) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (3) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (4) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (5) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (6) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
            
            [AA] (7) Boss_flag_send()
            [BB] Boss_flag_wait(OR) flags = 0x0001
*/
