add_executable_m7(camera_detection
    camera_detection.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite
)

target_link_libraries(camera_detection
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
    libs_tensorflow-m7
    libs_tpu_freertos
)
