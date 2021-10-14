cmake_minimum_required(VERSION 3.13)
project(PersonDetection)

include_directories(.)

add_executable_m7(PersonDetection
    image_provider.cc
    main_functions.cc
    person_detection.cc
    test_helpers.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/person_detect_model.tflite
)

target_include_directories(PersonDetection PRIVATE
    .
)

target_link_libraries(PersonDetection
    libs_base-m7_freertos
    libs_tensorflow-m7
)
