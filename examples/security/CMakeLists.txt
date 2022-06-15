add_executable_m7(security
    security.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/testconv1-edgetpu.tflite
)

target_link_libraries(security
    libs_base-m7_freertos
    libs_a71ch
)
