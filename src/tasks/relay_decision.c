/* src/tasks/relay_decision.c
 *
 * xQueueSystemState를 받아 릴레이 상태(RELAY_OPEN -> RELAY_PRECHARGE ->
 * RELAY_CLOSED)를 관리하고 xQueueRelayCommand로 전달.
 *
 * 프리차지 완료 판단은 실제로는 버스 전압 비교가 정석이지만, 지금
 * BatteryData_t에 버스 전압 측정 필드가 없어 고정 시간(타이머)으로
 * 단순화함 — 나중에 버스 전압 측정이 추가되면 재검토 대상.
 *
 * FAULT 상태는 어떤 릴레이 상태에 있든 즉시 RELAY_OPEN으로 강제 전이.
 *
 * 직접 구현할 것.
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"

#define PRECHARGE_DURATION_MS   500

typedef enum {
    RELAY_OPEN = 0,
    RELAY_PRECHARGE,
    RELAY_CLOSED,
} RelayInternalState_t;

void vRelayDecisionTask(void *pvParameters)
{
    printf("RelayDicision tick\n");
    (void) pvParameters;

    RelayInternalState_t eRelayState = RELAY_OPEN;
    TickType_t xPrechargeStartTick = 0;

    for (;;) {
        SystemState_t xSystemState;
        xQueueReceive(xQueueSystemStateForRelay, &xSystemState, portMAX_DELAY);

        if (xSystemState.state == BMS_STATE_FAULT) {
            eRelayState = RELAY_OPEN;
        } else if (eRelayState == RELAY_OPEN) {
            eRelayState = RELAY_PRECHARGE;
            xPrechargeStartTick = xTaskGetTickCount();
        } else if (eRelayState == RELAY_PRECHARGE && (xTaskGetTickCount() - xPrechargeStartTick > pdMS_TO_TICKS(PRECHARGE_DURATION_MS))) {
            eRelayState = RELAY_CLOSED;
        }

        RelayCommand_t relayCmd = {
            .timestamp_ms = xSystemState.timestamp_ms,
        };

        if (eRelayState == RELAY_OPEN) {
            relayCmd.main_relay_on = false;
            relayCmd.precharge_relay_on = false;
        } else if (eRelayState == RELAY_CLOSED) {
            relayCmd.main_relay_on = true;
            relayCmd.precharge_relay_on = false;
        } else if (eRelayState == RELAY_PRECHARGE) {
            relayCmd.main_relay_on = false;
            relayCmd.precharge_relay_on = true;
        }

        xQueueOverwrite(xQueueRelayCommand, &relayCmd);
    }
}