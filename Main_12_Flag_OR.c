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
    
    ++aa_count;
    
    PRINTF("\n[%s] (%d) Boss_flag_send( FLAG_01 )\n", Boss_self()->name, aa_count);
    Boss_flag_send(&test_flag_grp, FLAG_01);

    Boss_sleep(1000);
    PRINTF("\n[%s] (%d) Boss_flag_send( FLAG_02 )\n", Boss_self()->name, aa_count);
    Boss_flag_send(&test_flag_grp, FLAG_02);

    Boss_sleep(1000);
    PRINTF("\n[%s] (%d) Boss_flag_send( FLAG_03 )\n", Boss_self()->name, aa_count);
    Boss_flag_send(&test_flag_grp, FLAG_03);
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
    boss_flags_t flags = Boss_flag_wait(&test_flag_grp, FLAG_01 | FLAG_02 | FLAG_03,
                                _FLAG_OPT_OR + _FLAG_OPT_CONSUME, 20*1000/*20초*/);
    
    if(flags != 0)
    {
      PRINTF("[%s] Boss_flag_wait(OR + CONSUME) flags = 0x%04x\n",
                                                      Boss_self()->name, flags);
      if( flags & FLAG_01 ) {
        PRINTF(" FLAG_01\n");
      }

      if( flags & FLAG_02 ) {
        PRINTF(" FLAG_02\n");
      }

      if( flags & FLAG_03 ) {
        PRINTF(" FLAG_03\n");
      }
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
            
            [AA] (1) Boss_flag_send( FLAG_01 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0001
             FLAG_01
            
            [AA] (1) Boss_flag_send( FLAG_02 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0002
             FLAG_02
            
            [AA] (1) Boss_flag_send( FLAG_03 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0004
             FLAG_03
            
            [AA] (2) Boss_flag_send( FLAG_01 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0001
             FLAG_01
            
            [AA] (2) Boss_flag_send( FLAG_02 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0002
             FLAG_02
            
            [AA] (2) Boss_flag_send( FLAG_03 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0004
             FLAG_03
            
            [AA] (3) Boss_flag_send( FLAG_01 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0001
             FLAG_01
            
            [AA] (3) Boss_flag_send( FLAG_02 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0002
             FLAG_02
            
            [AA] (3) Boss_flag_send( FLAG_03 )
            [BB] Boss_flag_wait(OR + CONSUME) flags = 0x0004
             FLAG_03

             ...

*/
