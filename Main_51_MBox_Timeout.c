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

boss_mbox_q_t aa_mbox_q;
/*===============================================
    A A _ M A I N
-----------------------------------------------*/
int aa_main(void *p_arg)
{  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);

  Boss_mbox_q_init(&aa_mbox_q, &aa_tcb, SIG_00_BIT);
  
  for(;;)
  {
    boss_sigs_t sigs = Boss_wait(SIG_00_BIT);

    if(sigs & SIG_00_BIT)
    {
      int *mbox_add_arg;

      while( _BOSS_NULL != (mbox_add_arg = Boss_mbox_receive(&aa_mbox_q)) )
      {
        int result;
        
        Boss_sleep(10);     // 메일박스 타임아웃 테스트

        result = mbox_add_arg[0] + mbox_add_arg[1];
        
        //PRINTF("%d, %d \n", mbox_add_arg[0], mbox_add_arg[1]);
        
        Boss_mbox_done(mbox_add_arg, result);
      }
    }
  }
  
  return 0;
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
  
  PRINTF("[%s TASK] Init \n", Boss_self()->name);
  
  for(;;)
  {
    int         *mbox_add_arg;
    boss_uptr_t rsp;

    ++bb_count;
    PRINTF("\n");
    
    /*---[Timeout 없음(무한대기)] -----------------------*/    
    Boss_sleep(1 * 1000);  /* 1 Sec */
    mbox_add_arg = Boss_mbox_alloc(sizeof(int) * 2);

    mbox_add_arg[0] = 0;
    mbox_add_arg[1] = bb_count;

    if( _BOSS_SUCCESS == Boss_mbox_pend(&aa_mbox_q, mbox_add_arg, &rsp, 0) ) {
        PRINTF("Timeout( 0  ) rsp = %d  \n", rsp);
    } else {
        PRINTF("Timeout( 0  ) [타임 아웃] \n");
    }
    
    /*---[5ms Timeout] ----------------------------------*/    
    Boss_sleep(1 * 1000);  /* 1 Sec */
    mbox_add_arg = Boss_mbox_alloc(sizeof(int) * 2);

    mbox_add_arg[0] = 5000;
    mbox_add_arg[1] = bb_count;

    if( _BOSS_SUCCESS == Boss_mbox_pend(&aa_mbox_q, mbox_add_arg, &rsp, 5) ) {
        PRINTF("Timeout( 5ms) rsp = %d  \n", rsp);
    } else {
        PRINTF("Timeout( 5ms) [타임 아웃] \n");
    }

    /*---[5ms Timeout] ----------------------------------*/    
    Boss_sleep(1 * 1000);  /* 1 Sec */
    mbox_add_arg = Boss_mbox_alloc(sizeof(int) * 2);
    
    mbox_add_arg[0] = 20000;
    mbox_add_arg[1] = bb_count;
    
    if( _BOSS_SUCCESS == Boss_mbox_pend(&aa_mbox_q, mbox_add_arg, &rsp, 20) ) {
        PRINTF("Timeout(20ms) rsp = %d  \n", rsp);
    } else {
        PRINTF("Timeout(20ms) [타임 아웃] \n");
    }

    Boss_sleep(5 * 1000);  /* 5 Sec */
  }
  
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
  
  Boss_task_create( bb_main, _BOSS_NULL, &bb_tcb, BB_PRIO_2,
                    (boss_stk_t *)bb_stk, sizeof(bb_stk), "BB" );

  Boss_device_init();
  Boss_start();               /* Boss Scheduling Start */
  
  BOSS_ASSERT(_BOSS_FALSE);   /* Invalid */
  return 0;
}


/*

요약 : 메일박스 처리시간이 10ms일때, 타임아웃 시간을 무한대기(0), 5ms, 20ms로
       변경하여 메일박스 타임아웃 테스트 예제.


        ########## 실행 결과 ##########

            [AA TASK] Init 
            [BB TASK] Init 
            
            Timeout( 0  ) rsp = 1  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20001  
            
            Timeout( 0  ) rsp = 2  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20002  
            
            Timeout( 0  ) rsp = 3  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20003  
            
            Timeout( 0  ) rsp = 4  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20004  
            
            Timeout( 0  ) rsp = 5  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20005  
            
            Timeout( 0  ) rsp = 6  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20006  
            
            Timeout( 0  ) rsp = 7  
            Timeout( 5ms) [타임 아웃] 
            Timeout(20ms) rsp = 20007

            --- 중략 ---
*/
