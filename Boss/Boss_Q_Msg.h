#ifndef _BOSS_MSG_Q_H_
#define _BOSS_MSG_Q_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                          RT-BOSS (Message Queue)                            *
*                                                                             *
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*/

/*===========================================================================*/
/*                               INCLUDE FILE                                */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
typedef enum {
  MSG_Q_PRIORITY  = 0x00,     /* 우선순위 */
  MSG_Q_FIFO      = 0x01,     /* 대기순서 */
} _msg_q_opt_t;


typedef struct {
  msg_cmd_t       m_cmd;
  boss_uptr_t     param;
} boss_msg_t;


typedef struct _msg_link_struct {
  struct _msg_link_struct   *prev;      /* Message link */
  struct _msg_link_struct   *next;

  boss_tcb_t    *p_tcb;
  boss_msg_t    msg;
} _msg_link_t;


typedef struct {
  boss_msg_t    *msg_fifo;
  boss_u08_t    fifo_max;
  boss_u08_t    fifo_count;
  boss_u08_t    fifo_head;
  boss_u08_t    fifo_tail;
  
  _msg_q_opt_t  q_opt;
  
  _msg_link_t   *wait_list;
} boss_msg_q_t;

typedef boss_msg_q_t *  BOSS_MSG_Q_ID_T;    /* Message Queue ID TYPE */


/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/
#ifdef _BOSS_MEMORY_H_
BOSS_MSG_Q_ID_T Boss_msg_q_create(boss_u08_t fifo_max, _msg_q_opt_t q_opt);

#define Boss_msg_q_del(msg_q_id)  do {                                \
                                      Boss_msg_q_mfree_del(msg_q_id); \
                                      msg_q_id = _BOSS_NULL;          \
                                    } while(0)

void Boss_msg_q_mfree_del(boss_msg_q_t *msg_q);
#endif /* _BOSS_MEMORY_H_ */


void Boss_msg_q_init(boss_msg_q_t *msg_q, boss_msg_t *p_fifo,
                                      boss_uptr_t fifo_bytes, _msg_q_opt_t q_opt);

boss_reg_t Boss_msg_send(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param);
boss_reg_t Boss_msg_send_front(boss_msg_q_t *msg_q, msg_cmd_t m_cmd, boss_uptr_t param);
boss_msg_t Boss_msg_wait(boss_msg_q_t *msg_q, boss_tmr_ms_t timeout);

#endif  /* _BOSS_MSG_Q_H_ */
