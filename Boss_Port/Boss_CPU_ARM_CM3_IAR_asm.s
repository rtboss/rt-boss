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
;    _   B O S S _ S T A R T _ S C H E D U L E
;---------------------------------------------------------------------------*/
; 	void _Boss_start_schedule(void)
		PUBLIC	_Boss_start_schedule
_Boss_start_schedule:
		SVC     0         ;/* SVC 0 호출 (SVC_Handler() 실행)  */





;/*===========================================================================
;    S   V   C _   H A N D L E R
;---------------------------------------------------------------------------*/
;	void SVC_Handler(void)
		PUBLIC	SVC_Handler
SVC_Handler:
		CPSID   I                   ;/* [ IRQ 비활성화 ] */

		LDR     R0, =0xE000ED08     ;/* NVIC Vector Table Offset Register  */
		LDR     R0, [R0]            ;/* Vector Table 시작 번지             */
		LDR     R0, [R0, #0]        ;/* 최기 스택 (__initial_sp)           */
		MSR     MSP, R0             ;/* "MSP"를 초기 스택 값으로 초기화    */

		BL      _Boss_start_tcb_sp  ;/* 리턴값 : "R0"는 start_tcb_sp       */

		LDMIA   R0!, {R4-R11}
		MSR     PSP, R0

		LDR     LR, =0xFFFFFFFD     ;/* 스레드 특근 모드, PSP 사용         */
		CPSIE   I                   ;/* [ IRQ 활성화 ]   */
		BX      LR                  ;/* 리턴 SVC (R0-R3, R12, PC, PSR 복원)*/





;/*===========================================================================
;    P E N D S V _ H A N D L E R
;---------------------------------------------------------------------------*/
;	void PendSV_Handler(void)
		PUBLIC	PendSV_Handler
PendSV_Handler:
		MRS     R0, PSP         ;// R0-R3, R12, PC, PSR 저장되어 있음
		STMDB   R0!, {R4-R11}   ;// R4-R11 저장
		MOV     R4, LR          ;// LR 임시저장 (BL 사용을 위해)

		CPSID   I                   ;/* [ IRQ 비활성화 ] */
		;/*
		;** void *_Boss_switch_current_tcb(void *cur_task_sp)
		;** 매개변수 : "R0"는 실행중인 태스크 스택 포인터
		;** 리턴값   : "R0"는 실행할 태스크 스택 포인터
		;*/
		BL      _Boss_switch_current_tcb

		CPSIE   I                   ;/* [ IRQ 활성화 ]   */

		MOV     LR, R4          ;// LR 임시저장 (복원)
		LDMIA   R0!, {R4-R11}   ;// R4-R11 복원
		MSR     PSP, R0

		BX      LR              ;// 리턴 PendSV (R0-R3, R12, PC, PSR 복원)



		END
