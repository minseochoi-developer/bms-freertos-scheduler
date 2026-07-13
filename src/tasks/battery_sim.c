/* src/tasks/battery_sim.c
 *
 * 가짜 배터리 센서 드라이버 구현. 시간에 따라 서서히 변하는 방전 곡선 +
 * 노이즈로 값을 생성하고, 낮은 확률로 fault 임계값을 넘는 값도 섞어
 * FaultDiag 테스트에 쓸 수 있게 한다.
 */

#include "battery_sim.h"
#include "bms_thresholds.h"

static uint32_t ulRandState = 0x12345678u;
static uint32_t ulDischargeStep = 0;

static uint32_t prvNextRand(void)
{
    ulRandState = ulRandState * 1103515245u + 12345u;
    return ulRandState;
}

void BMS_SimInit(void)
{
    ulRandState = 0x12345678u;
    ulDischargeStep = 0;
}

void BMS_SimReadBatteryData(BatteryData_t *pxOut)
{
    /* 시뮬레이션된 배터리 데이터를 생성 */
    uint32_t ulCycle = ulDischargeStep % 20000u;
    int32_t baseVoltage = CELL_VOLTAGE_NORMAL_MAX_MV - (int32_t)((CELL_VOLTAGE_NORMAL_MAX_MV - CELL_VOLTAGE_NORMAL_MIN_MV) * ulCycle / 20000u);

    for (int i = 0; i < BMS_NUM_CELLS; i++) {
        int32_t Noise = (int32_t)(prvNextRand() % 31u) - 15;
        int32_t voltage = baseVoltage + Noise;

        if ((prvNextRand() % 100u) == 0u) {
            voltage = CELL_VOLTAGE_FAULT_HIGH_MV + 10;
        } else if ((prvNextRand() % 100u) == 0u) {
            voltage = CELL_VOLTAGE_FAULT_LOW_MV - 10;
        }

        pxOut->cell_voltage_mv[i] = (uint16_t)voltage;
    }

    int32_t current = (ulCycle < 10000u) ? -(int32_t)(PACK_CURRENT_NORMAL_MAX_MA * 8 / 10) : (int32_t)(PACK_CURRENT_NORMAL_MAX_MA * 5 / 10);
    current += (int32_t)(prvNextRand() % 101u) - 50;
    if ((prvNextRand() % 100u) == 0u) {
        int32_t sign = (ulCycle < 10000u) ? -1 : 1;
        current = sign * (PACK_CURRENT_FAULT_MAX_MA + 200);
    }
    pxOut->pack_current_ma = (int16_t) current;

    int32_t temp = TEMPERATURE_NORMAL_MIN_C10 + (int32_t)((TEMPERATURE_NORMAL_MAX_C10 - TEMPERATURE_NORMAL_MIN_C10) * ulCycle / 20000u);
    temp += (int32_t)(prvNextRand() % 21u) - 10;

    if ((prvNextRand() % 200u) == 0u) {
        temp = TEMPERATURE_FAULT_MAX_C10 + 20;
    } else if ((prvNextRand() % 200u) == 0u) {
        temp = TEMPERATURE_FAULT_MIN_C10 - 20;
    }
    pxOut->pack_temperature_c10 = (int16_t) temp;
 
    ulDischargeStep++;
}
