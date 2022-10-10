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

# Toolchain must be set before project() call.
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/toolchain-arm-none-eabi-gcc.cmake)
endif()

project(CoralMicroProject)

include(third_party/github/cristianadam/bundle_static_library.cmake)

if (NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build Type" FORCE)
endif()

if (NOT DEFINED WIFI_SSID)
    set(WIFI_SSID "MyAccessPoint")
endif()
add_definitions(-DWIFI_SSID="${WIFI_SSID}")

# Leave as is for an open AP.
# If a password is specified, we try to connect as WPA2.
if (NOT DEFINED WIFI_PSK)
    set(WIFI_PSK "")
endif()
add_definitions(-DWIFI_PSK="${WIFI_PSK}")

if (NOT DEFINED CORAL_MICRO_ARDUINO)
    set(CORAL_MICRO_ARDUINO 0)
endif()
add_definitions(-DCORAL_MICRO_ARDUINO=${CORAL_MICRO_ARDUINO})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(.)

if (${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
    add_subdirectory(apps)
    add_subdirectory(examples)
else()
    add_subdirectory(apps/elf_loader)
endif()

add_subdirectory(libs)
