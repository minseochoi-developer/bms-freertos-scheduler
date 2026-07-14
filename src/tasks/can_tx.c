/* src/tasks/can_tx.c
 *
 * xQueueSystemState / xQueueRelayCommand를 CAN 프레임으로 구성해 로그로
 * 출력. 두 큐가 각각 depth 1 overwrite라 하나의 태스크가 둘 다 블로킹으로
 * 기다리기 애매함 — 대신 고정 주기로 깨어나 두 큐를 논블로킹으로 확인하고,
 * 새 값이 있으면 로컬 상태 갱신, 없어도 마지막 값으로 매 주기 프레임을
 * 내보내는 방식(실제 CAN 네트워크의 주기적 하트비트 프레임과 유사).
 *
 * 직접 구현할 것.
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"
#include "can_frame.h"

#define CAN_TX_PERIOD_MS   100

void vCANTxTask(void *pvParameters)
{
    (void) pvParameters;

    /* 마지막으로 알려진 상태 — 큐에 새 값 없으면 이걸로 계속 전송. */
    SystemState_t xLastSystemState = { .state = BMS_STATE_INIT };
    RelayCommand_t xLastRelayCommand = { 0 };

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        /* TODO:
         * 1. xQueueSystemState, xQueueRelayCommand를 각각 xQueueReceive로
         *    타임아웃 0(논블로킹) 확인 — pdPASS면 xLastSystemState/
         *    xLastRelayCommand 갱신, 실패하면 이전 값 유지
         * 2. xLastSystemState -> CanFrame_t (CAN_ID_SYSTEM_STATE) 구성
         *    - data[0]에 state 값, data[1..4]에 fault_flags,
         *      data[5..8]에 warn_flags 등 바이트 단위로 어떻게
         *      패킹할지 직접 결정
         * 3. xLastRelayCommand -> CanFrame_t (CAN_ID_RELAY_COMMAND) 구성
         * 4. 두 프레임을 printf로 로그 출력 (id, dlc, data 바이트 hex)
         *    printf 동작 확인: semihosting_io.c가 _write()를 세미호스팅으로
         *    연결해뒀으니 QEMU 콘솔에 그대로 찍혀야 함
         */

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_TX_PERIOD_MS));
    }
}
