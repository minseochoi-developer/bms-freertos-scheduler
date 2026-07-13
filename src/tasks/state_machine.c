/* src/tasks/state_machine.c
 *
 * xQueueFaultState를 받아 BMS 전체 상태(BmsState_t)를 판단하고
 * xQueueSystemState로 전달.
 *
 * FAULT 상태는 래치(latch) 방식 — 한 번 진입하면 fault_flags가 다시
 * 0이 되어도 자동으로 NORMAL/WARNING으로 복귀하지 않는다. 사람 개입
 * (리셋)으로만 해제되어야 함 — 단, 이 리셋을 어디서/어떻게 트리거할지는
 * 아직 설계 안 됨 (지금 큐 구조엔 리셋 입력 경로가 없음). 우선 래치만
 * 구현하고, 리셋 경로는 별도로 다룰 것.
 */

#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"

void vStateMachineTask(void *pvParameters)
{
    (void) pvParameters;

    /* 래치 상태는 태스크 지역에 유지 (다른 태스크가 직접 건드릴 필요 없음). */
    bool bFaultLatched = false;

    for (;;) {
         /* 1. xQueueFaultState에서 xQueueReceive */
         FaultState_t xFaultState;
         xQueueReceive(xQueueFaultState, &xFaultState, portMAX_DELAY);
        
        /* 2. fault_flags != 0 이면 bFaultLatched = true 로 래치 */
         if (xFaultState.fault_flags != 0) {
             bFaultLatched = true;
         }

        /* 3. bFaultLatched 상태에 따라 BmsState_t 결정 */
         BMSState_t BMSState;
         if (bFaultLatched) {
             BMSState = BMS_STATE_FAULT;
         } else if (xFaultState.warn_flags != 0) {
             BMSState = BMS_STATE_WARNING;
         } else {
             BMSState = BMS_STATE_NORMAL;
         }

        /* 4. xQueueSystemState로 전달 */
         SystemState_t xSystemState = {
             .timestamp_ms = xFaultState.timestamp_ms,
             .state = BMSState,
             .warn_flags = xFaultState.warn_flags,
             .fault_flags = xFaultState.fault_flags
         };
         xQueueOverwrite(xQueueSystemState, &xSystemState);
    }
}