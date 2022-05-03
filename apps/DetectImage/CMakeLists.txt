add_executable_m7(DetectImage
    detect_image.cc
    DATA
    cat_300x300.rgb
    ${VALIANT_SOURCE_DIR}/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite
)

target_link_libraries(DetectImage
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
