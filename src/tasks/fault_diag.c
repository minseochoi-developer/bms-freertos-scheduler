/* src/tasks/fault_diag.c
 *
 * xQueueBatteryData에서 측정값(히스토리 최대 5개)을 받아 bms_thresholds.h
 * 기준으로 이상 여부를 판단하고, 결과를 xQueueFaultState로 전달.
 */

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"
#include "bms_thresholds.h"

// test끝나면 다시 3으로 변경
#define FAULT_COUNT_THRESHOLD 3

void vFaultDiagTask(void *pvParameters)
{
    printf("FaultDiag tick\n");
    (void) pvParameters;

    /* 최근 5개의 배터리 데이터를 저장할 버퍼 */
    BatteryData_t xBatteryDataBuffer[QUEUE_LEN_BATTERY_DATA] = {0};
    int bufferIndex = 0, bufferCount = 0;

    for (;;) {
        /* 1. xQueueBatteryData에서 xQueueReceive로 값 수신 */
        xQueueReceive(xQueueBatteryData, &xBatteryDataBuffer[bufferIndex], portMAX_DELAY);

        int overVoltageFault = 0, overVoltageWarn = 0,
            underVoltageFault = 0, underVoltageWarn = 0,
            overCurrentFault = 0, overCurrentWarn = 0,
            overTempFault = 0, overTempWarn = 0,
            underTempFault = 0, underTempWarn = 0;

        if (bufferCount < QUEUE_LEN_BATTERY_DATA) {
            bufferCount++;
        }

        /* 2. 최근 5개 데이터에 대해 이상 여부 판단 */
        for (int i = 0; i < bufferCount; i++) {
            BatteryData_t *data = &xBatteryDataBuffer[i];

            for (int cell = 0; cell < BMS_NUM_CELLS; cell++) {
                if (data->cell_voltage_mv[cell] > CELL_VOLTAGE_FAULT_HIGH_MV) {
                    overVoltageFault++;
                } else if (data->cell_voltage_mv[cell] < CELL_VOLTAGE_FAULT_LOW_MV) {
                    underVoltageFault++;
                } else if (data->cell_voltage_mv[cell] > CELL_VOLTAGE_WARN_HIGH_MV) {
                    overVoltageWarn++;
                } else if (data->cell_voltage_mv[cell] < CELL_VOLTAGE_WARN_LOW_MV) {
                    underVoltageWarn++;
                }

                if (data->cell_temperature_c10[cell] > TEMPERATURE_FAULT_MAX_C10) {
                    overTempFault++;
                } else if (data->cell_temperature_c10[cell] < TEMPERATURE_FAULT_MIN_C10) {
                    underTempFault++;
                } else if (data->cell_temperature_c10[cell] > TEMPERATURE_WARN_MAX_C10) {
                    overTempWarn++;
                } else if (data->cell_temperature_c10[cell] < TEMPERATURE_WARN_MIN_C10) {
                    underTempWarn++;
                }
            }

            if (abs(data->pack_current_ma) > PACK_CURRENT_FAULT_MAX_MA) {
                overCurrentFault++;
            } else if (abs(data->pack_current_ma) > PACK_CURRENT_WARN_MAX_MA) {
                overCurrentWarn++;
            }
        }

        /* 3. fault_flags/warn_flags 결정 */
        FaultState_t xFaultState = {
            .timestamp_ms = xBatteryDataBuffer[bufferIndex].timestamp_ms,
            .warn_flags = 0,
            .fault_flags = 0
        };

        if (overVoltageFault >= FAULT_COUNT_THRESHOLD) {
            xFaultState.fault_flags |= FAULT_OVER_VOLTAGE;
        } else if (overVoltageWarn >= FAULT_COUNT_THRESHOLD) {
            xFaultState.warn_flags |= FAULT_OVER_VOLTAGE;
        } else if (underVoltageFault >= FAULT_COUNT_THRESHOLD) {
            xFaultState.fault_flags |= FAULT_UNDER_VOLTAGE;
        } else if (underVoltageWarn >= FAULT_COUNT_THRESHOLD) {
            xFaultState.warn_flags |= FAULT_UNDER_VOLTAGE;
        }

        if (overCurrentFault >= FAULT_COUNT_THRESHOLD) {
            xFaultState.fault_flags |= FAULT_OVER_CURRENT;
        } else if (overCurrentWarn >= FAULT_COUNT_THRESHOLD) {
            xFaultState.warn_flags |= FAULT_OVER_CURRENT;
        }

        if (overTempFault >= FAULT_COUNT_THRESHOLD) {
            xFaultState.fault_flags |= FAULT_OVER_TEMPERATURE;
        } else if (overTempWarn >= FAULT_COUNT_THRESHOLD) {
            xFaultState.warn_flags |= FAULT_OVER_TEMPERATURE;
        } else if (underTempFault >= FAULT_COUNT_THRESHOLD) {
            xFaultState.fault_flags |= FAULT_UNDER_TEMPERATURE;
        }
        else if (underTempWarn >= FAULT_COUNT_THRESHOLD)
        {
            xFaultState.warn_flags |= FAULT_UNDER_TEMPERATURE;
        }

        bufferIndex = (bufferIndex + 1) % QUEUE_LEN_BATTERY_DATA;
        /* 4. xQueueFaultState로 xQueueSend/xQueueOverwrite */
        xQueueOverwrite(xQueueFaultState, &xFaultState);
    }
}