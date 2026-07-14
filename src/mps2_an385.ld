/* src/mps2_an385.ld
 *
 * QEMU MPS2-AN385 (Cortex-M3) 메모리 맵. 실제 AN385 FPGA 이미지 기준이며
 * QEMU의 mps2-an385 머신 모델이 이 맵을 그대로 구현함.
 *
 *   0x00000000 - 0x003FFFFF : ZBT SSRAM1 (코드 영역, 4MB)  -> FLASH
 *   0x20000000 - 0x203FFFFF : ZBT SSRAM2/3 (데이터 영역, 4MB) -> RAM
 *
 * heap_4.c는 정적 배열을 BSS에 할당하는 방식이라 힙 전용 메모리
 * 리전은 별도로 두지 않는다.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

/* 메인 스택(MSP) 크기. 리셋 직후부터 vTaskStartScheduler() 호출 전까지,
 * 그리고 이후에는 인터럽트 핸들러가 사용. FreeRTOS 태스크 스택(힙에서
 * 할당됨)과는 별개. */
_stack_size = 0x1000; /* 4KB */

ENTRY(Reset_Handler)

SECTIONS
{
    /* 벡터 테이블은 0x00000000에 위치해야 함 — Cortex-M3는 리셋 시
     * 이 주소에서 초기 SP와 PC를 읽는다. */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    _sidata = LOADADDR(.data); /* .data 초기값의 FLASH 상 주소 */

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* newlib _sbrk()가 힙 시작 위치로 참조하는 심볼. FreeRTOS 자체는
     * heap_4.c의 정적 배열을 쓰니 이 힙을 실제로 쓰진 않지만,
     * printf 내부 malloc 경로가 이 심볼 존재를 요구함. */
    . = ALIGN(4);
    PROVIDE(end = .);
    PROVIDE(_end = .);

    ._user_heap_stack :
    {
        . = ALIGN(8);
        . = . + _stack_size;
        . = ALIGN(8);
        _estack = .;   /* 초기 MSP. 벡터 테이블이 이 심볼을 참조함 */
    } > RAM

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }
}
