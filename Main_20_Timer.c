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



boss_tmr_t timer1 = {0,};   // 타이머 선언후 "0"으로 초기화


/*===========================================================================
    T I M E R 1 _ C A L L B A C K
---------------------------------------------------------------------------*/
void timer1_callback(boss_tmr_t *p_tmr)
{
  PRINTF("timer1_callback() 실행\n");
}


/*===========================================================================
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{  
  PRINTF("Timer1 등록\n");

  Boss_tmr_start(&timer1, 3*1000 /*ms*/, timer1_callback); // 3초후 timer1_callback 실행
  
  for(;;)
  {
    Boss_sleep(500);  /* 500ms */
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
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

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
설명 : 타이머(timer1)을 등록한후 3초후 콜백함수 실행


########## 실행 결과 ##########

  Timer1 등록
  timer1_callback() 실행
    
*/

