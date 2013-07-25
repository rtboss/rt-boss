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
		SVC     0         ;/* SVC 0 ȣ�� (SVC_Handler() ����)  */


;/*===========================================================================
;    S   V   C _   H A N D L E R
;---------------------------------------------------------------------------*/
;	void SVC_Handler(void)
		PUBLIC	SVC_Handler
SVC_Handler:
		BL      _Boss_start_tcb_sp  ;/* ���ϰ� : "R0"�� start_tcb_sp       */

		LDMIA   R0!, {R4-R11,LR}    ;// R4-R11, EXC_RETURN ����
		MSR     PSP, R0

		BX      LR                  ;/* ���� SVC (R0-R3, R12, PC, PSR ����)*/





;/*===========================================================================
;    P E N D S V _ H A N D L E R
;---------------------------------------------------------------------------*/
;	void PendSV_Handler(void)
		PUBLIC	PendSV_Handler
PendSV_Handler:
		MRS     R0, PSP         ;// R0-R3, R12, PC, PSR ����Ǿ� ����
  
  		#if defined __ARMVFP__
  		TST       LR, #0x10
  		IT        EQ
  		VSTMDBEQ  R0!, {S16-S31}
  		#endif
  		
		STMDB   R0!, {R4-R11,LR}   ;// R4-R11, EXC_RETURN ����

		;/*
		;** void *_Boss_switch_current_tcb(void *cur_task_sp)
		;** �Ű����� : "R0"�� �������� �½�ũ ���� ������
		;** ���ϰ�   : "R0"�� ������ �½�ũ ���� ������
		;*/
		BL      _Boss_switch_current_tcb

  		LDMIA   R0!, {R4-R11,LR}   ;// R4-R11, EXC_RETURN ����

  		#if defined __ARMVFP__
  		TST       LR, #0x10
  		IT        EQ
  		VLDMIAEQ  R0!, {S16-S31}
  		#endif

		MSR     PSP, R0

		BX      LR              ;// ���� PendSV (R0-R3, R12, PC, PSR ����)



		END
