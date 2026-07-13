# BMS FreeRTOS Scheduler

AUTOSAR 기반 BMS(Battery Management System) 프로젝트의 태스크 로직을 FreeRTOS 태스크 구조로 재구현하는 프로젝트. QEMU(MPS2-AN385, Cortex-M3) 상에서 동작하며, 실제 하드웨어 없이 RTOS 스케줄링 검증이 가능한 형태로 구성.

## 배경

기존 AUTOSAR BMS 프로젝트는 AUTOSAR OS/RTE 기반으로 구현했음. 이 프로젝트는 동일한 BMS 로직(전압/전류/온도 측정, Fault 진단, 상태 관리, 릴레이 제어)을 범용 RTOS인 FreeRTOS로 재구현해, AUTOSAR라는 특정 프레임워크에 종속되지 않은 RTOS 설계 역량과, 스케줄링/동기화/타이밍 분석 관점의 이해를 보여주는 것을 목표로 함.

## 환경

- MacBook (Apple Silicon)
- arm-none-eabi-gcc 15.2 (ARM GNU Toolchain)
- QEMU 11.0.2 (MPS2-AN385 머신 모델, Cortex-M3)
- CMake + Ninja
- FreeRTOS-Kernel (별도 clone, 아래 빌드 방법 참조)

## 아키텍처

6개 태스크가 큐를 통해 데이터를 주고받는 파이프라인 구조:

```
BatteryMeas ──(BatteryData)──> FaultDiag ──(FaultStatus)──> StateMachine
                                                                   │
                                                          (SystemState)
                                                                   │
                                                                   ▼
                                                           RelayDecision ──(RelayCommand)──> CANTx
                                                                                                 ▲
                                                                           SysMonitor ───────────┘
                                                                    (전체 태스크 상태 모니터링)
```

| 태스크 | 역할 | 주기 |
|---|---|---|
| BatteryMeas | 셀 전압/전류/온도 측정 | 100ms |
| FaultDiag | 임계값 기반 이상 진단 | TBD |
| StateMachine | BMS 전체 상태(NORMAL/WARNING/FAULT 등) 관리 | TBD |
| RelayDecision | 상태 기반 릴레이 on/off 결정 | TBD |
| CANTx | 상태/명령을 CAN 프레임으로 송신 | TBD |
| SysMonitor | 태스크 상태·스택 워터마크 감시 | TBD |

우선순위 배정은 태스크 구현 완료 후 확정 예정.

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
종료: `Ctrl+A` 누른 뒤 `X`.

## 진행 상황

- [x] 툴체인 / CMake 빌드 시스템
- [x] 링커 스크립트 / startup 코드 (MPS2-AN385)
- [x] FreeRTOS 커널 연동, 부팅 검증 완료
- [ ] 태스크 간 큐 설계 
- [ ] BatteryMeas 태스크 구현
- [ ] FaultDiag / StateMachine / RelayDecision / CANTx / SysMonitor 태스크 구현
- [ ] 태스크 우선순위 확정 및 근거 문서화
- [ ] WCET 측정 (DWT 사이클 카운터 기반)
- [ ] 스택 워터마크 기반 스택 크기 튜닝
- [ ] 우선순위 역전 재현 및 대응

## 심화 목표

- **WCET 측정**: DWT `CYCCNT` 레지스터를 태스크 진입/종료 시점에서 직접 읽어 실행 시간 측정
- **스택 워터마크 튜닝**: `uxTaskGetStackHighWaterMark()`로 실측 후 각 태스크 스택 크기 최적화
- **우선순위 역전 재현**: 공유 자원 접근 패턴을 의도적으로 구성해 우선순위 역전 상황을 재현하고, 뮤텍스 우선순위 상속으로 해소되는 과정을 확인