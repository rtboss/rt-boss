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

boss_flag_obj_t test_flag_obj;


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

  PRINTF("test_flag_obj Init\n");
  Boss_flag_obj_init(&test_flag_obj);

  Boss_sleep(100); // TASK init wait
  
  for(;;)
  {
    static int param_count = 0;

    Boss_sleep(2000);    
    param_count++;
    
    PRINTF("\n[%s] (%d) Boss_flag_send()\n", Boss_self()->name, param_count);
    Boss_flag_send(&test_flag_obj, 0x0001);
  }
  
  return 0;   // Task Exit
}


/*===========================================================================
    [ C x _ T A S K ]
---------------------------------------------------------------------------*/
#define CX_TASK_MAX   5

boss_tcb_t    cx_tcb[CX_TASK_MAX];
boss_stk_t    cx_stk[CX_TASK_MAX][ 512 / sizeof(boss_stk_t) ];   /* 256 bytes */

int cx_task(void *p_arg)
{
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  Boss_sleep(100); // TASK init wait

  for(;;)
  {
    boss_u16_t flags;
    flags = Boss_flag_wait(&test_flag_obj, _FLAG_OPT_OR + _FLAG_OPT_CONSUME,
                                                      0x0001, 20*1000/*20초*/);
    if(flags == 0) {
      PRINTF("[%s] Timeout Flag Wait\n", Boss_self()->name);
    } else {
      PRINTF("[%s] Boss_flag_wait(OR + CONSUME) flags = 0x%04x\n",
                                                      Boss_self()->name, flags);
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

  {
    int idx = 0;
    boss_prio_t prio = Cx_PRIO_3;
  
    for(idx=0; idx < CX_TASK_MAX; idx++)
    {
      char name[10];
      sprintf(name, "C%02d", idx + 1);
      
      //prio++;     /* 우선순위 및 타임아웃 테스트 */
      
      Boss_task_create( cx_task,              /* Task Entry Point       */
                      (void *)idx,            /* Task Argument          */
                      &cx_tcb[idx],           /* TCB(Task Control Block)*/
                      prio,                   /* Priority               */
                      cx_stk[idx],            /* Stack Point (Base)     */
                      sizeof(cx_stk[idx]),    /* Stack Size (Bytes)     */
                      name                    /* Task Name String       */
                      );
    }
  }

  Boss_device_init();
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
        ########## 실행 결과 ##########

            [AA TASK] Init 
            test_flag_obj Init
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
