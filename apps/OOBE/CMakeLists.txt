cmake_minimum_required(VERSION 3.13)
project(OOBE)

add_executable_m7(OOBE
    oobe_cm7.cc
    OOBEM4.obj
)

target_link_libraries(OOBE
    libs_base-m7_freertos
    libs_posenet
)

add_executable_m4(OOBEM4
    image_provider.cc
    oobe_cm4.cc
    main_functions.cc
)

target_link_libraries(OOBEM4
    libs_base-m4_freertos
    libs_tensorflow-m4
)
