/* src/bms_tasks.h
 *
 * 다른 태스크의 핸들을 참조해야 하는 경우(SysMonitor의
 * uxTaskGetStackHighWaterMark() 등)를 위한 태스크 핸들 공유.
 * 큐 핸들(bms_types.h)과 동일한 패턴 — main.c가 xTaskCreate()에서
 * 실제 핸들을 받아 여기 정의하고, 필요한 태스크가 extern으로 참조.
 */

#ifndef BMS_TASKS_H
#define BMS_TASKS_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t xHandleBatteryMeas;
extern TaskHandle_t xHandleFaultDiag;
extern TaskHandle_t xHandleStateMachine;
extern TaskHandle_t xHandleRelayDecision;
extern TaskHandle_t xHandleCANTx;

#endif /* BMS_TASKS_H */