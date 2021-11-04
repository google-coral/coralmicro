cmake_minimum_required(VERSION 3.13)

set(VALIANT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
include(${VALIANT_SOURCE_DIR}/cmake/bundle_static_library.cmake)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${VALIANT_SOURCE_DIR}/cmake/toolchain-arm-none-eabi-gcc.cmake)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build Type" FORCE)
endif()

if (NOT DEFINED VALIANT_BOARD_REVISION)
    set(VALIANT_BOARD_REVISION P1 CACHE STRING "Board Revision" FORCE)
endif()

if (NOT DEFINED WIFI_SSID)
    SET(WIFI_SSID "MyAccessPoint")
endif()
ADD_DEFINITIONS(-DWIFI_SSID="${WIFI_SSID}")

# Leave as-is for an open AP.
# If a password is specified, we try to connect as WPA2.
if (NOT DEFINED WIFI_PSK)
    SET(WIFI_PSK "")
endif()
ADD_DEFINITIONS(-DWIFI_PSK="${WIFI_PSK}")

if (NOT DEFINED VALIANT_ARDUINO)
    SET(VALIANT_ARDUINO 0)
endif()
ADD_DEFINITIONS(-DVALIANT_ARDUINO=${VALIANT_ARDUINO})

if (NOT DEFINED DEBUG_CONSOLE_TRANSFER_BLOCKING)
    ADD_DEFINITIONS(-DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING)
endif()
set(ignore ${DEBUG_CONSOLE_TRANSFER_BLOCKING})

project(valiant)
include_directories(${VALIANT_SOURCE_DIR})

if (${CMAKE_SOURCE_DIR} STREQUAL ${VALIANT_SOURCE_DIR})
    add_subdirectory(${VALIANT_SOURCE_DIR}/apps)
else()
    add_subdirectory(${VALIANT_SOURCE_DIR}/apps/ELFLoader)
endif()

add_subdirectory(${VALIANT_SOURCE_DIR}/libs)
add_subdirectory(${VALIANT_SOURCE_DIR}/third_party/nanopb)
