/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                       RT-BOSS PORT [ Cortex-M3 Keil]                        *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
/*===========================================================================*/
/*                      [[ Cortex-M3 스택 초기 구성 ]]                       */
/*---------------------------------------------------------------------------*/
/*
[ _Boss_stk_init() ]
                  |------------|
                  | 하위  번지 |
                  |############|------------------------+
                  | 0xEEEEEEEE | <- sp_base             |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  | 0xEEEEEEEE |                        |
                  |============|                        |
    리턴 "SP" ->  |     R4     |                        |
                  |------------|                        |
                  |     R5     |                        |
                  |------------|                        |
                  |     R6     |                        |
                  |------------|                        |
                  |     R7     |                      size
                  |------------|                   (스택 크기)
                  |     R8     |                        |
                  |------------|                        |
                  |     R9     |                        |
                  |------------|                        |
                  |     R10    |                        |
                  |------------|                        |
                  |     R11    |                        |
                  |============|                        |
                  |  R0(p_arg) |                        |
                  |------------|                        |
                  |     R1     |                        |
                  |------------|                        |
                  |     R2     |                        |
                  |------------|                        |
                  |     R3     |                        |
                  |------------|                        |
                  |     R12    |                        |
                  |------------|                        |
                  |     LR     |                        |
                  |------------|                        |
                  |  PC(Entry) |                        |
                  |------------|                        |
                  |     PSR    |                        |
                  |############|------------------------+
                  | 상위  번지 | <- sp_limit
                  |------------|
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss.h"

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_task_exit(int exit_code);


/*===========================================================================
    _ B O S S _ S T K _ I N I T
---------------------------------------------------------------------------*/
boss_stk_t *_Boss_stk_init( int (*task)(void *p_arg), void *p_arg,
                                boss_stk_t *sp_base,  boss_uptr_t stk_bytes)
{
  boss_uptr_t size  = stk_bytes / sizeof(boss_stk_t);
  boss_stk_t  *sp   = &sp_base[size]; /* FD(Full Descending) Stack */
  
  --sp;   *sp = 0x01000000L;          /* PSR  */
  --sp;   *sp = (boss_stk_t)task;     /* PC : Task Entry Point */
  --sp;   *sp = (boss_stk_t)_Boss_task_exit;  /* LR   */
  --sp;   *sp = 0x00000012L;          /* R12  */
  --sp;   *sp = 0x00000003L;          /* R3   */
  --sp;   *sp = 0x00000002L;          /* R2   */
  --sp;   *sp = 0x00000001L;          /* R1   */
  --sp;   *sp = (boss_stk_t)p_arg;    /* R0 : Argument */
  --sp;   *sp = 0x00000011L;          /* R11  */
  --sp;   *sp = 0x00000010L;          /* R10  */
  --sp;   *sp = 0x00000009L;          /* R9   */
  --sp;   *sp = 0x00000008L;          /* R8   */
  --sp;   *sp = 0x00000007L;          /* R7   */
  --sp;   *sp = 0x00000006L;          /* R6   */
  --sp;   *sp = 0x00000005L;          /* R5   */
  --sp;   *sp = 0x00000004L;          /* R4   */
  
  return sp;
}


/*===========================================================================
    _   B O S S _ S T A R T _ S C H E D U L E
---------------------------------------------------------------------------*/
__ASM void _Boss_start_schedule(void)
{
  SVC     0         /* SVC 0 호출 (SVC_Handler() 실행)  */
}


/*===========================================================================
    S   V   C _   H A N D L E R
---------------------------------------------------------------------------*/
__ASM void SVC_Handler(void)
{
  IMPORT  _Boss_start_tcb_sp

  CPSID   I                   /* [ IRQ 비활성화 ] */
  
  LDR     R0, =0xE000ED08     /* NVIC Vector Table Offset Register  */
  LDR     R0, [R0]            /* Vector Table 시작 번지             */
  LDR     R0, [R0, #0]        /* 최기 스택 (__initial_sp)           */
  MSR     MSP, R0             /* "MSP"를 초기 스택 값으로 초기화    */

  BL      _Boss_start_tcb_sp  /* 리턴값 : "R0"는 start_tcb_sp       */
  
  LDMIA   R0!, {R4-R11}
  MSR     PSP, R0

  LDR     LR, =0xFFFFFFFD     /* 스레드 특근 모드, PSP 사용         */
  CPSIE   I                   /* [ IRQ 활성화 ]   */
  BX      LR                  /* 리턴 SVC (R0-R3, R12, PC, PSR 복원)*/

  ALIGN
}


/*===========================================================================
    _ B O S S _ C O N T E X T _ S W I T C H
---------------------------------------------------------------------------*/
void _Boss_context_switch(void)
{
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;   /* PendSV_Handler 호출 */
}


/*===========================================================================
    P E N D S V _ H A N D L E R
---------------------------------------------------------------------------*/
__ASM void PendSV_Handler(void)
{
  IMPORT  _Boss_switch_current_tcb

  MRS     R0, PSP         // R0-R3, R12, PC, PSR 저장되어 있음
  STMDB   R0!, {R4-R11}   // R4-R11 저장
  MOV     R4, LR          // LR 임시저장 (BL 사용을 위해)

  CPSID   I                   /* [ IRQ 비활성화 ] */
  /*
  ** void *_Boss_switch_current_tcb(void *cur_task_sp)
  ** 매개변수 : "R0"는 실행중인 태스크 스택 포인터
  ** 리턴값   : "R0"는 실행할 태스크 스택 포인터
  */
  BL      _Boss_switch_current_tcb

  CPSIE   I                   /* [ IRQ 활성화 ]   */

  MOV     LR, R4          // LR 임시저장 (복원)
  LDMIA   R0!, {R4-R11}   // R4-R11 복원
  MSR     PSP, R0
  
  BX      LR              // 리턴 PendSV (R0-R3, R12, PC, PSR 복원)

  ALIGN
}

