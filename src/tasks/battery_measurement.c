/* src/tasks/battery_meas.c
 *
 * 셀 전압/전류/온도를 주기적으로 측정해 xQueueBatteryData로 전달. */

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bms_types.h"
#include "battery_sim.h"

#define BATTERY_MEAS_PERIOD_MS 100

void vBatteryMeasTask(void *pvParameters)
{
    printf("BatteryMeas tick\n");
    (void)pvParameters;

    BMS_SimInit();

    TickType_t xLastWakeTime = xTaskGetTickCount();

    BatteryData_t xData;

    for (;;) {
        BMS_SimReadBatteryData(&xData);
        xData.timestamp_ms = xLastWakeTime * portTICK_PERIOD_MS;

        if (xQueueSend(xQueueBatteryData, &xData, 0) != pdPASS) {
            BatteryData_t xDiscard;
            xQueueReceive(xQueueBatteryData, &xDiscard, 0);
            xQueueSend(xQueueBatteryData, &xData, 0);
        }

        printf("%d %d %d\n", xData.cell_voltage_mv[0], xData.cell_voltage_mv[1], xData.cell_voltage_mv[2]);
        printf("%d %d %d\n", xData.cell_temperature_c10[0], xData.cell_temperature_c10[1], xData.cell_temperature_c10[2]);
        printf("%d\n", xData.pack_current_ma);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BATTERY_MEAS_PERIOD_MS));
    }
}