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
    [ A A _ T A S K ]
---------------------------------------------------------------------------*/
boss_tcb_t    aa_tcb;
boss_align_t  aa_stk[ 512 / sizeof(boss_align_t) ];         /* 512 bytes */

/*===============================================
    A A _ M A I N
-----------------------------------------------*/
int aa_main(void *p_arg)
{  
  int aa_count = 0;
  
  PRINTF("[AA_TASK] Init \n");
  
  for(;;)
  {    
    Boss_sleep(1000);  /* 1000ms (1초) */
    
    PRINTF(" [AA_TASK] (1초마다) count = %d \n", ++aa_count);

    if(100 <= aa_count)
    {
      break;
    }
  }
  
  PRINTF("[AA_TASK] Exit \n");
  
  return 0;       // 테스크 종료
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
  
  PRINTF("[BB_TASK] Init \n");
  
  for(;;)
  {
    Boss_sleep(10 * 1000);   /* 10 Sec */

    PRINTF("----- [BB_TASK (10초마다)] ----- count = %d \n", ++bb_count);
    
    if(10 <= bb_count)
    {
      break;
    }
  }

  PRINTF("[BB_TASK] Exit \n");
  
  return 0;       // 테스크 종료
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
                    AA_PRIO_1,            /* 우선순위               */
                    (boss_stk_t *)aa_stk, /* 스택 포인터(base)      */
                    sizeof(aa_stk),       /* 스택 크기(Bytes)       */
                    "AA"                  /* 테스크 이름            */
                    );
  
  Boss_task_create( bb_main, _BOSS_NULL, &bb_tcb, BB_PRIO_2,
                    (boss_stk_t *)bb_stk, sizeof(bb_stk), "BB" );

  Boss_device_init();         /* 타이머 초기화 */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
요약 : 2개의 테스크 AA테스크와 BB테스크를 생성하여 
       AA테스크는 1초마다 메시지를 출력하고 100번 실행 후 종료
       BB테스크는 10초마다 메시지를 출력하고 10번 실행 후 종료.

설명 : 테스크 생성은 Boss_task_create()로 생성하며
       테스크 종료는 테스크 실행중 "0"을 리턴하면 종료한다.(rtetur 0)

       #### 1. 테스크 생성 함수 ####
          void Boss_task_create(  int (*task)(void *p_arg), void *p_arg, 
                          boss_tcb_t *p_tcb, boss_prio_t prio, 
                          boss_stk_t *sp_base, boss_uptr_t stk_bytes,
                          const char *name )

            - *task  : 테스크 함수 포인터.
            - *p_arg : 테스크 실행시 전달값.
            - *p_tcb : 테스크의 정보를 가지고 있는 TCB
            - prio   : 우선순위 (1~254 까지 가능)
                        0 - 예약(현재 미사용)
                        1 - 가장 높은 우선순위
                      254 - 가장 낮은 우선순위  
                      255 - Idle테스크 우선수위 (Idle 테스크외 사용불가)
            - sp_base   : 스택 메모리의(base 주소)
            - stk_bytes : 스택 크기
            - name      : 테스크 이름

      #### 2. 테스크 종료 ####
        int aa_main(void *p_arg)
        {
          // 테스크 초기화
          
          for(;;) {
           // 테스크 실행 코드
          }

          // 테스크 종료 코드
          return 0;       // 테스크 종료
        }





### 실행결과 ###

  [AA_TASK] Init 
  [BB_TASK] Init 
   [AA_TASK] (1초마다) count = 1 
   [AA_TASK] (1초마다) count = 2 
   [AA_TASK] (1초마다) count = 3 
   [AA_TASK] (1초마다) count = 4 
   [AA_TASK] (1초마다) count = 5 
   [AA_TASK] (1초마다) count = 6 
   [AA_TASK] (1초마다) count = 7 
   [AA_TASK] (1초마다) count = 8 
   [AA_TASK] (1초마다) count = 9 
   [AA_TASK] (1초마다) count = 10 
  ----- [BB_TASK (10초마다)] ----- count = 1 
   [AA_TASK] (1초마다) count = 11 
   [AA_TASK] (1초마다) count = 12 
   [AA_TASK] (1초마다) count = 13 
   [AA_TASK] (1초마다) count = 14 
   [AA_TASK] (1초마다) count = 15 
   [AA_TASK] (1초마다) count = 16 
   [AA_TASK] (1초마다) count = 17 
   [AA_TASK] (1초마다) count = 18 
   [AA_TASK] (1초마다) count = 19 
   [AA_TASK] (1초마다) count = 20 
  ----- [BB_TASK (10초마다)] ----- count = 2 
   [AA_TASK] (1초마다) count = 21 
   [AA_TASK] (1초마다) count = 22 
   [AA_TASK] (1초마다) count = 23 
  
   --- 중략 ----
    
   [AA_TASK] (1초마다) count = 87 
   [AA_TASK] (1초마다) count = 88 
   [AA_TASK] (1초마다) count = 89 
   [AA_TASK] (1초마다) count = 90 
  ----- [BB_TASK (10초마다)] ----- count = 9 
   [AA_TASK] (1초마다) count = 91 
   [AA_TASK] (1초마다) count = 92 
   [AA_TASK] (1초마다) count = 93 
   [AA_TASK] (1초마다) count = 94 
   [AA_TASK] (1초마다) count = 95 
   [AA_TASK] (1초마다) count = 96 
   [AA_TASK] (1초마다) count = 97 
   [AA_TASK] (1초마다) count = 98 
   [AA_TASK] (1초마다) count = 99 
   [AA_TASK] (1초마다) count = 100 
  [AA_TASK] Exit 
  ----- [BB_TASK (10초마다)] ----- count = 10 
  [BB_TASK] Exit 

    
*/
