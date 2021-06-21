cmake_minimum_required(VERSION 3.13)

include(${CMAKE_SOURCE_DIR}/cmake/bundle_static_library.cmake)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi-gcc.cmake)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build Type" FORCE)
endif()

if (NOT DEFINED VALIANT_BOARD_REVISION)
    set(VALIANT_BOARD_REVISION P0 CACHE STRING "Board Revision" FORCE)
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

project(valiant)
include_directories(.)
add_subdirectory(apps)
add_subdirectory(libs)
