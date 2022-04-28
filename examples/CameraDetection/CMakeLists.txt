add_executable_m7(CameraDetection
    camera_detection.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite
)

target_link_libraries(CameraDetection
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
    libs_tensorflow-m7
    libs_tpu_freertos
)
