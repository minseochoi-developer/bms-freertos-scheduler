/* src/tasks/sys_monitor.c
 *
 * 1초 주기로 각 태스크의 스택 워터마크(uxTaskGetStackHighWaterMark)와
 * 시스템 힙 여유량을 로그로 출력. 스택 크기 튜닝(심화 목표)의 실측
 * 데이터를 여기서 얻음 — 값이 안정된 뒤 각 태스크 xTaskCreate()의
 */
 
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bms_types.h"
#include "bms_tasks.h"
 
#define SYS_MONITOR_PERIOD_MS   1000
 
void vSysMonitorTask(void *pvParameters)
{
    printf("sysMonitor tick\n");
    (void) pvParameters;
 
    TickType_t xLastWakeTime = xTaskGetTickCount();
 
    for (;;) {
        printf("[SysMonitor] %s : watermark = %u words\n",
       pcTaskGetName(xHandleBatteryMeas),
       (unsigned) uxTaskGetStackHighWaterMark(xHandleBatteryMeas));

       printf("[SysMonitor] %s : watermark = %u words\n",
       pcTaskGetName(xHandleFaultDiag),
       (unsigned) uxTaskGetStackHighWaterMark(xHandleFaultDiag));

       printf("[SysMonitor] %s : watermark = %u words\n",
       pcTaskGetName(xHandleStateMachine),
       (unsigned) uxTaskGetStackHighWaterMark(xHandleStateMachine));

       printf("[SysMonitor] %s : watermark = %u words\n",
       pcTaskGetName(xHandleRelayDecision),
       (unsigned) uxTaskGetStackHighWaterMark(xHandleRelayDecision));

       printf("[SysMonitor] %s : watermark = %u words\n",
       pcTaskGetName(xHandleCANTx),
       (unsigned) uxTaskGetStackHighWaterMark(xHandleCANTx));

       printf("[SysMonitor] %s : watermark = %u words\n\n",
       pcTaskGetName(NULL),
       (unsigned) uxTaskGetStackHighWaterMark(NULL));

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SYS_MONITOR_PERIOD_MS));
    }
}