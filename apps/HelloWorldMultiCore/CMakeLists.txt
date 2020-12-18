cmake_minimum_required(VERSION 3.13)
project(HelloWorldMultiCore)

add_executable_m7(HelloWorldMultiCore
    hello_world_cm7.cc
    HelloWorldMultiCoreM4.obj
)

target_include_directories(HelloWorldMultiCore PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCore
    libs_nxp_rt1176-sdk_bm
)
target_sources(HelloWorldMultiCore PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)

add_executable_m4(HelloWorldMultiCoreM4
    hello_world_cm4.cc
)
target_include_directories(HelloWorldMultiCoreM4 PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCoreM4
    libs_nxp_rt1176-sdk_bm-m4
)
target_sources(HelloWorldMultiCoreM4 PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm-m4>)
