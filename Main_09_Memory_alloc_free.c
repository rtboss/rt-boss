/*
*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
*                                                                             *
*                                M  A  I  N                                   *
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
/*                             GLOBAL VARIABLES                              */
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*                            FUNCTION PROTOTYPES                            */
/*---------------------------------------------------------------------------*/


/*===========================================================================
    M A I N
---------------------------------------------------------------------------*/
int main(void)
{
  PRINTF("\n[[ 메모리 관리자 예제 ]]\n");
  PRINTF("메모리 관리자는 독립적으로 실행함.\n");

  /*------------------------------------------------------[ 초기화 ]----------*/
  Boss_mfree( Boss_malloc(1) );   // 초기화 ( Boss_malloc() 최초 실행시 초기화함 )
  PRINTF("\n초기화  Boss_mfree( Boss_malloc(1) ) \n");
  Boss_mem_info_report();         // 메모리 사용량 확인

  /*------------------------------------------------------[ 사용 방법 ]-------*/
  {
    void *p_ex;
    
    p_ex = Boss_malloc(32); //  p_ex 32 byte 할당
    /* p_ex 사용 */
    Boss_mfree(p_ex);       // p_ex 반환
  }


  /*--------------------------------------------------------------------------*/
  Boss_mem_info_report();

  {
    void *p1;
    void *p2;
    void *p3;

    p1 = Boss_malloc(32);       // 할당
    p2 = Boss_malloc(128);
    p3 = Boss_malloc(256);

    PRINTF("\np1, p2, p3 할당\n");
    Boss_mem_info_report();

    /* p1, p2, p3 사용 */

    Boss_mfree(p1);     PRINTF("\n p1 반환\n");  Boss_mem_info_report();
    Boss_mfree(p2);     PRINTF("\n p2 반환\n");  Boss_mem_info_report();
    Boss_mfree(p3);     PRINTF("\n p3 반환\n");  Boss_mem_info_report();
  }
  
  PRINTF("메모리가 블럭화 되지 않고 하나의 메모리로 병합됨\n");

  for(;;)
  {
  }
  
  return 0;
}


/*
        ########## 실행 결과 ##########

        [[ 메모리 관리자 예제 ]]
        메모리 관리자는 독립적으로 실행함.
        
        초기화  Boss_mfree( Boss_malloc(1) ) 
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]    24 ( 2%)    0 ( 0%)  1024     1    1024
        
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]    48 ( 4%)    0 ( 0%)  1024     1    1024
        
        
        p1, p2, p3 할당
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]   464 (45%)  464 (45%)  1024     4     560
        
        
         p1 반환
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]   464 (45%)  416 (40%)  1024     4     560
        
        
         p2 반환
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]   464 (45%)  272 (26%)  1024     3     560
        
        
         p3 반환
        
        [Mmory]  Peak byte  Used byte  Total  Block  first
        [Info]   464 (45%)    0 ( 0%)  1024     1    1024
        
        메모리가 블럭화 되지 않고 하나의 메모리로 병합됨
*/
