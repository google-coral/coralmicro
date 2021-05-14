cmake_minimum_required(VERSION 3.13)

set(libs_tensorflow_SOURCES
    debug_log.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/c/common.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/error_reporter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/flatbuffer_conversions.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/op_resolver.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/core/api/tensor_utils.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/fft.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/filterbank.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/frontend.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/log_lut.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/log_scale.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/window.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/experimental/microfrontend/lib/window_util.c
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/kernels/internal/quantization_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/kernels/kernel_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/all_ops_resolver.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/activations.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/add_n.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/arg_min_max.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/ceil.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/add.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/mul.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/comparisons.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/concatenation.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/conv_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/dequantize.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/detection_postprocess.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/div.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/elementwise.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/elu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/ethosu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/floor.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/fully_connected_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/hard_swish.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/kernel_util.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/l2_pool_2d.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/l2norm.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/leaky_relu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/logical.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/logistic.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/maximum_minimum.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/neg.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/pack.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/pad.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/prelu.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/quantize.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/quantize_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/reduce.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/reshape.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/round.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/shape.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/softmax_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/split.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/split_v.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/squeeze.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/strided_slice.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/sub.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/svdf_common.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/tanh.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/transpose_conv.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/kernels/unpack.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/memory_helpers.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_allocator.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_error_reporter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_string.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/micro_utils.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/simple_memory_allocator.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/system_setup.cc
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/schema/schema_utils.cc
)

add_library_m7(libs_tensorflow-m7 STATIC
    ${libs_tensorflow_SOURCES}
)

target_compile_definitions(libs_tensorflow-m7 PUBLIC
    CMSIS_NN
)

target_include_directories(libs_tensorflow-m7 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)

target_link_libraries(libs_tensorflow-m7
    libs_CMSIS-m7
    libs_flatbuffers
    libs_gemmlowp
    libs_kissfft-m7
    libs_ruy
)

add_library_m4(libs_tensorflow-m4 STATIC
    ${libs_tensorflow_SOURCES}
)

target_compile_definitions(libs_tensorflow-m4 PUBLIC
    CMSIS_NN
)

target_include_directories(libs_tensorflow-m4 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)

target_link_libraries(libs_tensorflow-m4
    libs_CMSIS-m4
    libs_flatbuffers
    libs_gemmlowp
    libs_kissfft-m4
    libs_ruy
)

add_library_m7(libs_tensorflow_model_hello_world STATIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/hello_world/model.cc
)
target_include_directories(libs_tensorflow_model_hello_world PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/tensorflow
)
