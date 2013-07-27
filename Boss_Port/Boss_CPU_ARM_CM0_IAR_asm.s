;/*
;*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*=====*
;*                                                                             *
;*                       RT-BOSS PORT [ Cortex-M0 IAR ASM ]                    *
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

		LDMIA   R0!, {R2-R7}    ;// R10, R11, R4-R7 복원
		MOV     R10, R2
		MOV     R11, R3

		LDMIA   R0!, {R2-R3}    ;// R8-R9 복원
		MOV     R8, R2
		MOV     R9, R3

		MSR     PSP, R0

		LDR     R1, =0xFFFFFFFD     ;/* 스레드 특근 모드, PSP 사용         */
		BX      R1                  ;/* 리턴 SVC (R0-R3, R12, PC, PSR 복원)*/





;/*===========================================================================
;    P E N D S V _ H A N D L E R
;---------------------------------------------------------------------------*/
;	void PendSV_Handler(void)
		PUBLIC	PendSV_Handler
PendSV_Handler:
		MRS     R0, PSP         ;// R0-R3, R12, PC, PSR 저장되어 있음

		SUBS    R0, R0, #32
  
		MOV     R2, R10
		MOV     R3, R11
		STMIA   R0!, {R2-R7}    ;// R10, R11, R4-R7 저장
  
		MOV     R2, R8
		MOV     R3, R9
		STMIA   R0!, {R2-R3}    ;// R8-R9 저장
  
		SUBS    R0, R0, #32
  
		MOV     R4, LR          ;// LR 임시저장 (BL 사용을 위해)

		;/*
		;** void *_Boss_switch_current_tcb(void *cur_task_sp)
		;** 매개변수 : "R0"는 실행중인 태스크 스택 포인터
		;** 리턴값   : "R0"는 실행할 태스크 스택 포인터
		;*/
		BL      _Boss_switch_current_tcb

		MOV     LR, R4          ;// LR 임시저장 (복원)

		LDMIA   R0!, {R2-R7}    ;// R10, R11, R4-R7 복원
		MOV     R10, R2
		MOV     R11, R3

		LDMIA   R0!, {R2-R3}    ;// R8-R9 복원
		MOV     R8, R2
		MOV     R9, R3

		MSR     PSP, R0

		BX      LR              ;// 리턴 PendSV (R0-R3, R12, PC, PSR 복원)



		END
