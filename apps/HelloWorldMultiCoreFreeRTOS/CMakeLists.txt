cmake_minimum_required(VERSION 3.13)
project(HelloWorldMultiCoreFreeRTOS)

add_executable_m7(HelloWorldMultiCoreFreeRTOS
    hello_world_cm7.cc
    HelloWorldMultiCoreFreeRTOSM4.obj
)

target_include_directories(HelloWorldMultiCoreFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCoreFreeRTOS
    libs_nxp_rt1176-sdk_freertos
)
target_sources(HelloWorldMultiCoreFreeRTOS PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)

add_executable_m4(HelloWorldMultiCoreFreeRTOSM4
    hello_world_cm4.cc
)
target_include_directories(HelloWorldMultiCoreFreeRTOSM4 PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCoreFreeRTOSM4
    libs_nxp_rt1176-sdk_freertos-m4
)
target_sources(HelloWorldMultiCoreFreeRTOSM4 PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos-m4>)
