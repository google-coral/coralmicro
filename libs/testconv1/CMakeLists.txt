add_library_m7(libs_testconv1 STATIC
    testconv1.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/testconv1-edgetpu.tflite
    ${VALIANT_SOURCE_DIR}/models/testconv1-expected-output.bin
    ${VALIANT_SOURCE_DIR}/models/testconv1-test-input.bin
)

target_include_directories(libs_testconv1 PUBLIC
    .
)

target_link_libraries(libs_testconv1 PUBLIC
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
