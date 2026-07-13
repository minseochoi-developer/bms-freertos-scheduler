/* src/tasks/fault_diag.c
 *
 * xQueueBatteryData에서 측정값(히스토리 최대 5개)을 받아 bms_thresholds.h
 * 기준으로 이상 여부를 판단하고, 결과를 xQueueFaultState로 전달.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"
#include "bms_thresholds.h"

void vFaultDiagTask(void *pvParameters)
{
    (void) pvParameters;

    for (;;) {
        /* TODO:
         * 1. xQueueBatteryData에서 xQueueReceive로 값 수신
         *    - 블로킹으로 기다릴지, 주기 폴링할지 결정할 것
         * 2. BMS_NUM_CELLS개 셀 전압, pack_current_ma, pack_temperature_c10을
         *    bms_thresholds.h의 기준값과 비교
         * 3. FaultFlags_t 비트를 조합해 FaultState_t 구성
         *    (여러 fault 동시 발생 가능 — 비트 OR)
         * 4. xQueueFaultState로 xQueueSend/xQueueOverwrite
         *    (depth 1 큐이므로 어떤 함수가 적절할지 생각할 것)
         */
        /* 1. xQueueBatteryData에서 xQueueReceive로 값 수신 */
        BatteryData_t xData;

        xQueueReceive(xQueueBatteryData, &xData, portMAX_DELAY);

        FaultState_t xFaultState = {
            .timestamp_ms = xData.timestamp_ms,
            .warn_flags = 0,
            .fault_flags = 0
        };

        /* 셀 전압, 전류, 온도를 bms_thresholds.h의 기준값과 비교 */
        for (int i = 0; i < BMS_NUM_CELLS; i++) {
            if (xData.cell_voltage_mv[i] < CELL_VOLTAGE_FAULT_LOW_MV) {
                xFaultState.fault_flags |= FAULT_UNDER_VOLTAGE;
            } else if (xData.cell_voltage_mv[i] < CELL_VOLTAGE_WARN_LOW_MV) {
                xFaultState.warn_flags |= FAULT_UNDER_VOLTAGE;
            } else if (xData.cell_voltage_mv[i] > CELL_VOLTAGE_FAULT_HIGH_MV) {
                    xFaultState.fault_flags |= FAULT_OVER_VOLTAGE;
            } else if (xData.cell_voltage_mv[i] > CELL_VOLTAGE_WARN_HIGH_MV) {
                    xFaultState.warn_flags |= FAULT_OVER_VOLTAGE;
            }
        }

        int16_t current_abs = (xData.pack_current_ma < 0) ? -xData.pack_current_ma : xData.pack_current_ma;
        if (current_abs > PACK_CURRENT_FAULT_MAX_MA) {
            xFaultState.fault_flags |= FAULT_OVER_CURRENT;
        } else if (current_abs > PACK_CURRENT_WARN_MAX_MA) {
            xFaultState.warn_flags |= FAULT_OVER_CURRENT;
        }

        if (xData.pack_temperature_c10 < TEMPERATURE_FAULT_MIN_C10) {
            xFaultState.fault_flags |= FAULT_UNDER_TEMPERATURE;
        } else if (xData.pack_temperature_c10 < TEMPERATURE_WARN_MIN_C10) {
            xFaultState.warn_flags |= FAULT_UNDER_TEMPERATURE;
        } else if (xData.pack_temperature_c10 > TEMPERATURE_FAULT_MAX_C10) {
            xFaultState.fault_flags |= FAULT_OVER_TEMPERATURE;
        } else if (xData.pack_temperature_c10 > TEMPERATURE_WARN_MAX_C10) {
            xFaultState.warn_flags |= FAULT_OVER_TEMPERATURE;
        }

        /* 4. xQueueFaultState로 xQueueSend/xQueueOverwrite */
        xQueueOverwrite(xQueueFaultState, &xFaultState);
    }
}