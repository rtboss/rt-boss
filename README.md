RT-BOSS
=======
> [RT-BOSS 네이버 카페](http://cafe.naver.com/rtboss "http://cafe.naver.com/rtboss")

### V1.31 (2013/10/29) ###
	- 기능별 생성/삭제 기능 추가. (메모리 관리자에서 할당)
	 * 생성 : xxx_ID = Boss_xxx_create()
	 * 삭제 : Boss_xxx_del(xxx_ID)
	 (타이머 / 플래그 그룹 / 세마포어 / 메지시 큐)

	- 예제 추가 및 정리. (기능별 생성/삭제 적용)
	- 메시지 큐 기능 개선. (이중 원형 링크)
	- 플래그 이벤트 기능 개선. (이중 원형 링크)
	- 커널 Timeout Tick 과 Timer Tick 분리.
	- Demo 프로젝트 폴더 이름 변경.
	- 기능별 헤드파일 추가/제거로 동작 설정. (Boss_Conf.h)


### V1.30 (2013/10/13) ###
	- 시그널(Signal) 기능 제거. (Flag로 대체 가능)
	- TCB 와 Stack 통합. (Stack에 TCB 할당)
	- 세마포어 기능 개선.
	- 예제 정리 및 Flag 예제 추가.


### V1.29 (2013/10/02) ###
	- 메시지 큐 기능 개선.
	- 카운트 세마포어 구현.
	- Mail Box 기능 제거. (메시지 큐로 대체가능)
	- Task 상태 확인 추가.

###### TimeOUT 정의 ######
	- NO_WAIT      (0x00000000) : 대기 없이 결과 확인.
	- WAIT_FOREVER (0xFFFFFFFF) : 결과를 받을때 까지 대기.
	- NO_WAIT < timeout < WAIT_FOREVER : timeout 동안 결과 대기.


### V1.28b (2013/09/22) ###
	- Boss Flag 기능 추가.
	- 시그널 인터페이스 정리.
	- git CRLF 관련 개선.


### V1.27 (2013/08/26) ###
	- 예제 코드 추가. ( Task, Timer, Signal 등 14개 )
	- Cortex-M0 포팅.
	- Cortex-M4 포팅.
	- Main.c 파일 통합.
	- Message Queue 외부 할당 방식으로 변경.
	- 메일박스 관련 정리. (Boss_mbox_send() 기능 제거)
	- Wait Sleep시 타임아웃 0이면 타이머 등록 안함.
	- 타이머 콜백 실행시 문맥전환 금지.
	- Cortex-Mx MSP 출력 추가. (Boss_spy_report)
	- Task Exit 외부에서 호출 가능 하도록 함.


### V1.26 (2013/07/23) ###
	- 라운드로빈(Round Robin) 스케줄링 구현.(_BOSS_RR_QUANTUM_MS)
	- STM32L1xx(Cortex-M3) KEIL/IAR 프로젝트 추가.


### V1.25 (2013/07/20) ###
	- SPY 기능 정리 및 "Boss_SPY.c/h" 파일로 분리.
	- TCB 상태 제거 하고 TCB 스케줄러 링크(run_next)로 판단함.
	- 타이머를 Boss_mem 에서 할당하지 않고 변수로 처리하도록함.
	- 타이머 콜백 기능만 지원, 타이머 시그널 처리 제거.
	- 메일박스 pend 및 세마포어 타임아웃이 "0" 일때 타임아웃없이 무기한 대기 하도록함.


### v1.24 (2012/02/18) ###
	- Cortex-M3 Main Stack 검사 기능. (MSP)
	- Cortex-M3 IAR 적용.
	- SPY 관련 TCB 멈버 이름 변경. 
	- SPY를 위한 스택 초기값을 _Boss_spy_setup()함수로 이동.
	- SPY 관련 코드 최적화.
	- Main의 스택 변수를 boss_mem_align_t형으로 선언.


### v1.23 (2012/02/12) ###
	- 메모리 첫번째 블럭에 할당하지 못할때 버그 수정.
	- 메모리 관리자 정보 출력 기능 추가. 
	- 메모리 할당/해제 함수 이름 변경.
  		Boss_mem_alloc() -> Boss_malloc()
  		Boss_mem_free() -> Boss_mfree()
	- 시그널 전송 함수 이름 변경.
  		Boss_sigs_send() -> Boss_send()
	- 타이머를 Boss_mem 에서 할당 및 해제 하는 방식으로 변경.
	- 타이머 반복 기능 추가.
	- CPU 포팅파일 이름변경.
  		Port_ARM_CM3_Keil.c -> Boss_CPU_ARM_CM3_Keil.c
  		Port_ATmega_GCC.c   -> Boss_CPU_ATmega_GCC.c
	- Idle Task를 Main.c 파일로 이동.
	- PRINTF() 출력 문자열 "\r\n" -> '\n' 적용. (출력함수 변경)
	- ATmega : BOSS_ASSERT 파일명 출력하도록 변경.
	- SPY 출력시 전체 CPU 점유율 및 전체 문맥전환 횟수 출력하도록함.


### v1.22 (2012/01/28) ###
	- Task CPU 점유률 측정 기능 추가.


### v1.21 (2012/01/08) ###
	- TCB name 추가.
	- Stack 검사 기능 추가.
	- TCB 초기화 함수 추가.


### v1.20 (2012/01/03) ###
	- 파일이름 및 구조 변경.
	- 메모리 할당 및 해제 속도 항상.
	- 메모리 정보관련 _BOSS_MEM_INFO_ 추가.
	- Kernel의 sleep 기능을 Timer와 통합.
	- _Boss_start_schedule() 호출시 인자 전송방식에서 함수호출 방식으로변경. _Boss_start_tcb_sp()
	- Cortex-M3 SVC 처리 부분 제거.
	- Mail Box 전송시 FIFO방식으로 변경.
	- Mail Box Pand 시 메시지 처리 결과와 응답 결과값을 분리.
	- 세마포어 및 Mail Box Pand 시 우선순위 제어 기능 제외.
	- 크리티컬 섹션, 변수 저장방식 추가.
	- TCB 상태를 WAITING(대기) / LISTING(실행중 또는 준비) 두가지 상태로 축소.


### v1.13 (2011/02/08) ###
	- _Boss_schedule()에서 처리하던 "_current_tcb->status = TASK_SUSPEND"를 
  		_Boss_switch_current_tcb()에서 처리 하도록함.
	- 문맥전환시 인터럽트 중지 및 복원을 "_Boss_context_switch()"에서 MCU별로 처리하도록함.
	- ATmega 스택 구성 변경 및 SREG 추가.


### v1.12 (2011/01/14) ###
	- Mail Box에 우선순위 인자를 추가함.

### v1.11 (2010/12/23) ###
	- Boss_wait() 시그널 확인 IRQ 중지 후 비교.

### v1.10 (2010/12/15) ###
	- Cortex-M3 IAR 지원
	- ATmage128 GCC 지원
	- 폴더 구조 및 함수 이름 변경
	- args TCB 저장방식으로 변경
	- Boss_Conf.c 파일 추가.(설정 함수)
	- 타이머, 메시지, 메일박스의 메모리 할당 방법 변경
		* Boss_Tmr    : _BOSS_TMR_ALLOC() / _BOSS_TMR_FREE()
		* Boss_Q_Mail : _BOSS_MAIL_Q_BOX_ALLOC() / _BOSS_MAIL_Q_BOX_FREE()
		* Boss_Q_Msg  : _BOSS_MSG_FIFO_ALLOC() / _BOSS_MSG_FIFO_FREE()

	- Boss_task_delete() 반환값(sp_base) 제거
	- 스케줄러 정렬시 "prio_orig" 비교 루틴제거
	- 문맥전환 실행시 "_Boss_schedule()" 호출하여 "_Boss_context_switch()" 실행하도록함.
	- 메일박스 세마포어, 스케줄러(시그널 전송) 실행시 IRQ 활성화 상태로함.
	- Cortex-M3 SVC 처리 함수 C언어로 구현(스택 확인 코드 ASM) 


### v1.00 (2010/11/30) ###
	- RT-BOSS (Cortex-M3 Keil)
	- 커널 / 세마포어 / 타이머 / 메시지 / 메일박스 / 메모리 관리 기능구현

