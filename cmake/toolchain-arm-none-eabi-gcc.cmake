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

find_program(CMAKE_MAKE_PROGRAM make REQUIRED)
find_program(CMAKE_XXD_PROGRAM xxd REQUIRED)

set(COMMON_C_FLAGS
    "-Wall"
    "-mthumb"
    "-fno-common"
    "-ffunction-sections"
    "-fdata-sections"
    "-ffreestanding"
    "-fno-builtin"
    "-mapcs-frame"
    "--specs=nano.specs"
    "--specs=nosys.specs"
)
list(JOIN COMMON_C_FLAGS " " COMMON_C_FLAGS_STR)

set(COMMON_C_FLAGS_DEBUG
    "-g"
    "-u _printf_float"
    "-O0"
)
list(JOIN COMMON_C_FLAGS_DEBUG " " COMMON_C_FLAGS_DEBUG_STR)

set(COMMON_C_FLAGS_RELEASE
    "-Os"
)
list(JOIN COMMON_C_FLAGS_RELEASE " " COMMON_C_FLAGS_RELEASE_STR)

set(CM7_C_FLAGS
    "-mcpu=cortex-m7"
    "-mfloat-abi=hard"
    "-mfpu=fpv5-d16"
)
list(JOIN CM7_C_FLAGS " " CM7_C_FLAGS_STR)

set(COMMON_LINK_FLAGS
    "-Xlinker" "--defsym=__use_shmem__=1"
)
list(JOIN COMMON_LINK_FLAGS " " COMMON_LINK_FLAGS_STR)

set(CM7_LINK_FLAGS
    "-mcpu=cortex-m7"
    "-mfloat-abi=hard"
    "-mfpu=fpv5-d16"
    "-T${CMAKE_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_flexspi_nor.ld"
)
list(JOIN CM7_LINK_FLAGS " " CM7_LINK_FLAGS_STR)

set(CM4_C_FLAGS
    "-mcpu=cortex-m4"
    "-mfloat-abi=hard"
    "-mfpu=fpv4-sp-d16"
)
list(JOIN CM4_C_FLAGS " " CM4_C_FLAGS_STR)

set(CM4_LINK_FLAGS
    "-mcpu=cortex-m4"
    "-mfloat-abi=hard"
    "-mfpu=fpv4-sp-d16"
    "-T${CMAKE_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm4_ram.ld"
)
list(JOIN CM4_LINK_FLAGS " " CM4_LINK_FLAGS_STR)

unset(CMAKE_C_FLAGS)
unset(CMAKE_C_FLAGS_DEBUG)
unset(CMAKE_C_FLAGS_RELEASE)
unset(CMAKE_CXX_FLAGS)
unset(CMAKE_CXX_FLAGS_DEBUG)
unset(CMAKE_CXX_FLAGS_RELEASE)
unset(CMAKE_EXE_LINKER_FLAGS)
set(CMAKE_C_FLAGS "${COMMON_C_FLAGS_STR} -std=gnu99" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG "${COMMON_C_FLAGS_DEBUG_STR}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${COMMON_C_FLAGS_RELEASE_STR}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${COMMON_C_FLAGS_STR} -fno-rtti -fno-exceptions -std=gnu++11" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_C_FLAGS_DEBUG_STR}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_C_FLAGS_RELEASE_STR}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${COMMON_LINK_FLAGS_STR} -Xlinker --gc-sections -Xlinker -Map=output.map" CACHE STRING "" FORCE)

function(add_executable_m7)
    add_executable(${ARGV})
    target_compile_options(${ARGV0} PUBLIC ${CM7_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM7_LINK_FLAGS})
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${ARGV0} image.hex
        DEPENDS ${ARGV0}
    )
endfunction()

function(add_library_m7)
    add_library(${ARGV})
    target_compile_options(${ARGV0} PUBLIC ${CM7_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM7_LINK_FLAGS})
endfunction()

function(add_executable_m4)
    add_executable(${ARGV})
    target_compile_options(${ARGV0} PUBLIC ${CM4_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM4_LINK_FLAGS})
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ARGV0} ${ARGV0}.bin
        COMMAND ${CMAKE_OBJCOPY}
            -I binary
            --rename-section .data=.core1_code
            -O elf32-littlearm
            --redefine-sym _binary_${ARGV0}_bin_start=m4_binary_start
            --redefine-sym _binary_${ARGV0}_bin_end=m4_binary_end
            --redefine-sym _binary_${ARGV0}_bin_size=m4_binary_size
            ${ARGV0}.bin ${ARGV0}.obj
        DEPENDS ${ARGV0}
        BYPRODUCTS ${ARGV0}.bin ${ARGV0}.obj
    )
    add_custom_command(OUTPUT ${ARGV0}.obj
        DEPENDS ${ARGV0}
    )
endfunction()

function(add_library_m4)
    add_library(${ARGV})
    target_compile_options(${ARGV0} PUBLIC ${CM4_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM4_LINK_FLAGS})
endfunction()
