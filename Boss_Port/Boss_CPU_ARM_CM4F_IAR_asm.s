;/*
;*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
;*                                                                             *
;*                       RT-BOSS PORT [ Cortex-M3 IAR ASM ]                    *
;*                                                                             *
;*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
;*/
		RSEG    CODE:CODE(2)
		thumb


		EXTERN  _Boss_switch_current_tcb
		EXTERN  _Boss_start_tcb_sp

;/*===========================================================================
;    _ S V C _ C A L L _ 0
;---------------------------------------------------------------------------*/
; 	void _svc_call_0(void)
		PUBLIC	_svc_call_0
_svc_call_0:
		SVC     0         ;/* SVC 0 호출 (SVC_Handler() 실행)  */


;/*===========================================================================
;    S   V   C _   H A N D L E R
;---------------------------------------------------------------------------*/
;	void SVC_Handler(void)
		PUBLIC	SVC_Handler
SVC_Handler:
		BL      _Boss_start_tcb_sp  ;/* 리턴값 : "R0"는 start_tcb_sp       */

		LDMIA   R0!, {R4-R11,LR}    ;// R4-R11, EXC_RETURN 복원
		MSR     PSP, R0

		BX      LR                  ;/* 리턴 SVC (R0-R3, R12, PC, PSR 복원)*/





;/*===========================================================================
;    P E N D S V _ H A N D L E R
;---------------------------------------------------------------------------*/
;	void PendSV_Handler(void)
		PUBLIC	PendSV_Handler
PendSV_Handler:
		MRS     R0, PSP         ;// R0-R3, R12, PC, PSR 저장되어 있음
  
  		#if defined __ARMVFP__
  		TST       LR, #0x10
  		IT        EQ
  		VSTMDBEQ  R0!, {S16-S31}
  		#endif
  		
		STMDB   R0!, {R4-R11,LR}   ;// R4-R11, EXC_RETURN 저장

		;/*
		;** void *_Boss_switch_current_tcb(void *cur_task_sp)
		;** 매개변수 : "R0"는 실행중인 태스크 스택 포인터
		;** 리턴값   : "R0"는 실행할 태스크 스택 포인터
		;*/
		BL      _Boss_switch_current_tcb

  		LDMIA   R0!, {R4-R11,LR}   ;// R4-R11, EXC_RETURN 복원

  		#if defined __ARMVFP__
  		TST       LR, #0x10
  		IT        EQ
  		VLDMIAEQ  R0!, {S16-S31}
  		#endif

		MSR     PSP, R0

		BX      LR              ;// 리턴 PendSV (R0-R3, R12, PC, PSR 복원)



		END
