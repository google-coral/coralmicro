cmake_minimum_required(VERSION 3.13)
project(BlinkLED)

include_directories(.)

add_executable_m7(BlinkLEDM7 blink_led_m7.cc)

target_link_libraries(BlinkLEDM7 libs_base-m7_freertos)
