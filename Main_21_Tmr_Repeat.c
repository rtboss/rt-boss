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



boss_tmr_t timer1 = {0,};   // Ÿ�̸� ������ "0"���� �ʱ�ȭ


/*===========================================================================
    T I M E R 1 _ C A L L B A C K
---------------------------------------------------------------------------*/
void timer1_callback(boss_tmr_t *p_tmr)
{
  static int timer1_cnt = 0;
  
  ++timer1_cnt;

  if(timer1_cnt < 10) {
    Boss_tmr_start(p_tmr, 3*1000 /*ms*/, timer1_callback);
    PRINTF("(%d) timer1_callback() �ݺ�\n", timer1_cnt);
  } else {
    PRINTF("(%d) timer1_callback() �ݺ� ��\n", timer1_cnt);
  }
}


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
  
  PRINTF("Timer1 ���\n");

  Boss_tmr_start(&timer1, 3*1000 /*ms*/, timer1_callback); // 3���� timer1_callback ����
  
  for(;;)
  {    
    Boss_sleep(500);  /* 500ms */
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
  return 0;
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

  Boss_device_init();
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
���� : Ÿ�̸�(timer1)�� ������� 3�ʸ��� 10ȸ �ݺ� ����


### ������ ###

  Timer1 ���
  (1) timer1_callback() �ݺ�
  (2) timer1_callback() �ݺ�
  (3) timer1_callback() �ݺ�
  (4) timer1_callback() �ݺ�
  (5) timer1_callback() �ݺ�
  (6) timer1_callback() �ݺ�
  (7) timer1_callback() �ݺ�
  (8) timer1_callback() �ݺ�
  (9) timer1_callback() �ݺ�
  (10) timer1_callback() �ݺ� ��
    
*/

