/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                      RT-BOSS PORT [ ATmega GCC(WinAVR) ]                    *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/
/*===========================================================================*/
/*                        [[ ATmega 스택 초기 구성 ]]                        */
/*---------------------------------------------------------------------------*/
/*
[ _Boss_stk_init() ]
                  |------------|
                  | 하위  번지 |
                  |############|------------------------+
                  | 0xEEEEEEEE | <- sp_base[0]          |
                  | 0xEEEEEEEE | <- sp_base[1]          |
                  |============|                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |    0xEE    |                        |
                  |------------|                        |
    리턴 "SP" ->  |    0xEE    |                        |
                  |============|                        |
                  |     R31    |                        |
                  |------------|                        |
                  |     R30    |                        |
                  |------------|                        |
                  |     R29    |                        |
                  |------------|                        |
                  |     R28    |                      size
                  |------------|                   (스택 크기)
                  |     R27    |                        |
                  |------------|                        |
                  |     R26    |                        |
                  |------------|                        |
                  |R25 p_arg(H)|                        |
                  |------------|                        |
                  |R24 p_arg(L)|                        |
                  |------------|                        |
                  |     R23    |                        |
                  |      ~     |                        |
                  |     R4     |                        |
                  |------------|                        |
                  |     R3     |                        |
                  |------------|                        |
                  |     R2     |                        |
                  |------------|                        |
                  |     R1     |                        |
                  |------------|                        |
                  |    SREG    |                        |
                  |------------|                        |
                  |     R0     |                        |
                  |++++++++++++|                        |
                  | PC Entry(H)|                        |
                  |------------|                        |
                  | PC Entry(L)|                        |
                  |++++++++++++|                        |
                  |  PC exit(H)|                        |
                  |------------|                        |
                  |  PC exit(L)|                        |
                  |============|                        |
                  |    0xEE    | <-  sp_limit[-2]       |
                  |    0xEE    | <-  sp_limit[-1]       |
                  |############|------------------------+
                  | 상위  번지 | <- sp_limit == sp_base[size]
                  |------------|
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include "Boss.h"

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
#define ALL_REGISTER_PUSH() 	              \
	asm volatile ( "push	r0						\n\t"	\
					"in		r0, __SREG__			\n\t"	\
					"cli							\n\t"	\
					"push	r0						\n\t"	\
					"push	r1						\n\t"	\
					"clr	r1						\n\t"	\
					"push	r2						\n\t"	\
					"push	r3						\n\t"	\
					"push	r4						\n\t"	\
					"push	r5						\n\t"	\
					"push	r6						\n\t"	\
					"push	r7						\n\t"	\
					"push	r8						\n\t"	\
					"push	r9						\n\t"	\
					"push	r10						\n\t"	\
					"push	r11						\n\t"	\
					"push	r12						\n\t"	\
					"push	r13						\n\t"	\
					"push	r14						\n\t"	\
					"push	r15						\n\t"	\
					"push	r16						\n\t"	\
					"push	r17						\n\t"	\
					"push	r18						\n\t"	\
					"push	r19						\n\t"	\
					"push	r20						\n\t"	\
					"push	r21						\n\t"	\
					"push	r22						\n\t"	\
					"push	r23						\n\t"	\
					"push	r24						\n\t"	\
					"push	r25						\n\t"	\
					"push	r26						\n\t"	\
					"push	r27						\n\t"	\
					"push	r28						\n\t"	\
					"push	r29						\n\t"	\
					"push	r30						\n\t"	\
					"push	r31						\n\t"	\
				);


#define ALL_REGISTER_POP()							  \
	asm volatile ( "pop	r31						\n\t"	\
					"pop	r30						\n\t"	\
					"pop	r29						\n\t"	\
					"pop	r28						\n\t"	\
					"pop	r27						\n\t"	\
					"pop	r26						\n\t"	\
					"pop	r25						\n\t"	\
					"pop	r24						\n\t"	\
					"pop	r23						\n\t"	\
					"pop	r22						\n\t"	\
					"pop	r21						\n\t"	\
					"pop	r20						\n\t"	\
					"pop	r19						\n\t"	\
					"pop	r18						\n\t"	\
					"pop	r17						\n\t"	\
					"pop	r16						\n\t"	\
					"pop	r15						\n\t"	\
					"pop	r14						\n\t"	\
					"pop	r13						\n\t"	\
					"pop	r12						\n\t"	\
					"pop	r11						\n\t"	\
					"pop	r10						\n\t"	\
					"pop	r9						\n\t"	\
					"pop	r8						\n\t"	\
					"pop	r7						\n\t"	\
					"pop	r6						\n\t"	\
					"pop	r5						\n\t"	\
					"pop	r4						\n\t"	\
					"pop	r3						\n\t"	\
					"pop	r2						\n\t"	\
					"pop	r1						\n\t"	\
					"pop	r0						\n\t"	\
					"out	__SREG__, r0			\n\t"	\
					"pop	r0						\n\t"	\
				);



/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/
boss_reg_t  _isr_nesting = 0;


/*===========================================================================
    _ B O S S _ S T K _ I N I T
---------------------------------------------------------------------------*/
boss_stk_t *_Boss_stk_init( void (*task)(void *p_arg), void *p_arg,
                                boss_stk_t *sp_base,  boss_uptr_t stk_bytes)
{
  boss_uptr_t tmp;
  boss_uptr_t size  = stk_bytes / sizeof(boss_stk_t);
  boss_stk_t  *sp   = &sp_base[size-1];   /* ED(Empty Descending) Stack */
  
  #ifdef _BOSS_SPY_ 
  boss_uptr_t i;
  for(i = 0; i < size; i++) {
    sp_base[i] = (boss_stk_t)0xEE;      // 스택 [E] empty
  }
  sp = sp - 2;
  #endif

  tmp = (boss_uptr_t)_Boss_task_exit;
  *sp = (boss_stk_t)tmp;          /* PC(L) : Task Exit Point */
  sp--;
  *sp = (boss_stk_t)(tmp >> 8);   /* PC(H) */
  sp--;
  
  tmp = (boss_uptr_t)task;
  *sp = (boss_stk_t)tmp;          /* PC(L) : Task Entry Point */
  sp--;
  *sp = (boss_stk_t)(tmp >> 8);   /* PC(H) */
  sp--;
  
  *sp = 0x00;   sp--;       /* R0 */
  *sp = 0x80;   sp--;       /* SREG : 0x80 = 인터럽트 활성화 */
  *sp = 0x00;   sp--;       /* R1 : Zero register */
  *sp = 0x02;   sp--;       /* R2 */
  *sp = 0x03;   sp--;       /* R3 */
  *sp = 0x04;   sp--;       /* R4 */
  *sp = 0x05;   sp--;       /* R5 */
  *sp = 0x06;   sp--;       /* R6 */
  *sp = 0x07;   sp--;       /* R7 */
  *sp = 0x08;   sp--;       /* R8 */
  *sp = 0x09;   sp--;       /* R9 */
  *sp = 0x10;   sp--;       /* R10 */
  *sp = 0x11;   sp--;       /* R11 */
  *sp = 0x12;   sp--;       /* R12 */
  *sp = 0x13;   sp--;       /* R13 */
  *sp = 0x14;   sp--;       /* R14 */
  *sp = 0x15;   sp--;       /* R15 */
  *sp = 0x16;   sp--;       /* R16 */
  *sp = 0x17;   sp--;       /* R17 */
  *sp = 0x18;   sp--;       /* R18 */
  *sp = 0x19;   sp--;       /* R19 */
  *sp = 0x20;   sp--;       /* R20 */
  *sp = 0x21;   sp--;       /* R21 */
  *sp = 0x22;   sp--;       /* R22 */
  *sp = 0x23;   sp--;       /* R23 */
  
  tmp = (boss_uptr_t)p_arg;
  *sp = (boss_stk_t)tmp;          /* R24 : p_arg(L) */
  sp--;
  *sp = (boss_stk_t)(tmp >> 8);   /* R25 : p_arg(H) */
  sp--;
  
  *sp = 0x26;   sp--;       /* R26 */
  *sp = 0x27;   sp--;       /* R27 */
  *sp = 0x28;   sp--;       /* R28 */
  *sp = 0x29;   sp--;       /* R29 */
  *sp = 0x30;   sp--;       /* R30 */
  *sp = 0x31;   sp--;       /* R31 */

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
void _Boss_start_schedule(void) __attribute__ ( ( naked ) );
void _Boss_start_schedule(void)
{
  asm volatile("call  _Boss_start_tcb_sp");   /* 리턴값 : "R25:R24"는 start_tcb_sp */
  
  asm volatile("out   __SP_H__, R25");
  asm volatile("out   __SP_L__, R24");
  ALL_REGISTER_POP();
  
  asm volatile("ret");
}


/*===========================================================================
    _ B O S S _ C O N T E X T _ S W I T C H
---------------------------------------------------------------------------*/
extern boss_stk_t *_Boss_switch_current_tcb(boss_stk_t *cur_task_sp);
void _Boss_context_switch(void) __attribute__ ( ( naked ) );
void _Boss_context_switch(void)
{
  ALL_REGISTER_PUSH();
  asm volatile("in    R25, __SP_H__");            
  asm volatile("in    R24, __SP_L__");            /* 매개변수 : "R25:R24"는 실행중인 태스크 스택 포인터  */
  
  asm volatile("call  _Boss_switch_current_tcb"); /* void *_Boss_switch_current_tcb(void *cur_task_sp) 호출*/
  
  asm volatile("out   __SP_H__, R25");            /* 리턴값   : "R25:R24"는 실행할 태스크 스택 포인터 */
  asm volatile("out   __SP_L__, R24");
  ALL_REGISTER_POP();
  
  asm volatile("ret");
}


/*===========================================================================
    _ M C U _ I S R _ B E G I N
---------------------------------------------------------------------------*/
void _mcu_isr_begin(void)
{
  BOSS_ASSERT(_isr_nesting < 35);
  
  _isr_nesting++;
}


/*===========================================================================
    _ M C U _ I S R _ F I N I S
---------------------------------------------------------------------------*/
void _mcu_isr_finis(void)
{
  BOSS_ASSERT(_isr_nesting != 0);
  
  _isr_nesting--;
}


/*===========================================================================
    _ M C U _ I S R _
---------------------------------------------------------------------------*/
boss_reg_t _mcu_isr_(void)
{
  /* ISR (Interrupt Service Routine) 실행 상태 */
  
  return _isr_nesting;
}

