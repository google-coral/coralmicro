cmake_minimum_required(VERSION 3.13)

set(libs_tensorflow_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/c/common.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/error_reporter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/flatbuffer_conversions.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/op_resolver.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/tensor_utils.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/kernels/internal/quantization_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/kernels/kernel_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/activations.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/add.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/arg_min_max.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/ceil.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/comparisons.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/concatenation.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/conv.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/depthwise_conv.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/dequantize.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/elementwise.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/ethosu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/floor.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/fully_connected.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/l2norm.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/logical.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/logistic.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/maximum_minimum.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/mul.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/neg.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/pack.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/pad.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/pooling.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/prelu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/quantize.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/reduce.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/reshape.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/round.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/softmax.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/split.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/strided_slice.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/sub.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/svdf.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/tanh.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/unpack.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/memory_helpers.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_allocator.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_optional_debug_tools.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_string.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_utils.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/simple_memory_allocator.cc
)

add_library_m7(libs_tensorflow-m7 STATIC
    ${libs_tensorflow_SOURCES}
)

target_include_directories(libs_tensorflow-m7 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)

target_link_libraries(libs_tensorflow-m7
    libs_flatbuffers
    libs_gemmlowp
    libs_ruy
)

add_library_m4(libs_tensorflow-m4 STATIC
    ${libs_tensorflow_SOURCES}
)

target_include_directories(libs_tensorflow-m4 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)

target_link_libraries(libs_tensorflow-m4
    libs_flatbuffers
    libs_gemmlowp
    libs_ruy
)

add_library_m7(libs_tensorflow_model_hello_world STATIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/hello_world/model.cc
)
target_include_directories(libs_tensorflow_model_hello_world PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)

add_library_m7(libs_tensorflow_model_testconv1 STATIC
    testconv1_edgetpu.c
    testconv1_expected_output.c
    testconv1_test_input.c
)
