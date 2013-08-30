/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                       RT-BOSS PORT [ Cortex-M0 IAR]                         *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
/*===========================================================================*/
/*                            [[ ����  ���� ]]                               */
/*---------------------------------------------------------------------------*/
/*
[ _Boss_stk_init() ]
                  |------------|
                  | ����  ���� |
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
    ���� "SP" ->  |     R10    |                        |
                  |------------|                        |
                  |     R11    |                        |
                  |------------|                        |
                  |     R4     |                        |
                  |------------|                        |
                  |     R5     |                      size
                  |------------|                   (���� ũ��)
                  |     R6     |                        |
                  |------------|                        |
                  |     R7     |                        |
                  |------------|                        |
                  |     R8     |                        |
                  |------------|                        |
                  |     R9     |                        |
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
                  | ����  ���� | <- sp_limit == sp_base[size]
                  |------------|
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss.h"

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
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
    sp_base[i] = (boss_stk_t)0xEEEEEEEE;      // ���� [E] empty
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
  
  --sp;   *sp = 0x00000009L;                  /* R9   */
  --sp;   *sp = 0x00000008L;                  /* R8   */
  --sp;   *sp = 0x00000007L;                  /* R7   */
  --sp;   *sp = 0x00000006L;                  /* R6   */
  --sp;   *sp = 0x00000005L;                  /* R5   */
  --sp;   *sp = 0x00000004L;                  /* R4   */
  --sp;   *sp = 0x00000011L;                  /* R11  */
  --sp;   *sp = 0x00000010L;                  /* R10  */
  
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
  extern const boss_u32_t CSTACK$$Limit;

  
  __set_MSP((boss_u32_t)&CSTACK$$Limit);  /* MSP�� �ʱ� ����(__initial_sp)���� ���� */
  
  _svc_call_0();
}


/*===========================================================================
    _ B O S S _ C O N T E X T _ S W I T C H
---------------------------------------------------------------------------*/
void _Boss_context_switch(void)
{
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;   /* PendSV_Handler ȣ�� */
}



/*
** �Ʒ��� �Լ��� "Boss_CPU_ARM_CM0_IAR_asm.s" ����
**
** void _svc_call_0(void);
** void SVC_Handler(void);
** void PendSV_Handler(void);
*/
