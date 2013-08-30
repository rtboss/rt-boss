#ifndef _BOSS_MEMORY_H_
#define _BOSS_MEMORY_H_
/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                             RT-BOSS (Memory)                                *
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
void *Boss_malloc(boss_uptr_t size);
void Boss_mfree(void *p);


#ifdef _BOSS_MEM_INFO_
boss_uptr_t _Boss_mem_info_total(void);
boss_uptr_t _Boss_mem_info_free(void);
boss_uptr_t _Boss_mem_info_used(void);
boss_uptr_t _Boss_mem_info_peak(void);
boss_uptr_t _Boss_mem_info_block(void);
boss_uptr_t _Boss_mem_info_first_free(void);
#endif

#endif  /* _BOSS_MEMORY_H_ */
