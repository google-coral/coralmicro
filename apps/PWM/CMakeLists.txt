cmake_minimum_required(VERSION 3.13)
project(PWM)

include_directories(.)

add_executable_m7(PWM
    pwm.cc
)

target_include_directories(PWM PRIVATE
    .
)

target_link_libraries(PWM
    libs_base-m7_freertos
)
