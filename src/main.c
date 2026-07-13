/* src/main.c
 *
 * 태스크 생성 및 스케줄러 시작. 태스크 생성 로직(우선순위, 스택 크기,
 * xTaskCreate 호출)은 src/tasks/의 각 태스크가 구현되는 순서대로 채워진다.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"

void vBatteryMeasTask(void *pvParameters);
void vFaultDiagTask(void *pvParameters);
void vStateMachineTask(void *pvParameters);
void vRelayDecisionTask(void *pvParameters);
/*
void vCANTxTask(void *pvParameters);
void vSysMonitorTask(void *pvParameters);
*/

/* bms_types.h에서 extern으로 선언된 큐 핸들의 실제 정의 */
QueueHandle_t xQueueBatteryData = NULL;
QueueHandle_t xQueueFaultState = NULL;
QueueHandle_t xQueueSystemState = NULL;
QueueHandle_t xQueueRelayCommand = NULL;

static void prvCreateQueues(void) {
    xQueueBatteryData = xQueueCreate(QUEUE_LEN_BATTERY_DATA, sizeof(BatteryData_t));
    xQueueFaultState = xQueueCreate(QUEUE_LEN_FAULT_STATE, sizeof(FaultState_t));
    xQueueSystemState = xQueueCreate(QUEUE_LEN_SYSTEM_STATE, sizeof(SystemState_t));
    xQueueRelayCommand = xQueueCreate(QUEUE_LEN_RELAY_COMMAND, sizeof(RelayCommand_t));

    /* 큐 생성 실패(힙 부족) 시 configASSERT로 즉시 정지 */
    configASSERT(xQueueBatteryData);
    configASSERT(xQueueFaultState);
    configASSERT(xQueueSystemState);
    configASSERT(xQueueRelayCommand);
}

int main(void)
{
    prvCreateQueues();
    
    xTaskCreate(vBatteryMeasTask, "BatteryMeas", 256, NULL, 3, NULL);
    xTaskCreate(vFaultDiagTask, "FaultDiag", 256, NULL, 1, NULL);
    xTaskCreate(vStateMachineTask, "StateMachine", 256, NULL, 2, NULL);
    xTaskCreate(vRelayDecisionTask, "RelayDecision", 256, NULL, 4, NULL);

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