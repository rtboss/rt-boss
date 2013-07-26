/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                      RT-BOSS PORT [ Cortex-M4(F) IAR ]                      *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
/*===========================================================================*/
/*                            [[ 스택  구성 ]]                               */
/*---------------------------------------------------------------------------*/
/*
[ _Boss_stk_init() ]
                  |------------|
                  | 하위  번지 |
                  |############|------------------------+
                  | 0xEEEEEEEE | <- sp_base[0]          |
                  | 0xEEEEEEEE | <- sp_base[1]          |
                  |============|                        |
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
                  |------------|                        |
                  | EXC_RETURN | = 0xFFFFFFFD           |
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
                  |   LR(exit) |                        |
                  |------------|                        |
                  |  PC(Entry) |                        |
                  |------------|                        |
                  |     PSR    |                        |
                  |============|                        |
                  | 0xEEEEEEEE | <-  sp_limit[-2]       |
                  | 0xEEEEEEEE | <-  sp_limit[-1]       |
                  |############|------------------------+
                  | 상위  번지 | <- sp_limit == sp_base[size]
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

void _svc_call_0(void);

/*===========================================================================
    _ B O S S _ S T K _ I N I T
---------------------------------------------------------------------------*/
boss_stk_t *_Boss_stk_init( int (*task)(void *p_arg), void *p_arg,
                                boss_stk_t *sp_base,  boss_uptr_t stk_bytes)
{
  boss_uptr_t size  = stk_bytes / sizeof(boss_stk_t);
  boss_stk_t  *sp   = &sp_base[size]; /* FD(Full Descending) Stack */

  #ifdef _BOSS_SPY_ 
  boss_uptr_t i;
  for(i = 0; i < size; i++) {
    sp_base[i] = (boss_stk_t)0xEEEEEEEE;      // 스택 [E] empty
  }
  sp = sp - 2;
  #endif

  
  --sp;   *sp = 0x01000000L;                  /* PSR  */
  --sp;   *sp = (boss_stk_t)task;             /* PC : Task Entry Point    */
  --sp;   *sp = (boss_stk_t)_Boss_task_exit;  /* LR : Task Exit Function  */
  --sp;   *sp = 0x00000012L;                  /* R12  */
  --sp;   *sp = 0x00000003L;                  /* R3   */
  --sp;   *sp = 0x00000002L;                  /* R2   */
  --sp;   *sp = 0x00000001L;                  /* R1   */
  --sp;   *sp = (boss_stk_t)p_arg;            /* R0 : Argument */
  
  --sp;   *sp = 0xFFFFFFFD;                   /* EXC_RETURN : 0xFFFFFFFD    */
                                              /* 스레드 특근 모드, PSP 사용 */
  
  --sp;   *sp = 0x00000011L;                  /* R11  */
  --sp;   *sp = 0x00000010L;                  /* R10  */
  --sp;   *sp = 0x00000009L;                  /* R9   */
  --sp;   *sp = 0x00000008L;                  /* R8   */
  --sp;   *sp = 0x00000007L;                  /* R7   */
  --sp;   *sp = 0x00000006L;                  /* R6   */
  --sp;   *sp = 0x00000005L;                  /* R5   */
  --sp;   *sp = 0x00000004L;                  /* R4   */
  
  return sp;
}


/*===========================================================================
    _   B O S S _ S T A R T _ T C B _ S P
---------------------------------------------------------------------------*/
boss_stk_t *_Boss_start_tcb_sp(void)
{
  return (Boss_self()->sp);
}


/*===========================================================================
    _   B O S S _ S T A R T _ S C H E D U L E
---------------------------------------------------------------------------*/
void _Boss_start_schedule(void)
{
  /* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
  SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

  /*set ASPEN and LSPEN automatic preservation and restoration  */
  FPU->FPCCR |= (1UL << FPU_FPCCR_ASPEN_Pos) | (1UL << FPU_FPCCR_LSPEN_Pos);
#endif
  
  __set_MSP(*(__IO uint32_t*)SCB->VTOR);  /* MSP를 초기 스택(__initial_sp)으로 설정 */
  
  _svc_call_0();
}


/*===========================================================================
    _ B O S S _ C O N T E X T _ S W I T C H
---------------------------------------------------------------------------*/
void _Boss_context_switch(void)
{
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;   /* PendSV_Handler 호출 */
}



/*
** 아래의 함수는 "Boss_CPU_ARM_CM4F_IAR_asm.s" 참조
**
** void _svc_call_0(void);
** void SVC_Handler(void);
** void PendSV_Handler(void);
*/
