/* src/tasks/can_frame.h
 *
 * CANTx 태스크 내부에서 쓰는 CAN 프레임 표현. 실제 CAN 컨트롤러/
 * SocketCAN 전송은 이 프로젝트 스코프 밖 — 여기선 프레임을 구성해서
 * 로그(printf, 세미호스팅)로 출력하는 데까지만 함. 실물 전송(예:
 * SocketCAN 브릿지)은 별도 프로젝트(Virtual BMS-CAN Network)의 몫.
 */

#ifndef CAN_FRAME_H
#define CAN_FRAME_H

#include <stdint.h>

/* 표준 11비트 ID, 최대 8바이트 데이터 (Classic CAN 기준). */
typedef struct {
    uint32_t id;
    uint8_t  dlc;       /* 0~8 */
    uint8_t  data[8];
} CanFrame_t;

/* CAN ID 할당표. 실제 DBC 파일로 관리하는 게 정석이지만, 지금 단계는
 * 프로젝트 안에서만 쓰는 임의 ID. */
#define CAN_ID_SYSTEM_STATE   0x100
#define CAN_ID_RELAY_COMMAND  0x101

#endif /* CAN_FRAME_H */
