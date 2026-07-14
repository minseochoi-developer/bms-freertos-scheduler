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
        xQueueReceive(xQueueSystemState, &xSystemState, portMAX_DELAY);

        /* 1. xSystemState.state == BMS_STATE_FAULT 이면 eRelayState를
         *    무조건 RELAY_OPEN으로 (다른 상태에 있었든 상관없이 강제)
         * 2. 그 외 상태(NORMAL/WARNING)에서:
         *    - eRelayState가 RELAY_OPEN이면 RELAY_PRECHARGE로 전이,
         *      이때 xPrechargeStartTick = xTaskGetTickCount()로 기록
         *    - eRelayState가 RELAY_PRECHARGE면 경과시간
         *      (xTaskGetTickCount() - xPrechargeStartTick)이
         *      PRECHARGE_DURATION_MS를 넘었는지 확인, 넘었으면
         *      RELAY_CLOSED로 전이
         *    - eRelayState가 RELAY_CLOSED면 유지
         */
        if (xSystemState.state == BMS_STATE_FAULT) {
            eRelayState = RELAY_OPEN;
        } else if (eRelayState == RELAY_OPEN) {
            eRelayState = RELAY_PRECHARGE;
            xPrechargeStartTick = xTaskGetTickCount();
        } else if (eRelayState == RELAY_PRECHARGE && (xTaskGetTickCount() - xPrechargeStartTick > pdMS_TO_TICKS(PRECHARGE_DURATION_MS))) {
            eRelayState = RELAY_CLOSED;
        }

        /* 
         * 3. eRelayState를 RelayCommand_t(main_relay_on, precharge_relay_on)로
         *    매핑 (RELAY_OPEN: 둘다 false / RELAY_PRECHARGE: precharge만
         *    true / RELAY_CLOSED: main만 true)
         */
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