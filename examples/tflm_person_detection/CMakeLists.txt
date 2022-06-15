add_executable_m7(tflm_person_detection
    image_provider.cc
    main_functions.cc
    person_detection.cc
    test_helpers.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/person_detect_model.tflite
)

target_link_libraries(tflm_person_detection
    libs_base-m7_freertos
    libs_tensorflow-m7
)
