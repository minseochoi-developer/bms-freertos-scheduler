/* FreeRTOSConfig.h
 *
 * QEMU MPS2-AN385 (Cortex-M3) 타겟용 커널 설정.
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* --- 클럭 / 틱 --------------------------------------------------------- */
#define configCPU_CLOCK_HZ                     ( 25000000UL )  /* AN385 CPU 클럭 */
#define configTICK_RATE_HZ                     ( 1000 )
#define configUSE_16_BIT_TICKS                 0

/* --- 스케줄러 ------------------------------------------------------------ */
#define configUSE_PREEMPTION                   1
#define configUSE_TIME_SLICING                 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                0
#define configIDLE_SHOULD_YIELD                1

#define configMAX_PRIORITIES                   ( 7 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )  /* words, idle 태스크 */
#define configMAX_TASK_NAME_LEN                 ( 16 )

/* --- 메모리 --------------------------------------------------------------- */
#define configSUPPORT_DYNAMIC_ALLOCATION       1
#define configSUPPORT_STATIC_ALLOCATION        0
#define configTOTAL_HEAP_SIZE                  ( ( size_t ) ( 64 * 1024 ) )
#define configAPPLICATION_ALLOCATED_HEAP        0

/* --- 훅 / 진단 --------------------------------------------------------------- */
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configUSE_MALLOC_FAILED_HOOK           1

#define configCHECK_FOR_STACK_OVERFLOW          2

#define configUSE_TRACE_FACILITY               1
#define configUSE_STATS_FORMATTING_FUNCTIONS   0

/* WCET 측정은 태스크 코드에서 DWT 사이클 카운터를 직접 읽는 방식으로
 * 수행하며, 커널 내장 런타임 통계는 사용하지 않는다. */
#define configGENERATE_RUN_TIME_STATS          0

/* --- 동기화 프리미티브 --------------------------------------------------------- */
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            1
#define configUSE_COUNTING_SEMAPHORES          1
#define configUSE_QUEUE_SETS                   0
#define configQUEUE_REGISTRY_SIZE              8

/* 소프트웨어 타이머는 사용하지 않음 — 각 태스크가 vTaskDelayUntil로
 * 자체 주기를 관리한다. */
#define configUSE_TIMERS                       0
#define configTIMER_TASK_PRIORITY              ( 2 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH           ( configMINIMAL_STACK_SIZE * 2 )

/* --- 선택적 API 포함 -------------------------------------------------------- */
#define INCLUDE_vTaskPrioritySet               1
#define INCLUDE_uxTaskPriorityGet              1
#define INCLUDE_vTaskDelete                    1
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelay                     1
#define INCLUDE_xTaskGetSchedulerState         1
#define INCLUDE_xTaskGetCurrentTaskHandle      1
#define INCLUDE_uxTaskGetStackHighWaterMark    1
#define INCLUDE_eTaskGetState                  1
#define INCLUDE_xTimerPendFunctionCall         0

/* --- Cortex-M3 NVIC 우선순위 설정 ------------------------------------------------
 * AN385의 NVIC은 3비트 우선순위(8단계)를 구현함. */
#define configPRIO_BITS                        3

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY        0x07
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY   5

#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* --- assert ------------------------------------------------------------------ */
#define configASSERT( x ) \
    if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */