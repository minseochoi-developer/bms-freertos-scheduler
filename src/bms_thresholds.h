/* src/bms_thresholds.h
 *
 * 셀 전압/팩 전류/온도 기준값. 일반적인 NCM 셀 스펙 기준 임의값 */


#ifndef BMS_THRESHOLDS_H
#define BMS_THRESHOLDS_H

/* --- 셀 전압 (mV) --------------------------------------------------- */
#define CELL_VOLTAGE_NORMAL_MIN_MV      3000   /* 정상 방전 컷오프 */
#define CELL_VOLTAGE_NORMAL_MAX_MV      4200   /* 정상 충전 컷오프 */
#define CELL_VOLTAGE_WARN_LOW_MV        2900   /* 저전압 경고 (전류 제한) */
#define CELL_VOLTAGE_WARN_HIGH_MV       4230   /* 고전압 경고 (충전 전류 감축) */
#define CELL_VOLTAGE_FAULT_LOW_MV       2800   /* 이하 UNDER_VOLTAGE fault */
#define CELL_VOLTAGE_FAULT_HIGH_MV      4300   /* 이상 OVER_VOLTAGE fault */

/* --- 팩 전류 (mA, 절대값 기준) --------------------------------------- */
#define PACK_CURRENT_NORMAL_MAX_MA      2000   /* 정격 충/방전 전류 */
#define PACK_CURRENT_WARN_MAX_MA        2500   /* 정격 초과 경고 */
#define PACK_CURRENT_FAULT_MAX_MA       3000   /* 절대값 초과 시 OVER_CURRENT fault */

/* --- 온도 (0.1°C 단위) ------------------------------------------------ */
#define TEMPERATURE_NORMAL_MIN_C10      0       /* 정상 동작 최소 온도 0.0C */
#define TEMPERATURE_NORMAL_MAX_C10      450     /* 정상 동작 최대 온도 45.0C */
#define TEMPERATURE_WARN_MIN_C10        (-50)  /* 저온 경고 -5.0C */
#define TEMPERATURE_WARN_MAX_C10       550     /* 고온 경고 55.0C */
#define TEMPERATURE_FAULT_MIN_C10       (-100) /* 저온 fault -10.0C */
#define TEMPERATURE_FAULT_MAX_C10       600     /* 고온 fault 60.0C */

#endif /* BMS_THRESHOLDS_H */