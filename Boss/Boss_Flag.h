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
/* Flag Wait Option */
#define _FLAG_OPT_OR        0x00
#define _FLAG_OPT_AND       0x01
#define _FLAG_OPT_CONSUME   0x02

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

typedef boss_flag_grp_t *   BOSS_FLAG_ID_T;  /* Flag ID TYPE (Flag grp Point) */

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
#ifdef _BOSS_MEMORY_H_
BOSS_FLAG_ID_T Boss_flag_grp_create(void);

#define Boss_flag_grp_del(flag_id)  do {                                \
                                      Boss_flag_grp_mfree_del(flag_id); \
                                      flag_id = _BOSS_NULL;             \
                                    } while(0)

void Boss_flag_grp_mfree_del(boss_flag_grp_t *p_grp);
#endif /* _BOSS_MEMORY_H_ */

void Boss_flag_grp_init(boss_flag_grp_t *p_grp);

void Boss_flag_send(boss_flag_grp_t *p_grp, boss_flags_t set_flags);
void Boss_flag_clear(boss_flag_grp_t *p_grp, boss_flags_t clr_flags);

boss_flags_t Boss_flag_wait(boss_flag_grp_t *p_grp, boss_flags_t wait_flags,
                                    boss_reg_t wait_opt, boss_tmr_ms_t timeout);
#endif  /* _BOSS_FLAG_H_ */
