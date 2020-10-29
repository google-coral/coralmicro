cmake_minimum_required(VERSION 3.13)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

get_filename_component(CMAKE_C_COMPILER ${CMAKE_SOURCE_DIR}/third_party/toolchain/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc REALPATH)
get_filename_component(CMAKE_CXX_COMPILER ${CMAKE_SOURCE_DIR}/third_party/toolchain/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-g++ REALPATH)

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-libgcc-file-name
    OUTPUT_VARIABLE CMAKE_FIND_ROOT_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(CMAKE_FIND_ROOT_PATH
    "${CMAKE_FIND_ROOT_PATH}" PATH
)
get_filename_component(CMAKE_FIND_ROOT_PATH
    "${CMAKE_FIND_ROOT_PATH}/.." REALPATH
)

message(STATUS "Toolchain prefix: ${CMAKE_FIND_ROOT_PATH}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

find_program(CMAKE_MAKE_PROGRAM make)

unset(CMAKE_C_FLAGS)
unset(CMAKE_CXX_FLAGS)
unset(CMAKE_EXE_LINKER_FLAGS)
set(CMAKE_C_FLAGS "-mcpu=cortex-m7 -Wall -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb -fno-common -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mapcs-frame -std=gnu99 --specs=nano.specs --specs=nosys.specs -DCPU_MIMXRT1176DVMAA_cm7" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m7 -Wall -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb -fno-common -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mapcs-frame -fno-rtti -fno-exceptions -std=gnu++11 --specs=nano.specs --specs=nosys.specs -DCPU_MIMXRT1176DVMAA_cm7" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/MIMXRT1176xxxxx_cm7_flexspi_nor.ld" CACHE STRING "" FORCE)
