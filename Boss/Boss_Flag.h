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

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
typedef struct _flag_link_struct {
  struct _flag_link_struct    *prev;      /* Flag link */
  struct _flag_link_struct    *next;
  
  boss_flags_t    wait_flags;
  
  boss_tcb_t      *p_tcb;
} _flag_link_t;


typedef struct {
  boss_flags_t  flags;
  
  _flag_link_t  *wait_list;
} boss_flag_grp_t;


/* Flag Wait Option */
#define _FLAG_OPT_OR        0x00
#define _FLAG_OPT_AND       0x01
#define _FLAG_OPT_CONSUME   0x02


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
void Boss_flag_grp_init(boss_flag_grp_t *p_grp);

void Boss_flag_send(boss_flag_grp_t *p_grp, boss_flags_t set_flags);
void Boss_flag_clear(boss_flag_grp_t *p_grp, boss_flags_t clr_flags);

boss_flags_t Boss_flag_wait(boss_flag_grp_t *p_grp, boss_flags_t wait_flags,
                                    boss_reg_t wait_opt, boss_tmr_ms_t timeout);
#endif  /* _BOSS_FLAG_H_ */
