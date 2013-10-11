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
    boss_u16_t flags = Boss_flag_wait(&test_flag_grp, FLAG_01,
                            _FLAG_OPT_OR + _FLAG_OPT_CONSUME, 20*1000/*20초*/);
    if(flags != 0)
    {
      PRINTF("[%s] Boss_flag_wait(OR + CONSUME) flags = 0x%04x\n",
                                                      Boss_self()->name, flags);
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

  {
    int idx = 0;
    boss_prio_t prio = PRIO_3;
  
    for(idx=0; idx < CX_TASK_MAX; idx++)
    {
      char name[10];
      sprintf(name, "C%02d", idx + 1);
      
      //prio++;     /* 우선순위 및 타임아웃 테스트 */
      
      (void)Boss_task_create( cx_task,              /* Task Entry Point     */
                              (void *)idx,          /* Task Argument        */
                              cx_stk[idx],          /* Stack Point (Base)   */
                              sizeof(cx_stk[idx]),  /* Stack Size (Bytes)   */
                              prio,                 /* Priority             */
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
            test_flag_grp Init
            [C01 TASK] Init 
            [C02 TASK] Init 
            [C03 TASK] Init 
            [C04 TASK] Init 
            [C05 TASK] Init 
            
            [AA] (1) Boss_flag_send()
            [C01] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (2) Boss_flag_send()
            [C02] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (3) Boss_flag_send()
            [C03] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (4) Boss_flag_send()
            [C04] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (5) Boss_flag_send()
            [C05] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (6) Boss_flag_send()
            [C01] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (7) Boss_flag_send()
            [C02] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (8) Boss_flag_send()
            [C03] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (9) Boss_flag_send()
            [C04] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (10) Boss_flag_send()
            [C05] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (11) Boss_flag_send()
            [C01] Boss_flag_wait(OR + CONSUME) flags = 0x0001
            
            [AA] (12) Boss_flag_send()
            [C02] Boss_flag_wait(OR + CONSUME) flags = 0x0001

            ------- 중략 -------

*/
