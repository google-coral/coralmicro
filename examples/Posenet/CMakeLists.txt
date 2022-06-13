add_executable_m7(Posenet
    posenet.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite
    ${PROJECT_SOURCE_DIR}/models/posenet_test_input_324.bin
)

target_link_libraries(Posenet
    libs_base-m7_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
