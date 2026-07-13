/* src/startup_mps2_an385.c
 *
 * QEMU MPS2-AN385 타겟용 Cortex-M3 벡터 테이블 및 리셋 핸들러.
 * mps2_an385.ld와 두 지점에서 반드시 일치해야 함:
 *   1) vector_table[0] == _estack (초기 MSP)
 *   2) 이 파일의 .isr_vector 섹션이 링커 스크립트에 의해 FLASH 시작
 *      주소에 배치됨
 */

#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

/* 코어 예외. fault 계열은 디버깅을 위해 개별 심볼을 유지하고,
 * 나머지는 Default_Handler로 약한 별칭 처리한다. */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void);
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* FreeRTOS Cortex-M3 포트(port.c)가 이 세 개를 강한 심볼로 정의하므로,
 * 여기서는 약한 별칭만 두고 port.o가 링크에 포함되면 그 정의로 대체된다. */
void SVC_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* CMSDK/AN385 외부 인터럽트 (IRQ0~31), 현재 미사용. */
void IRQ0_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ1_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ2_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ3_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ4_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ5_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ6_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ7_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ8_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ9_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void IRQ10_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ11_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ12_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ13_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ14_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ15_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ16_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ17_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ18_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ19_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ20_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ21_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ22_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ23_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ24_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ25_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ26_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ27_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ28_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ29_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ30_Handler(void) __attribute__((weak, alias("Default_Handler")));
void IRQ31_Handler(void) __attribute__((weak, alias("Default_Handler")));

typedef void (*vector_entry_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_entry_t vector_table[] = {
    (vector_entry_t)&_estack,   /* 0  초기 MSP */
    Reset_Handler,               /* 1  */
    NMI_Handler,                  /* 2  */
    HardFault_Handler,            /* 3  */
    MemManage_Handler,            /* 4  */
    BusFault_Handler,             /* 5  */
    UsageFault_Handler,           /* 6  */
    0, 0, 0, 0,                   /* 7-10 예약 */
    SVC_Handler,                  /* 11 */
    DebugMon_Handler,             /* 12 */
    0,                             /* 13 예약 */
    PendSV_Handler,               /* 14 */
    SysTick_Handler,              /* 15 */
    IRQ0_Handler,  IRQ1_Handler,  IRQ2_Handler,  IRQ3_Handler,
    IRQ4_Handler,  IRQ5_Handler,  IRQ6_Handler,  IRQ7_Handler,
    IRQ8_Handler,  IRQ9_Handler,  IRQ10_Handler, IRQ11_Handler,
    IRQ12_Handler, IRQ13_Handler, IRQ14_Handler, IRQ15_Handler,
    IRQ16_Handler, IRQ17_Handler, IRQ18_Handler, IRQ19_Handler,
    IRQ20_Handler, IRQ21_Handler, IRQ22_Handler, IRQ23_Handler,
    IRQ24_Handler, IRQ25_Handler, IRQ26_Handler, IRQ27_Handler,
    IRQ28_Handler, IRQ29_Handler, IRQ30_Handler, IRQ31_Handler,
};

void Reset_Handler(void)
{
    /* .data 초기값을 FLASH에서 RAM으로 복사. */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* .bss를 0으로 초기화. */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();

    /* main()은 정상적으로 리턴하지 않음. */
    while (1) {
        __asm volatile ("nop");
    }
}

void Default_Handler(void)
{
    while (1) {
        __asm volatile ("nop");
    }
}

void HardFault_Handler(void)
{
    while (1) {
        __asm volatile ("nop");
    }
}