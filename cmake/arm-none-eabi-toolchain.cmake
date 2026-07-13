# cmake/arm-none-eabi-toolchain.cmake
#
# Cortex-M3 (QEMU MPS2-AN385) 크로스 컴파일 툴체인 파일.
# 사용법: cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-toolchain.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# bare-metal 타겟은 진입점/시스템콜이 없으므로 CMake의 컴파일러 검증이
# 전체 링크까지 시도하지 않도록 정적 라이브러리 빌드로 제한한다.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT TOOLCHAIN_PREFIX)
    set(TOOLCHAIN_PREFIX arm-none-eabi-)
endif()

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE FILEPATH "")
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump CACHE FILEPATH "")
set(CMAKE_SIZE         ${TOOLCHAIN_PREFIX}size    CACHE FILEPATH "")

# Cortex-M3, Thumb-2, FPU 없음 (AN385는 FPU 미탑재).
set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")

set(CMAKE_C_FLAGS_INIT   "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-common -Wall -Wextra")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS} -nostartfiles -Wl,--gc-sections -specs=nano.specs -specs=nosys.specs")

set(CMAKE_EXECUTABLE_SUFFIX ".out")