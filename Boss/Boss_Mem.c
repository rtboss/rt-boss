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

#ifdef _BOSS_MEMORY_H_    // 사용 시 "Boss_Conf.h"에 #include "Boss_Mem.h" 포함

/*===========================================================================*/
/*                      DEFINITIONS & TYPEDEFS & MACROS                      */
/*---------------------------------------------------------------------------*/
#define _MEM_MAGIC_CODE_        (boss_uptr_t)0x93467878

typedef struct _boss_mem_blk_struct {     /* Memory Block Head Tyep */
  boss_uptr_t               in_use;
  boss_uptr_t               size;

  struct _boss_mem_blk_struct *prev;
  struct _boss_mem_blk_struct *next;
} _boss_mem_blk_t;


#ifdef _BOSS_MEM_INFO_
typedef struct _boss_mem_info_struct {
  boss_uptr_t       used_size;
  boss_uptr_t       used_peak;
  
  boss_uptr_t       block;
  _boss_mem_blk_t   *first;
} _boss_mem_info_t;
#endif

/*===========================================================================*/
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/
boss_align_t _memory_pool[_BOSS_MEM_POOL_SIZE / sizeof(boss_align_t)];

#ifdef _BOSS_MEM_INFO_
_boss_mem_info_t _boss_mem_info;
#endif

#define _ALIGN_SIZE                 (boss_uptr_t)(sizeof(boss_align_t))

#define _MEM_POOL_START   (void *)_memory_pool
#define _MEM_POOL_END     (void *)((boss_uptr_t)_memory_pool+sizeof(_memory_pool))
/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/

/*===========================================================================
    _   B O S S _ M E M _ P O O L _ I N I T
---------------------------------------------------------------------------*/
static void _Boss_mem_pool_init(void)
{
  _boss_mem_blk_t *first_blk = (_boss_mem_blk_t *)_memory_pool;

  BOSS_ASSERT( ((boss_uptr_t)_memory_pool & (_ALIGN_SIZE - 1)) == 0 );
  BOSS_ASSERT( (sizeof(_memory_pool) & (_ALIGN_SIZE - 1)) == 0 );

  BOSS_ASSERT( first_blk->size == 0 );
  
  first_blk->in_use = _BOSS_FALSE;
  first_blk->size   = sizeof(_memory_pool);
  
  first_blk->prev   = _BOSS_NULL;
  first_blk->next   = _BOSS_NULL;
  
  #ifdef _BOSS_MEM_INFO_
  _boss_mem_info.used_size  = 0;
  _boss_mem_info.used_peak  = 0;
  _boss_mem_info.block      = 1;
  _boss_mem_info.first      = (_boss_mem_blk_t *)_memory_pool;
  #endif
}


/*===========================================================================
    B O S S _ M A L L O C
---------------------------------------------------------------------------*/
void *Boss_malloc(boss_uptr_t size)
{
  static boss_reg_t _mem_pool_init = _BOSS_FALSE;
  
  _boss_mem_blk_t *p_alloc;

  if(_mem_pool_init == _BOSS_FALSE)
  {
    BOSS_IRQ_DISABLE();
    _Boss_mem_pool_init();                              /* Memory Pool init */
    _mem_pool_init = _BOSS_TRUE;
    BOSS_IRQ_RESTORE();
  }
                                                        /* 메모리 크기 정렬 */
  size = size + ( sizeof(_boss_mem_blk_t) + (_ALIGN_SIZE-1) );
  size = size & ~(_ALIGN_SIZE-1);
  
  p_alloc = (_boss_mem_blk_t *)_memory_pool;
  //BOSS_ASSERT(p_alloc->size > size);  /* First 할당 */
  
  BOSS_IRQ_DISABLE();                         /* 할당 가능한 블럭을 찾는다. */
  while( p_alloc && ((p_alloc->size < size) || (p_alloc->in_use != _BOSS_FALSE)) )
  {
    p_alloc = p_alloc->next;
  }
  
  if(p_alloc != _BOSS_NULL)
  {
    if( p_alloc->size > (size + sizeof(_boss_mem_blk_t) + _ALIGN_SIZE) )
    {                                             /* 메모리 블럭 분할 시 */
      _boss_mem_blk_t *p_prev = p_alloc;
      
      p_prev->size = p_prev->size - size;
                                                      /* 블럭 분할 */
      p_alloc = (_boss_mem_blk_t *)((boss_uptr_t)p_prev + p_prev->size);
      p_alloc->size = size;
      
      p_alloc->prev = p_prev;
      p_alloc->next = p_prev->next;
      p_prev->next = p_alloc;
      
      if(p_alloc->next != _BOSS_NULL) {
        p_alloc->next->prev = p_alloc;
      }

      #ifdef _BOSS_MEM_INFO_
      _boss_mem_info.block++;
      #endif
    }
    
    p_alloc->in_use = _MEM_MAGIC_CODE_;               /* 메모리 할당 */
    
    #ifdef _BOSS_MEM_INFO_
    _boss_mem_info.used_size += p_alloc->size;
    if( _boss_mem_info.used_size > _boss_mem_info.used_peak ) {
      _boss_mem_info.used_peak = _boss_mem_info.used_size;
    }
    #endif
    
    p_alloc = (_boss_mem_blk_t *)( (boss_uptr_t)p_alloc + sizeof(_boss_mem_blk_t) );
  }
  BOSS_IRQ_RESTORE();

  BOSS_ASSERT(p_alloc != _BOSS_NULL);             /* 메모리 FULL */

  return (void *)p_alloc;
}


/*===========================================================================
    B O S S _ M F R E E
---------------------------------------------------------------------------*/
void Boss_mfree(void *p)
{
  _boss_mem_blk_t *p_prev;
  _boss_mem_blk_t *p_free;
  _boss_mem_blk_t *p_next;
  
  p_free = (_boss_mem_blk_t *)( (boss_uptr_t)p - sizeof(_boss_mem_blk_t) );

  BOSS_ASSERT( p_free->in_use == _MEM_MAGIC_CODE_ );
  BOSS_ASSERT( (_MEM_POOL_START <= (void *)p_free) && ((void *)p_free < _MEM_POOL_END) );  



  BOSS_IRQ_DISABLE();

  BOSS_ASSERT( (p_free->prev != _BOSS_NULL) ? (p_free->prev->next == p_free)
                                            : (_MEM_POOL_START == p_free) );
  
  BOSS_ASSERT( (p_free->next != _BOSS_NULL) ? (p_free->next->prev == p_free)
        : ((boss_uptr_t)p_free + p_free->size == (boss_uptr_t)_MEM_POOL_END) );
  
  #ifdef _BOSS_MEM_INFO_
  _boss_mem_info.used_size -= p_free->size;
  #endif
  
  p_free->in_use = _BOSS_FALSE;               /* 메모리 블럭 해제 */

  p_next = p_free->next;
  if( (p_next != _BOSS_NULL) && (p_next->in_use == _BOSS_FALSE) ) /* 병합 (뒤) */
  {
    BOSS_ASSERT( ((boss_uptr_t)p_next - (boss_uptr_t)p_free) == p_free->size);
    
    p_free->size = p_free->size + p_next->size;
    p_free->next = p_next->next;

    if(p_next->next != _BOSS_NULL) {
      p_next->next->prev = p_free;
    }
    
    #ifdef _BOSS_MEM_INFO_
    p_next->size = 0;
    p_next->next = _BOSS_NULL;
    p_next->prev = _BOSS_NULL;
    
    _boss_mem_info.block--;
    #endif
  }

  p_prev = p_free->prev;
  if( (p_prev != _BOSS_NULL) && (p_prev->in_use == _BOSS_FALSE) ) /* (앞) 병합 */
  {
    BOSS_ASSERT( ((boss_uptr_t)p_free - (boss_uptr_t)p_prev) == p_prev->size );
    
    p_prev->size = p_prev->size + p_free->size;
    p_prev->next = p_free->next;

    if(p_free->next != _BOSS_NULL) {
      p_free->next->prev = p_prev;
    }
    
    #ifdef _BOSS_MEM_INFO_
    p_free->size = 0;
    p_free->next = _BOSS_NULL;
    p_free->prev = _BOSS_NULL;
    
    _boss_mem_info.block--;
    #endif
  }
  BOSS_IRQ_RESTORE();
}


#ifdef _BOSS_MEM_INFO_
/*===========================================================================*/
/*                              MEM INFO & DEBUG                             */
/*---------------------------------------------------------------------------*/


/*===========================================================================
    _   B O S S _ M E M _ I N F O _ T O T A L
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_total(void)
{
  return sizeof(_memory_pool);
}


/*===========================================================================
    _   B O S S _ M E M _ I N F O _ F R E E
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_free(void)
{
  return (sizeof(_memory_pool) - _boss_mem_info.used_size);
}


/*===========================================================================
    _   B O S S _ M E M _ I N F O _ U S E D
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_used(void)
{
  return _boss_mem_info.used_size;
}


/*===========================================================================
    _   B O S S _ M E M _ I N F O _ P E A K
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_peak(void)
{
  return _boss_mem_info.used_peak;
}


/*===========================================================================
    _   B O S S _ M E M _ I N F O _ B L O C K
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_block(void)
{
  return _boss_mem_info.block;
}

/*===========================================================================
    _   B O S S _ M E M _ I N F O _ F I R S T _ F R E E
---------------------------------------------------------------------------*/
boss_uptr_t _Boss_mem_info_first_free(void)
{
  _boss_mem_blk_t *p_first = (_boss_mem_blk_t *)_memory_pool;
  
  return ( (p_first->in_use == _BOSS_FALSE) ? (p_first->size) : 0 );
}

#endif /* _BOSS_MEM_INFO_ */

#endif  /* _BOSS_MEMORY_H_ */
