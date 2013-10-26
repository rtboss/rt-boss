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
    T I M E R 1 _ C A L L B A C K
---------------------------------------------------------------------------*/
void timer1_callback(boss_tmr_t *p_tmr)
{
  static int timer1_cnt = 0;
  
  ++timer1_cnt;

  if(timer1_cnt < 10) {
    Boss_tmr_start(p_tmr, 3*1000 /*ms*/, timer1_callback);  // 재등록 (반복실행)
    PRINTF("(%d) timer1_callback() 반복\n", timer1_cnt);
  } else {
    PRINTF("(%d) timer1_callback() 타이머 반환(해제)\n", timer1_cnt);
    Boss_tmr_del(p_tmr);  // 타이머 반환(해제)
  }
}


/*===========================================================================
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{  
  PRINTF("Timer1 생성 및 등록\n");
  Boss_tmr_start( Boss_tmr_create(), 3*1000 /*ms*/, timer1_callback); // 3초후 timer1_callback 실행
  
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
설명 : 타이머를 등록한후 3초마다 10회 반복 실행


########## 실행 결과 ##########

  Timer1 생성 및 등록
  (1) timer1_callback() 반복
  (2) timer1_callback() 반복
  (3) timer1_callback() 반복
  (4) timer1_callback() 반복
  (5) timer1_callback() 반복
  (6) timer1_callback() 반복
  (7) timer1_callback() 반복
  (8) timer1_callback() 반복
  (9) timer1_callback() 반복
  (10) timer1_callback() 타이머 반환(해제)
    
*/

