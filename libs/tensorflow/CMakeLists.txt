cmake_minimum_required(VERSION 3.13)

# python3 scripts/tflm-sources.py | xclip -selection c
set(libs_tensorflow_SOURCES
    debug_log.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/test_helpers.cc  # IntArrayFromInts()
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/c/common.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/error_reporter.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/flatbuffer_conversions.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/op_resolver.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/tensor_utils.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_memmap_generator.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_lut.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_io.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_util.c
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/quantization_util.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/kernel_util.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/fake_micro_context.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/flatbuffer_utils.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/activations.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/activations_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/add_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/add_n.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/arg_min_max.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/assign_variable.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/call_once.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cast.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/ceil.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/add.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/mul.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/comparisons.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/concatenation.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/conv_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cumsum.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/depth_to_space.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/dequantize.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/dequantize_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/detection_postprocess.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/elementwise.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/elu.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/ethosu.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/exp.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/expand_dims.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/fill.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor_div.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor_mod.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/gather.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/gather_nd.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/hard_swish.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/hard_swish_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/if.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/kernel_runner.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/kernel_util.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/l2_pool_2d.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/l2norm.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/log_softmax.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logical.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logical_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logistic.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logistic_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/maximum_minimum.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/mirror_pad.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/mul_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/neg.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pack.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pad.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pooling_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/prelu.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/prelu_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/quantize.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/read_variable.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/reduce.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/reshape.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/resize_bilinear.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/round.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/shape.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/slice.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/softmax_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/space_to_depth.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/split.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/split_v.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/squeeze.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/strided_slice.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/sub.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/sub_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/svdf_common.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/tanh.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/transpose.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/transpose_conv.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/unpack.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/var_handle.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/zeros_like.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_helpers.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_allocator.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_context.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_graph.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_profiler.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_resource_variable.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_string.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_time.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_utils.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/mock_micro_graph.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/recording_micro_allocator.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/recording_simple_memory_allocator.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/simple_memory_allocator.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/system_setup.cc
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/schema/schema_utils.cc
)

add_library_m7(libs_tensorflow-m7 STATIC
    classification.cc
    detection.cc
    utils.cc
    ${libs_tensorflow_SOURCES}
)

target_compile_definitions(libs_tensorflow-m7 PUBLIC
    CMSIS_NN
)

target_include_directories(libs_tensorflow-m7 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro
)

target_link_libraries(libs_tensorflow-m7
    libs_base-m7_freertos
    libs_CMSIS-m7
    libs_flatbuffers
    libs_gemmlowp
    libs_kissfft-m7
    libs_ruy
    libs_tpu_freertos
)

add_library_m4(libs_tensorflow-m4 STATIC
    ${libs_tensorflow_SOURCES}
)

target_compile_definitions(libs_tensorflow-m4 PUBLIC
    CMSIS_NN
)

target_include_directories(libs_tensorflow-m4 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro
)

target_link_libraries(libs_tensorflow-m4
    libs_CMSIS-m4
    libs_flatbuffers
    libs_gemmlowp
    libs_kissfft-m4
    libs_ruy
)

add_library_m7(libs_tensorflow_model_hello_world STATIC
    ${VALIANT_SOURCE_DIR}/libs/tensorflow/hello_world_model.cc
)
target_include_directories(libs_tensorflow_model_hello_world PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/tflite-micro
)
