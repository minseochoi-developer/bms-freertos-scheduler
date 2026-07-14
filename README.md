# BMS FreeRTOS Scheduler

AUTOSAR 기반 BMS(Battery Management System) 프로젝트의 태스크 로직을 FreeRTOS 태스크 구조로 재구현한 프로젝트. QEMU(MPS2-AN385, Cortex-M3) 상에서 동작하며, 실제 하드웨어 없이 RTOS 스케줄링 검증이 가능한 형태로 구성.

## 배경

기존 AUTOSAR BMS 프로젝트는 AUTOSAR OS/RTE 기반으로 구현했음. 이 프로젝트는 동일한 BMS 로직(전압/전류/온도 측정, Fault 진단, 상태 관리, 릴레이 제어)을 범용 RTOS인 FreeRTOS로 재구현해, AUTOSAR라는 특정 프레임워크에 종속되지 않은 RTOS 설계 역량과, 스케줄링/동기화/타이밍 분석 관점의 이해를 보여주는 것을 목표로 함.

## 환경

- MacBook (Apple Silicon)
- arm-none-eabi-gcc 15.2 (ARM GNU Toolchain)
- QEMU 11.0.2 (MPS2-AN385 머신 모델, Cortex-M3)
- CMake + Ninja
- FreeRTOS-Kernel (별도 clone)

## 아키텍처

6개 태스크가 큐를 통해 데이터를 주고받는 파이프라인 구조:

```
BatteryMeas ──(BatteryData, depth5)──> FaultDiag ──(FaultState)──> StateMachine
                                                                          │
                                                                 (SystemState)
                                                                          │
                                              ┌───────────────────────────┼──────────────┐
                                              ▼                           ▼              ▼
                                       RelayDecision              CANTx (읽기만)    SysMonitor
                                              │                                   (전체 태스크 감시)
                                       (RelayCommand)
                                              │
                                              ▼
                                            CANTx
```

| 태스크 | 우선순위 | 역할 | 주기/트리거 |
|---|---|---|---|
| RelayDecision | 5 (최고) | 릴레이 상태 결정 (OPEN→PRECHARGE→CLOSED), FAULT 시 즉시 강제 OPEN | SystemState 큐 수신 시 |
| FaultDiag | 4 | 셀 전압/전류/온도 다수결(5샘플 중 3개 이상) 기반 이상 진단 | BatteryData 큐 수신 시 |
| StateMachine | 3 | BMS 전체 상태(NORMAL/WARNING/FAULT) 판단, FAULT는 래치 | FaultState 큐 수신 시 |
| CANTx | 2 | SystemState/RelayCommand를 CAN 프레임으로 구성, 세미호스팅 로그 출력 | 100ms 주기 (논블로킹 큐 폴링) |
| BatteryMeas | 1 | 셀 전압/전류/온도 시뮬레이션 생성 | 100ms 주기 |
| SysMonitor | 0 (최저) | 전 태스크 스택 워터마크 감시 | 1000ms 주기 |

우선순위 설계 원칙: 처리 체인(FaultDiag→StateMachine→RelayDecision)이 데이터 생산자(BatteryMeas)보다 높아 큐 적체를 방지, RelayDecision이 최고 우선순위인 이유는 안전 액추에이터(릴레이) 반응 지연을 최소화하기 위함.

## 빌드 방법

FreeRTOS-Kernel 소스가 별도로 필요함:
```
git clone https://github.com/FreeRTOS/FreeRTOS.git ~/FreeRTOS --recurse-submodules
```

빌드:
```
git clone <이 리포 주소>
cd bms-freertos-scheduler
cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake
ninja -C build
```

FreeRTOS 경로가 `~/FreeRTOS/FreeRTOS/Source`가 아니면:
```
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake \
  -DFREERTOS_KERNEL_PATH=/path/to/FreeRTOS/Source
```

## QEMU 실행

```
ninja -C build run
```
`printf`가 ARM 세미호스팅(`_write` → `BKPT 0xAB` → `SYS_WRITEC`)을 통해 QEMU 콘솔로 그대로 출력됨. 종료: `Ctrl+A` 누른 뒤 `X`.

## 진행 상황

- [x] 툴체인 / CMake 빌드 시스템
- [x] 링커 스크립트 / startup 코드 (MPS2-AN385)
- [x] FreeRTOS 커널 연동, 부팅 검증 완료
- [x] 태스크 간 큐 설계 (bms_types.h)
- [x] BatteryMeas — 방전 곡선 + 노이즈 + 저확률 fault 값 주입 시뮬레이션
- [x] FaultDiag — 5샘플 링버퍼, 다수결(3/5) 기반 노이즈 필터링 진단
- [x] StateMachine — FAULT 래치(수동 리셋 전까지 유지)
- [x] RelayDecision — 3단계 릴레이 FSM (OPEN→PRECHARGE→CLOSED), 고정시간 프리차지
- [x] CANTx — CAN 프레임 구성 + 세미호스팅 로그 (SocketCAN 연동은 스코프 밖, 별도 프로젝트로 분리)
- [x] SysMonitor — 6개 태스크 스택 워터마크 실측
- [x] 우선순위 확정 및 근거 문서화
- [ ] WCET 측정 (DWT 사이클 카운터 기반)
- [ ] 스택 워터마크 기반 스택 크기 튜닝
- [ ] 우선순위 역전 재현 및 대응
- [ ] FAULT 상태 리셋 경로 설계 (현재 래치 후 해제 수단 없음)

## 실측 스택 워터마크 (튜닝 전 기준값)

| 태스크 | 할당 스택 (word) | 워터마크 (word) |
|---|---|---|
| BatteryMeas | 256 | 205 |
| FaultDiag | 256 | 172 |
| StateMachine | 256 | 209 |
| RelayDecision | 256 | 208 |
| CANTx | 256 | 130 |
| SysMonitor | 256 | 148 |

## 알려진 제약 / 설계상 단순화

- **프리차지 완료 판단이 전압 비교가 아닌 고정 시간(500ms)** — `BatteryData_t`에 버스 전압 측정 필드가 없어 단순화. 실제 BMS는 버스 전압이 팩 전압의 90% 이상 도달했는지로 판단하는 게 정석.
- **FAULT 래치 해제 경로 없음** — StateMachine이 한 번 FAULT에 들어가면 사람 개입(리셋) 전까지 유지하도록 설계했으나, 그 리셋을 트리거할 입력 경로(CAN 명령, GPIO 등)가 아직 없음.
- **SocketCAN 연동 없음** — CANTx는 CAN 프레임을 구성하고 로그로만 출력함. bare-metal QEMU 펌웨어와 리눅스 SocketCAN은 직접 연결이 불가능한 구조라(커널 API vs bare-metal), 실제 vcan0 브릿지는 별도 리눅스 유저스페이스 프로젝트(Virtual BMS-CAN Network)의 몫으로 분리.

## 디버깅 노트

FreeRTOS 포트(`portable/GCC/ARM_CM3/port.c`)가 SVC/PendSV/SysTick 핸들러를 CMSIS 표준 이름(`SVC_Handler` 등)이 아니라 `vPortSVCHandler`/`xPortPendSVHandler`/`xPortSysTickHandler`라는 자체 이름으로 정의함. 벡터 테이블에 표준 이름으로 weak alias를 걸어뒀던 게 조용히 `Default_Handler`(빈 무한루프)로 링크되면서, `vTaskStartScheduler()` 호출 후 태스크가 한 줄도 실행되지 않는 상태가 발생. 크래시 로그도 없이 조용히 멈춰서 원인 파악에 시간이 걸렸고, `arm-none-eabi-nm`으로 `port.c.obj`의 실제 익스포트 심볼을 직접 확인해 원인을 특정함.

## 심화 목표

- **WCET 측정**: DWT `CYCCNT` 레지스터를 태스크 진입/종료 시점에서 직접 읽어 실행 시간 측정
- **스택 워터마크 튜닝**: 위 실측값 기준으로 각 태스크 스택 크기 재조정
- **우선순위 역전 재현**: 공유 자원 접근 패턴을 의도적으로 구성해 우선순위 역전 상황을 재현하고, 뮤텍스 우선순위 상속으로 해소되는 과정을 확인