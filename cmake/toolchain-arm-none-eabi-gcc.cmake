# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.18)

get_filename_component(CORAL_MICRO_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> rcsP <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> rcsP <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> qcsP <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qcsP <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "")
set(CMAKE_CXX_ARCHIVE_FINISH "")

if (${CMAKE_HOST_WIN32})
    set(TOOLCHAIN_DIR ${CORAL_MICRO_SOURCE_DIR}/third_party/toolchain-win)
    set(TOOLCHAIN_PREFIX "gcc-arm-none-eabi-9-2020-q2-update")
    set(TOOLCHAIN_ARCHIVE ${TOOLCHAIN_DIR}/toolchain-win.zip)
    set(TOOLCHAIN_EXE_EXTENSION ".exe")
    set(TOOLCHAIN_URL "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-win32.zip")
elseif(${CMAKE_HOST_APPLE})
    set(TOOLCHAIN_DIR ${CORAL_MICRO_SOURCE_DIR}/third_party/toolchain-mac)
    set(TOOLCHAIN_PREFIX "")
    set(TOOLCHAIN_ARCHIVE ${TOOLCHAIN_DIR}/toolchain-mac.tar.bz2)
    set(TOOLCHAIN_EXE_EXTENSION "")
    set(TOOLCHAIN_URL "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-mac.tar.bz2")
else() # Linux
    set(TOOLCHAIN_DIR ${CORAL_MICRO_SOURCE_DIR}/third_party/toolchain-linux)
    set(TOOLCHAIN_PREFIX "")
    set(TOOLCHAIN_ARCHIVE ${TOOLCHAIN_DIR}/toolchain-linux.tar.bz2)
    set(TOOLCHAIN_EXE_EXTENSION "")
    set(TOOLCHAIN_URL "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2")
endif()

if (NOT EXISTS ${TOOLCHAIN_DIR})
    message(STATUS "Fetching ${TOOLCHAIN_URL}")
    file(DOWNLOAD ${TOOLCHAIN_URL} ${TOOLCHAIN_ARCHIVE})
    file(ARCHIVE_EXTRACT INPUT ${TOOLCHAIN_ARCHIVE} DESTINATION ${TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX})
endif()
get_filename_component(CMAKE_AR ${TOOLCHAIN_DIR}/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-ar${TOOLCHAIN_EXE_EXTENSION} REALPATH CACHE)
get_filename_component(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-gcc${TOOLCHAIN_EXE_EXTENSION} REALPATH CACHE)
get_filename_component(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-g++${TOOLCHAIN_EXE_EXTENSION} REALPATH CACHE)
get_filename_component(CMAKE_OBJCOPY ${TOOLCHAIN_DIR}/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-objcopy${TOOLCHAIN_EXE_EXTENSION} REALPATH CACHE)
get_filename_component(CMAKE_STRIP ${TOOLCHAIN_DIR}/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-strip${TOOLCHAIN_EXE_EXTENSION} REALPATH CACHE)

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

set(COMMON_C_FLAGS
    "-Wall"
    "-Wno-psabi"
    "-mthumb"
    "-fno-common"
    "-ffunction-sections"
    "-fdata-sections"
    "-ffreestanding"
    "-fno-builtin"
    "-mapcs-frame"
    "--specs=nano.specs"
    "--specs=nosys.specs"
    "-u _printf_float"
    "-ffile-prefix-map=${CMAKE_SOURCE_DIR}="
)

list(JOIN COMMON_C_FLAGS " " COMMON_C_FLAGS_STR)

set(COMMON_C_FLAGS_DEBUG
    "-g"
    "-u _printf_float"
    "-O0"
)
list(JOIN COMMON_C_FLAGS_DEBUG " " COMMON_C_FLAGS_DEBUG_STR)

set(COMMON_C_FLAGS_RELEASE
    "-g"
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
)
list(JOIN CM7_LINK_FLAGS " " CM7_LINK_FLAGS_STR)

set(CM4_C_FLAGS
    "-mcpu=cortex-m4"
    "-mfloat-abi=hard"
    "-mfpu=fpv4-sp-d16"
    "-DNDEBUG"
)
list(JOIN CM4_C_FLAGS " " CM4_C_FLAGS_STR)

set(CM4_LINK_FLAGS
    "-mcpu=cortex-m4"
    "-mfloat-abi=hard"
    "-mfpu=fpv4-sp-d16"
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
set(CMAKE_CXX_FLAGS "${COMMON_C_FLAGS_STR} -fno-rtti -fno-exceptions" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_C_FLAGS_DEBUG_STR}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_C_FLAGS_RELEASE_STR}" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${COMMON_LINK_FLAGS_STR} -Xlinker --gc-sections -Xlinker -Map=output.map" CACHE STRING "" FORCE)

function(get_path_no_split path_with_split OUT_PATH)
    string(FIND "${path_with_split}" ">" SPLIT_POS)
    if(NOT "${SPLIT_POS}" STREQUAL "-1")
        string(SUBSTRING "${path_with_split}" 0 ${SPLIT_POS} REAL_PATH)
        set("${OUT_PATH}" "${REAL_PATH}" PARENT_SCOPE)
    else()
        set("${OUT_PATH}" "${path_with_split}" PARENT_SCOPE)
    endif()
endfunction()

function(safe_configure_file src_path_with_split dst_path_with_split)
    get_path_no_split("${src_path_with_split}" SRC_PATH)
    get_path_no_split("${dst_path_with_split}" DST_PATH)
    configure_file("${SRC_PATH}" "${DST_PATH}" COPYONLY)
endfunction()

function(generate_data data_paths target)
    set(relative_data_paths "")
    foreach (data_path IN LISTS data_paths)
        if(IS_ABSOLUTE "${data_path}")
            file(RELATIVE_PATH relative_data_path "${CMAKE_SOURCE_DIR}" "${data_path}")
            set(src "${data_path}")
            set(dst "${CMAKE_BINARY_DIR}/${relative_data_path}")
        else()
            set(src "${CMAKE_CURRENT_SOURCE_DIR}/${data_path}")
            set(dst "${CMAKE_CURRENT_BINARY_DIR}/${data_path}")
            file(RELATIVE_PATH relative_data_path "${CMAKE_SOURCE_DIR}" "${src}")
        endif()
        safe_configure_file("${src}" "${dst}")
        list(APPEND relative_data_paths "${relative_data_path}")
    endforeach()
    file(GENERATE OUTPUT "${target}.data" CONTENT "${relative_data_paths}")
endfunction()

function(add_executable_m7)
    set(oneValueArgs LINKER_SCRIPT M4_EXECUTABLE)
    set(multiValueArgs DATA)
    cmake_parse_arguments(ADD_EXECUTABLE_M7 "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(LINKER_SCRIPT "-T${CORAL_MICRO_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_ram.ld")
    if (ADD_EXECUTABLE_M7_LINKER_SCRIPT)
        set(LINKER_SCRIPT "-T${ADD_EXECUTABLE_M7_LINKER_SCRIPT}")
    endif ()
    if (ADD_EXECUTABLE_M7_M4_EXECUTABLE)
        set(M4_EXECUTABLE "${ADD_EXECUTABLE_M7_M4_EXECUTABLE}.obj")
        file(GENERATE OUTPUT ${ARGV0}.m4_executable CONTENT "${ADD_EXECUTABLE_M7_M4_EXECUTABLE}")
    endif ()
    add_executable(${ADD_EXECUTABLE_M7_UNPARSED_ARGUMENTS} ${M4_EXECUTABLE})
    if (ADD_EXECUTABLE_M7_M4_EXECUTABLE)
        add_dependencies(${ARGV0} ${ADD_EXECUTABLE_M7_M4_EXECUTABLE})
    endif ()
    target_compile_options(${ARGV0} PUBLIC ${CM7_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM7_LINK_FLAGS} ${LINKER_SCRIPT})
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${ARGV0} image.hex
        DEPENDS ${ARGV0}
    )
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O srec ${ARGV0} image.srec
        DEPENDS ${ARGV0}
    )
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_STRIP} -s ${ARGV0} -o ${ARGV0}.stripped
        DEPENDS ${ARGV0}
    )
    file(GENERATE OUTPUT ${ARGV0}.libs CONTENT "$<TARGET_PROPERTY:${ARGV0},LINK_LIBRARIES>")
    generate_data("${ADD_EXECUTABLE_M7_DATA}" "${ARGV0}")
endfunction()

function(add_library_m7)
    set(multiValueArgs DATA)
    cmake_parse_arguments(ADD_LIBRARY_M7 "" "" "${multiValueArgs}" ${ARGN})
    add_library(${ADD_LIBRARY_M7_UNPARSED_ARGUMENTS})

    get_target_property(type ${ARGV0} TYPE)
    if (NOT ${type} STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${ARGV0} PUBLIC ${CM7_C_FLAGS})
        target_link_options(${ARGV0} PUBLIC ${CM7_LINK_FLAGS})
        file(GENERATE OUTPUT ${ARGV0}.libs CONTENT "$<TARGET_PROPERTY:${ARGV0},LINK_LIBRARIES>")
    endif()

    generate_data("${ADD_LIBRARY_M7_DATA}" "${ARGV0}")
endfunction()

function(add_executable_m4)
    set(multiValueArgs DATA)
    cmake_parse_arguments(ADD_EXECUTABLE_M4 "" "" "${multiValueArgs}" ${ARGN})
    add_executable(${ADD_EXECUTABLE_M4_UNPARSED_ARGUMENTS})
    target_compile_options(${ARGV0} PUBLIC ${CM4_C_FLAGS})
    target_link_options(${ARGV0} PUBLIC ${CM4_LINK_FLAGS} "-T${CORAL_MICRO_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm4_ram.ld")
    string(REGEX REPLACE "-" "_" ARGV0_UNDERSCORED ${ARGV0})
    add_custom_command(TARGET ${ARGV0} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${ARGV0} ${ARGV0}.bin
        COMMAND ${CMAKE_OBJCOPY}
            -I binary
            --rename-section .data=.core1_code
            -O elf32-littlearm
            --redefine-sym _binary_${ARGV0_UNDERSCORED}_bin_start=m4_binary_start
            --redefine-sym _binary_${ARGV0_UNDERSCORED}_bin_end=m4_binary_end
            --redefine-sym _binary_${ARGV0_UNDERSCORED}_bin_size=m4_binary_size
            ${ARGV0}.bin ${ARGV0}.obj
        DEPENDS ${ARGV0}
        BYPRODUCTS ${ARGV0}.bin ${ARGV0}.obj
    )
    file(GENERATE OUTPUT ${ARGV0}.libs CONTENT "$<TARGET_PROPERTY:${ARGV0},LINK_LIBRARIES>")

    generate_data("${ADD_EXECUTABLE_M4_DATA}" "${ARGV0}")
endfunction()

function(add_library_m4)
    set(multiValueArgs DATA)
    cmake_parse_arguments(ADD_LIBRARY_M4 "" "" "${multiValueArgs}" ${ARGN})
    add_library(${ADD_LIBRARY_M4_UNPARSED_ARGUMENTS})

    get_target_property(type ${ARGV0} TYPE)
    if (NOT ${type} STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${ARGV0} PUBLIC ${CM4_C_FLAGS})
        target_link_options(${ARGV0} PUBLIC ${CM4_LINK_FLAGS})
        file(GENERATE OUTPUT ${ARGV0}.libs CONTENT "$<TARGET_PROPERTY:${ARGV0},LINK_LIBRARIES>")
    endif()

    generate_data("${ADD_LIBRARY_M4_DATA}" "${ARGV0}")
endfunction()
