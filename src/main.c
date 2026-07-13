/* src/main.c
 *
 * 태스크 생성 및 스케줄러 시작. 태스크 생성 로직(우선순위, 스택 크기,
 * xTaskCreate 호출)은 src/tasks/의 각 태스크가 구현되는 순서대로 채워진다.
 */

#include "FreeRTOS.h"
#include "task.h"

/*
void vBatteryMeasTask(void *pvParameters);
void vFaultDiagTask(void *pvParameters);
void vStateMachineTask(void *pvParameters);
void vRelayDecisionTask(void *pvParameters);
void vCANTxTask(void *pvParameters);
void vSysMonitorTask(void *pvParameters);
*/

int main(void)
{
    /* BMS 태스크 6개에 대한 xTaskCreate() 호출 위치. */

    vTaskStartScheduler();

    /* 스케줄러 시작에 실패한 경우에만 도달 (예: idle/timer 태스크
     * 생성 전 힙 고갈). */
    for (;;) {
        __asm volatile ("nop");
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void) xTask;
    (void) pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;) {
        __asm volatile ("nop");
    }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;) {
        __asm volatile ("nop");
    }
}