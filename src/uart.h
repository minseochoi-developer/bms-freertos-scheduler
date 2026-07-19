/* src/uart.h
 *
 * CMSDK APB UART0 (MPS2-AN385 기본 콘솔 UART) 최소 드라이버.
 * 레지스터 레이아웃은 QEMU 소스(hw/char/cmsdk-apb-uart.c)로 직접
 * 확인함 — CMSIS 헤더 없이 메모리 매핑 주소 직접 접근.
 *
 * QEMU 실행 시 -nographic만 쓰고 -serial을 따로 안 주면, 기본으로
 * UART0이 현재 터미널(stdio)에 연결된다 — 즉 터미널에서 타이핑한
 * 문자가 그대로 UART0 RX로 들어옴.
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#define UART0_BASE       (0x40004000u)
#define UART0_DATA       (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART0_STATE      (*(volatile uint32_t *)(UART0_BASE + 0x04))
#define UART0_CTRL       (*(volatile uint32_t *)(UART0_BASE + 0x08))

#define UART_STATE_TXFULL_BIT  (1u << 0)
#define UART_STATE_RXFULL_BIT  (1u << 1)
#define UART_CTRL_TXEN_BIT     (1u << 0)
#define UART_CTRL_RXEN_BIT     (1u << 1)

static inline void UART_Init(void)
{
    UART0_CTRL |= (UART_CTRL_TXEN_BIT | UART_CTRL_RXEN_BIT);
}

//* 논블로킹 — 받은 바이트가 있으면 *pucByte에 채우고 true, 없으면 즉시 false.
static inline bool UART_TryReadByte(uint8_t *pucByte)
{
    if (UART0_STATE & UART_STATE_RXFULL_BIT) {
        *pucByte = (uint8_t) UART0_DATA;
        return true;
    }
    return false;
}

#endif /* UART_H */
