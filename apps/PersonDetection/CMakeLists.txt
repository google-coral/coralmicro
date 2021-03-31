cmake_minimum_required(VERSION 3.13)
project(PersonDetection)

include_directories(.)

add_executable_m7(PersonDetection
    image_provider.cc
    main_functions.cc
    person_detection.cc
    test_helpers.cc
)

target_include_directories(PersonDetection PRIVATE
    .
)

target_link_libraries(PersonDetection
    libs_base-m7_freertos
    libs_tensorflow-m7
)

# add_executable_m4(PersonDetectionM4
#     person_detection.cc
#     person_model_grayscale/no_person_image_data.cc
#     person_model_grayscale/person_image_data.cc
#     person_model_grayscale/person_detect_model_data.cc
#     ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/detection_responder.cc
#     ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/image_provider.cc
#     ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/person_detection/main_functions.cc
# )
# 
# target_include_directories(PersonDetectionM4 PRIVATE
#     .
# )
# 
# target_link_libraries(PersonDetectionM4
#     libs_base-m4_freertos
#     libs_tensorflow-m4
# )
