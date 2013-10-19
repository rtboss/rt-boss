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

boss_flag_grp_t test_flag_grp;


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

  PRINTF("test_flag_grp Init\n");
  Boss_flag_grp_init(&test_flag_grp);

  Boss_sleep(100); // TASK init wait
  
  for(;;)
  {
    Boss_sleep(2000);    
    
    PRINTF("\n[%s] (%d) Boss_flag_send()\n", Boss_self()->name, ++aa_count);
    Boss_flag_send(&test_flag_grp, 0x0001);
  }
  
  return 0;       // 테스크 종료
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
    boss_flags_t flags = Boss_flag_wait(&test_flag_grp, 0x0001,
                                                _FLAG_OPT_OR, 20*1000/*20초*/);
    if(flags != 0)
    {
      PRINTF("[%s] Boss_flag_wait(OR + CONSUME) flags = 0x%04x\n",
                                                      Boss_self()->name, flags);
      Boss_flag_clear(&test_flag_grp, 0x0001); // 수신한 flag 수동 클리어
    }
    else
    {
      PRINTF("[%s] Timeout Flag Wait\n", Boss_self()->name);
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
  
  (void)Boss_task_create(bb_task, _BOSS_NULL, bb_stk, sizeof(bb_stk),
                                                              PRIO_2, "BB");

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
        ########## 실행 결과 ##########

            [AA TASK] Init 
            test_flag_grp Init
            [BB TASK] Init 
            
            [AA] (1) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (2) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (3) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (4) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (5) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (6) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
            
            [AA] (7) Boss_flag_send()
            [BB] Boss_flag_wait( OR ) flags = 0x0001
*/
