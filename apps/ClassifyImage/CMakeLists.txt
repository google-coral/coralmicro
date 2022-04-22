add_executable_m7(ClassifyImage
    classify_image.cc
    DATA
    ${VALIANT_SOURCE_DIR}/apps/ClassifyImage/cat_224x224.rgb
    ${VALIANT_SOURCE_DIR}/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite
)

target_link_libraries(ClassifyImage
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
