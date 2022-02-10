cmake_minimum_required(VERSION 3.13)
project(ClassifyImage)

include_directories(.)

add_executable_m7(ClassifyImage
    classify_image.cc
    DATA
    ${VALIANT_SOURCE_DIR}/apps/ClassifyImage/cat_224x224.rgb
    ${VALIANT_SOURCE_DIR}/models/mobilenet_v1_1.0_224_quant_edgetpu.tflite
)

target_include_directories(ClassifyImage PRIVATE
    .
)

target_link_libraries(ClassifyImage
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
