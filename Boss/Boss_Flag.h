#ifndef _BOSS_FLAG_H_
#define _BOSS_FLAG_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                              RT-BOSS (Flag)                                 *
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
typedef struct _flag_wait_struct {
  struct _flag_wait_struct    *prev;      /* flag wait list link */
  struct _flag_wait_struct    *next;
  
  boss_u16_t      wait_opt;
  boss_flags_t    wait_flags;
  
  boss_tcb_t      *p_tcb;
} _flag_wait_t;


typedef struct {
  boss_flags_t  flags;
  
  _flag_wait_t  *wait_list;
} boss_flag_obj_t;


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_flag_obj_init(boss_flag_obj_t *p_flag_obj);

void Boss_flag_send(boss_flag_obj_t *p_flag_obj, boss_flags_t set_flags);
void Boss_flag_clear(boss_flag_obj_t *p_flag_obj, boss_flags_t clr_flags);

boss_flags_t Boss_flag_receive_or(boss_flag_obj_t *p_flag_obj, boss_flags_t or_flags);
boss_flags_t Boss_flag_receive_and(boss_flag_obj_t *p_flag_obj, boss_flags_t and_flags);

boss_flags_t Boss_flag_wait_or(boss_flag_obj_t *p_flag_obj,
                                boss_flags_t wait_flags, boss_tmr_ms_t timeout);
boss_flags_t Boss_flag_wait_and(boss_flag_obj_t *p_flag_obj,
                                boss_flags_t wait_flags, boss_tmr_ms_t timeout);
#endif  /* _BOSS_FLAG_H_ */
