#ifndef _BOSS_CONF_H_
#define _BOSS_CONF_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                 RT-BOSS (Config File)         [ ATmega128 ]                 *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                            RT-BOSS 데이터형                               */
/*---------------------------------------------------------------------------*/
typedef unsigned char       boss_u08_t;       /* unsigned  8bit 데이터형 */
typedef unsigned int        boss_u16_t;       /* unsigned 16bit 데이터형 */
typedef unsigned long int   boss_u32_t;       /* unsigned 32bit 데이터형 */
typedef unsigned long long  boss_u64_t;       /* unsigned 64bit 데이터형 */

//typedef boss_u08_t          boss_byte_t;      /* Byte                 */
typedef boss_u08_t          boss_reg_t;       /* MCU 레지스터 크기      */
typedef boss_u16_t          boss_uptr_t;      /* unsigned 포인터 크기   */
typedef boss_u08_t          boss_stk_t;       /* 스택                   */

typedef boss_u16_t          boss_sigs_t;      /* 시그널                 */
typedef boss_u32_t          boss_tmr_ms_t;    /* 타이머 카운트(ms)      */

typedef boss_u08_t          boss_align_t;     /* 메모리 정렬 (1byte)     */


/*===========================================================================*/
/*                           RT-BOSS 사용자 설정                             */
/*---------------------------------------------------------------------------*/
#define _BOSS_TICK_MS_          10      /* Tick (ms)          */
#define _BOSS_RR_QUANTUM_MS     50      /* RR(Round Robin) Sched Quantum Time(ms) */
#define _BOSS_TCB_NAME_SIZE     6       /* TCB Name           */

#define _BOSS_MEM_POOL_SIZE     1024    /* Bytes              */
#define _BOSS_MEM_INFO_                 /* 메모리 디버거 정보 */

/*===========================================================================*/
/*                           TCB 확장(Extension)                             */
/*---------------------------------------------------------------------------*/
#define _BOSS_TCB_EXTEND_

#ifdef _BOSS_TCB_EXTEND_
#define _BOSS_SPY_                      /* Stack, CPU, Context Switch */

typedef struct {
  #ifdef _BOSS_SPY_
    boss_stk_t    *sp_base;
    boss_stk_t    *sp_peak;
    boss_stk_t    *sp_limit;
  
    boss_u32_t    run_time;     /* Task run-time sum (us) */
    boss_u32_t    context;      /* Context Switch Number  */
  #endif
} _boss_tcb_ex_t;
#endif /* _BOSS_TCB_EXTEND_ */

/*===========================================================================*/
/*   IRQ (Interrupt request) / ISR (Interrupt Service Routine)               */
/*---------------------------------------------------------------------------*/
#define BOSS_IRQ_DISABLE_SR( _irq_storage_ )  \
                    do { _irq_storage_ = SREG; asm volatile("cli"); } while(0)
            
#define BOSS_IRQ_RESTORE_SR( _irq_storage_ )  \
                                        do { SREG = _irq_storage_; } while(0)

/*----------------------------------------------------------------------*/
#define BOSS_IRQ_DISABLE()    do { \
                                boss_reg_t _irq_storage_;           \
                                BOSS_IRQ_DISABLE_SR(_irq_storage_)

#define BOSS_IRQ_RESTORE()      BOSS_IRQ_RESTORE_SR(_irq_storage_); \
                              } while(0)

/*----------------------------------------------------------------------*/
#define _BOSS_IRQ_()    ( (SREG & (1 << SREG_I)) ? 0 : !0 ) /* 0 = Enable / !0 = Disable */

#define _BOSS_ISR_()    _mcu_isr_()    /* !0 = ISR Active  */

/*----------------------------------------------------------------------*/
#define _BOSS_ISR_BEGIN()   do { _Boss_sched_lock(); _mcu_isr_begin()
#define _BOSS_ISR_FINIS()   _mcu_isr_finis(); _Boss_sched_free(); } while(0)

boss_reg_t _mcu_isr_(void);
void _mcu_isr_begin(void);
void _mcu_isr_finis(void);


/*===========================================================================*/
/*                             태스크 우선순위                               */
/*---------------------------------------------------------------------------*/
typedef enum {
  PRIO_BOSS_KERNEL = 0,   /* 상위 우선순위 */
  
  AA_PRIO_1,              /* 1. TOP */
  BB_PRIO_2,              /* 2 */

  Cx_PRIO_3,
  
  PRIO_BOSS_IDLE = 255,   /* 하위 우선순위 */
} boss_prio_t;


/*===========================================================================*/
/*                           메시지 큐 명령어                                */
/*---------------------------------------------------------------------------*/
typedef enum {
  M_CMD_EMPTY = 0,

  M_CMD_1,
  M_CMD_2,
  /* ... */
  
  M_CMD_MAX = 255,
} msg_cmd_t;


/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/
#include <stdio.h>        // "printf 사용을 위해"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "Boss.h"
#include "Boss_Mem.h"
#include "Boss_Tmr.h"
#include "Boss_Q_Msg.h"
#include "Boss_Q_MBox.h"
#include "Boss_Sem.h"
#include "Boss_SPY.h"

/*===========================================================================*/
/*                                    ASSERT                                 */
/*---------------------------------------------------------------------------*/
#define BOSS_ASSERT(expr)  do { if(!(expr)) _assert(PSTR(__FILE__), __LINE__); } while(0)
void _assert(const char *file, unsigned int line);

/*===========================================================================*/
/*                                [ S P Y ]                                  */
/*---------------------------------------------------------------------------*/
#ifdef _BOSS_SPY_
void Boss_spy_report(void);
#endif

#ifdef _BOSS_MEM_INFO_
void Boss_mem_info_report(void);
#endif


/*===========================================================================*/
/*                              SIGNALS DEFINE                               */
/*---------------------------------------------------------------------------*/
/* SIG_15_BIT ~ SIG_12_BIT (상위 4비트는 RT-BOSS에서 사용함)      */
#define SIG_11_BIT          (boss_sigs_t)(1 << 11)      /* 0x0800 */
#define SIG_10_BIT          (boss_sigs_t)(1 << 10)      /* 0x0400 */
#define SIG_09_BIT          (boss_sigs_t)(1 << 9)       /* 0x0200 */
#define SIG_08_BIT          (boss_sigs_t)(1 << 8)       /* 0x0100 */

#define SIG_07_BIT          (boss_sigs_t)(1 << 7)       /* 0x0080 */
#define SIG_06_BIT          (boss_sigs_t)(1 << 6)       /* 0x0040 */
#define SIG_05_BIT          (boss_sigs_t)(1 << 5)       /* 0x0020 */
#define SIG_04_BIT          (boss_sigs_t)(1 << 4)       /* 0x0010 */
#define SIG_03_BIT          (boss_sigs_t)(1 << 3)       /* 0x0008 */
#define SIG_02_BIT          (boss_sigs_t)(1 << 2)       /* 0x0004 */
#define SIG_01_BIT          (boss_sigs_t)(1 << 1)       /* 0x0002 */
#define SIG_00_BIT          (boss_sigs_t)(1 << 0)       /* 0x0001 */

/*===========================================================================*/
/*                     USER DEFINE & FUNCTION PROTOTYPES                     */
/*---------------------------------------------------------------------------*/
#define PRINTF(fmt, args...)     \
  do{ _Boss_sched_lock(); printf_P( PSTR(fmt), ##args ); _Boss_sched_free(); } while(0)

#endif  /* _BOSS_CONF_H_ */
