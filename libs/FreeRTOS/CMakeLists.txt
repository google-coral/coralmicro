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

set(libs_FreeRTOS_SOURCES
    app_callbacks.c
    multicore.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/event_groups.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/list.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F/port.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/queue.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/stream_buffer.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/tasks.c
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/timers.c
    ${PROJECT_SOURCE_DIR}/third_party/FreeRTOS_helpers/heap_useNewlib_NXP.c
)

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC
    .
    ${PROJECT_SOURCE_DIR}/third_party/modified/FreeRTOS
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/include
    ${PROJECT_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F
)

add_library_m7(libs_FreeRTOS OBJECT
    ${libs_FreeRTOS_SOURCES}
)

target_include_directories(libs_FreeRTOS PUBLIC
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC}
    PRIVATE
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE}
)

add_library_m4(libs_FreeRTOS-m4 OBJECT
    ${libs_FreeRTOS_SOURCES}
)

target_include_directories(libs_FreeRTOS-m4 PUBLIC
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC}
    PRIVATE
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE}
)
