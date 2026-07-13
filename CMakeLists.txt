cmake_minimum_required(VERSION 3.20)

project(bms_freertos_scheduler C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- FreeRTOS 커널 경로 -----------------------------------------------------
# ~/FreeRTOS/FreeRTOS/Source 구조 기준. 경로가 다르면
# -DFREERTOS_KERNEL_PATH=/path/to/FreeRTOS/Source 로 오버라이드.
if(NOT FREERTOS_KERNEL_PATH)
    set(FREERTOS_KERNEL_PATH "$ENV{HOME}/FreeRTOS/FreeRTOS/Source")
endif()

if(NOT EXISTS "${FREERTOS_KERNEL_PATH}/tasks.c")
    message(FATAL_ERROR
        "FreeRTOS kernel not found at ${FREERTOS_KERNEL_PATH}. "
        "Set -DFREERTOS_KERNEL_PATH=/path/to/FreeRTOS/Source")
endif()

set(FREERTOS_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM3")
set(FREERTOS_MEMMANG_PATH "${FREERTOS_KERNEL_PATH}/portable/MemMang")

# --- FreeRTOS 커널 정적 라이브러리 -------------------------------------------
add_library(freertos_kernel STATIC
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_PORT_PATH}/port.c
    ${FREERTOS_MEMMANG_PATH}/heap_4.c
)

target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_PORT_PATH}
    ${CMAKE_SOURCE_DIR}          # FreeRTOSConfig.h
)

# --- 애플리케이션 실행 파일 ----------------------------------------------------
# add_executable()은 configure 시점에 소스 파일 존재를 검사하므로,
# 태스크 파일은 구현되는 순서대로 이 목록에 추가한다.
add_executable(bms_scheduler
    src/main.c
    src/startup_mps2_an385.c
    src/tasks/battery_measurement.c
    src/tasks/battery_sim.c
    src/tasks/fault_diag.c
    src/tasks/state_machine.c
    src/tasks/relay_decision.c
)

target_include_directories(bms_scheduler PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(bms_scheduler PRIVATE freertos_kernel)

target_link_options(bms_scheduler PRIVATE
    -T${CMAKE_SOURCE_DIR}/src/mps2_an385.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/bms_scheduler.map
)
set_target_properties(bms_scheduler PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/src/mps2_an385.ld
)

# --- ELF -> 순수 바이너리 변환 -------------------------------------------------
add_custom_command(TARGET bms_scheduler POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:bms_scheduler>
        ${CMAKE_BINARY_DIR}/bms_scheduler.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:bms_scheduler>
    COMMENT "Generating bms_scheduler.bin and printing size"
)

# --- QEMU 실행 타겟 -----------------------------------------------------------
find_program(QEMU_ARM qemu-system-arm)
if(QEMU_ARM)
    add_custom_target(run
        COMMAND ${QEMU_ARM} -M mps2-an385 -cpu cortex-m3
                -nographic -semihosting
                -kernel $<TARGET_FILE:bms_scheduler>
        DEPENDS bms_scheduler
        USES_TERMINAL
    )
else()
    message(WARNING "qemu-system-arm not found in PATH; 'run' target unavailable")
endif()