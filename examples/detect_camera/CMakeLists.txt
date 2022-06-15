add_executable_m7(detect_camera
    detect_camera.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite
)

target_link_libraries(detect_camera
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
    libs_tensorflow-m7
    libs_tpu_freertos
)
