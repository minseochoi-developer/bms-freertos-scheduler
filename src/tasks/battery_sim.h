/* src/tasks/battery_sim.h
 *
 * 가짜 배터리 센서 드라이버 (QEMU 환경, 실제 ADC 없음).
 */

#ifndef BATTERY_SIM_H
#define BATTERY_SIM_H

#include "bms_types.h"

/* 내부 시뮬레이션 상태 초기화. 태스크 시작 시 한 번 호출. */
void BMS_SimInit(void);

/* 현재 시점의 시뮬레이션된 배터리 데이터를 pxOut에 채운다. */
void BMS_SimReadBatteryData(BatteryData_t *pxOut);

#endif /* BATTERY_SIM_H */