cmake_minimum_required(VERSION 3.13)
add_library_m7(libs_posenet STATIC
    posenet.cc
    posenet_decoder.cc
    posenet_decoder_op.cc
    DATA
    ${CMAKE_SOURCE_DIR}/models/posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu.tflite
    ${CMAKE_SOURCE_DIR}/models/posenet_test_input.bin
)
target_include_directories(libs_posenet PUBLIC
    .
)
target_link_libraries(libs_posenet PUBLIC
    libs_base-m7_freertos
    libs_flatbuffers
    libs_tensorflow-m7
)
set_target_properties(libs_posenet PROPERTIES LINKER_LANGUAGE CXX)
