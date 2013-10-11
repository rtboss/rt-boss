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
    Boss_tmr_start(p_tmr, 3*1000 /*ms*/, timer1_callback);  // ���� (�ݺ�����)
    PRINTF("(%d) timer1_callback() �ݺ�\n", timer1_cnt);
  } else {
    PRINTF("(%d) timer1_callback() �ݺ� ��\n", timer1_cnt);
  }
}


/*===========================================================================
    A A _ T A S K
---------------------------------------------------------------------------*/
boss_stk_t aa_stk[ 512 / sizeof(boss_stk_t)];

int aa_task(void *p_arg)
{  
  PRINTF("Timer1 ���\n");

  Boss_tmr_start(&timer1, 3*1000 /*ms*/, timer1_callback); // 3���� timer1_callback ����
  
  for(;;)
  {
    Boss_sleep(500);  /* 500ms */
  }
  
  PRINTF("[%s TASK] Exit \n", Boss_self()->name);
  
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

  Boss_device_init();         /* Ÿ�̸� �ʱ�ȭ */
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*
���� : Ÿ�̸�(timer1)�� ������� 3�ʸ��� 10ȸ �ݺ� ����


########## ���� ��� ##########

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

