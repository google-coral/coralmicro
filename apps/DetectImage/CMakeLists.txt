cmake_minimum_required(VERSION 3.13)
project(DetectImage)

include_directories(.)

add_executable_m7(DetectImage
    detect_image.cc
    DATA
    ${VALIANT_SOURCE_DIR}/apps/DetectImage/cat.rgb
    ${VALIANT_SOURCE_DIR}/models/ssdlite_mobiledet_coco_qat_postprocess_edgetpu.tflite
)

target_link_libraries(DetectImage
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
