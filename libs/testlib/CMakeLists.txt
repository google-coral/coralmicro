add_library_m7(libs_testlib STATIC
    test_lib.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite
    ${PROJECT_SOURCE_DIR}/models/posenet_test_input_324.bin
)

target_link_libraries(libs_testlib PUBLIC
    libs_base-m7_freertos
    libs_mjson
    libs_audio_freertos
    libs_tensorflow-m7
    libs_testconv1
    libs_tpu_freertos
    libs_base-m7_wifi
)
