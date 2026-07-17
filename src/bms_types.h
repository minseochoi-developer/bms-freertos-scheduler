/* src/bms_types.h
 *
 * task 간 queue로 주고받는 메시지 타입과 큐 핸들 선언
 * 큐 자체(xQueueCreate 호출)는 main.c에서 스케줄러 시작 전에 생성하고,
 * 각 task 파일은 이 헤더만 include해서 핸들을 참조한다.
 */

 #ifndef BMS_TYPES_H
 #define BMS_TYPES_H

 #include <stdint.h>
 #include <stdbool.h>
 #include "FreeRTOS.h"
 #include "queue.h"

 /* --- 셀 구성 --------------------------------------------------------------------------------- */
 #define BMS_NUM_CELLS 3

 /* BatteryData는 FaultDiag가 추세(이동평균/변화율) 판단에 쓸 수 있게 히스토리를 유지하는 큐로 감 */
 #define QUEUE_LEN_BATTERY_DATA 5
 #define QUEUE_LEN_FAULT_STATE 1
 #define QUEUE_LEN_SYSTEM_STATE 1
 #define QUEUE_LEN_RELAY_COMMAND 1

 /* --- 메시지 타입 --------------------------------------------------------------------------------- */
 typedef struct {
     uint32_t timestamp_ms;
     uint16_t cell_voltage_mv[BMS_NUM_CELLS];
     int16_t pack_current_ma;
     int16_t cell_temperature_c10[BMS_NUM_CELLS];
 } BatteryData_t;

 /* FaultDiag -> StateMachine */
 typedef enum {
    FAULT_NONE = 0,
    FAULT_OVER_VOLTAGE = (1 << 0),
    FAULT_UNDER_VOLTAGE = (1 << 1),
    FAULT_OVER_CURRENT = (1 << 2),
    FAULT_OVER_TEMPERATURE = (1 << 3),
    FAULT_UNDER_TEMPERATURE = (1 << 4),
    FAULT_CELL_IMBALANCE = (1 << 5),
    FAULT_COMMUNICATION = (1 << 6),
    FAULT_RELAY_FAILURE = (1 << 7),
} FaultFlags_t;

typedef struct {
    uint32_t timestamp_ms;
    uint8_t warn_flags;
    uint8_t fault_flags;
} FaultState_t;

/* StateMachine -> RelayDecision, CANTx, SysMonitor */
typedef enum {
    BMS_STATE_INIT = 0,
    BMS_STATE_NORMAL,
    BMS_STATE_WARNING,
    BMS_STATE_FAULT,
    BMS_STATE_SHUTDOWN
} BMSState_t;

typedef struct {
    uint32_t timestamp_ms;
    BMSState_t state;
    uint8_t warn_flags;
    uint8_t fault_flags;
} SystemState_t;

/* RelayDecision -> CANTx */
typedef struct {
    uint32_t timestamp_ms;
    bool main_relay_on;
    bool precharge_relay_on;
} RelayCommand_t;

/* --- queue handles --------------------------------------------------------------------------------- */
extern QueueHandle_t xQueueBatteryData;
extern QueueHandle_t xQueueFaultState;
extern QueueHandle_t xQueueSystemStateForRelay;
extern QueueHandle_t xQueueSystemStateForCANTx;
extern QueueHandle_t xQueueRelayCommand;

#endif