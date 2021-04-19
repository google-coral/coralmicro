cmake_minimum_required(VERSION 3.13)

set(libs_tpu_SOURCES
    edgetpu_executable.cc
    edgetpu_manager.cc
    edgetpu_op.cc
    tpu_driver.cc
)

set(libs_tpu_TARGET_LINK_LIBRARIES
    libs_FreeRTOS
    libs_flatbuffers
)

add_library_m7(libs_tpu_freertos STATIC
    ${libs_tpu_SOURCES}
)
target_link_libraries(libs_tpu_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_usb_host_edgetpu_freertos
    libs_tensorflow-m7
    ${libs_tpu_TARGET_LINK_LIBRARIES}
)
