cmake_minimum_required(VERSION 3.13)

set(YAMNET_CPU true) # If CPU YAMNet is desired, uncomment.
if(YAMNET_CPU)
    add_definitions(-DYAMNET_CPU)
    set(model_name yamnet.tflite)
else()
    set(model_name yamnet_edgetpu.tflite)
endif()

add_library_m7(libs_yamnet STATIC
    yamnet.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/${model_name}
    ${VALIANT_SOURCE_DIR}/models/yamnet_test_input.bin
)
target_include_directories(libs_yamnet PUBLIC
    .
)
target_link_libraries(libs_yamnet PUBLIC
    libs_base-m7_freertos
    libs_flatbuffers
    libs_tensorflow-m7
)
set_target_properties(libs_yamnet PROPERTIES LINKER_LANGUAGE CXX)