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
    printf("CANTx tick\n");
    (void) pvParameters;

    /* 마지막으로 알려진 상태 — 큐에 새 값 없으면 이걸로 계속 전송. */
    SystemState_t xLastSystemState = { 0 };
    RelayCommand_t xLastRelayCommand = { 0 };

    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        xQueueReceive(xQueueSystemStateForCANTx, &xLastSystemState, 0);
        xQueueReceive(xQueueRelayCommand, &xLastRelayCommand, 0);

        CanFrame_t canFrameState = {.id = CAN_ID_SYSTEM_STATE,
                               .dlc = 3,
                               .data = {xLastSystemState.state,
                                        xLastSystemState.warn_flags,
                                        xLastSystemState.fault_flags}
                              };
        
        CanFrame_t canFrameRelay = {.id = CAN_ID_RELAY_COMMAND,
                                    .dlc = 2,
                                    .data = { xLastRelayCommand.main_relay_on,
                                        xLastRelayCommand.precharge_relay_on}
                                   };

        printf("[CAN] ID=0x%03lX DLC=%u DATA=%02X %02X %02X\n",
       (unsigned long)canFrameState.id, canFrameState.dlc,
       canFrameState.data[0], canFrameState.data[1], canFrameState.data[2]);

       printf("[CAN] ID=0x%03lX DLC=%u DATA=%02X %02X\n\n",
       (unsigned long)canFrameRelay.id, canFrameRelay.dlc,
       canFrameRelay.data[0], canFrameRelay.data[1]);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CAN_TX_PERIOD_MS));
    }
}
