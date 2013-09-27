#ifndef _BOSS_SPY_H_
#define _BOSS_SPY_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                               RT-BOSS (SPY)                                 *
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
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void _Boss_spy_context(boss_tcb_t *curr_tcb, boss_tcb_t *best_tcb);
void _Boss_spy_set(boss_tcb_t *p_tcb, boss_stk_t *sp_base, boss_uptr_t bytes);
boss_u32_t Boss_spy_elapse_us(void);
boss_u32_t Boss_spy_elapse_prev_us(void);
void Boss_spy_restart(void);
int Boss_spy_task_status(boss_tcb_t *p_tcb);

#endif  /* _BOSS_SPY_H_ */
