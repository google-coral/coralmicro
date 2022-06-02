add_executable_m7(detect_image
    detect_image.cc
    DATA
    cat_300x300.rgb
    ${PROJECT_SOURCE_DIR}/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite
)

target_link_libraries(detect_image
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)
