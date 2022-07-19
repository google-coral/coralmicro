# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# python3 scripts/tflm-sources.py | xclip -selection c
set(libs_tensorflow_SOURCES
    debug_log.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/test_helpers.cc  # IntArrayFromInts()
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/c/common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/error_reporter.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/flatbuffer_conversions.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/op_resolver.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/core/api/tensor_utils.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/fft_util.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/filterbank_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/frontend_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_lut.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/log_scale_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/noise_reduction_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/pcan_gain_control_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_io.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/experimental/microfrontend/lib/window_util.c
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/quantization_util.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/kernels/kernel_util.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/all_ops_resolver.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/fake_micro_context.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/flatbuffer_utils.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/activations.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/activations_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/add_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/add_n.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/arg_min_max.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/assign_variable.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/broadcast_args.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/broadcast_to.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/call_once.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cast.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/ceil.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/circular_buffer_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/add.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/mul.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/comparisons.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/concatenation.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/conv_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/cumsum.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/depth_to_space.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/dequantize.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/dequantize_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/detection_postprocess.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/div.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/elementwise.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/elu.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/ethosu.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/exp.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/expand_dims.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/fill.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor_div.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/floor_mod.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/fully_connected_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/gather.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/gather_nd.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/hard_swish.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/hard_swish_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/if.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/kernel_runner.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/kernel_util.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/l2_pool_2d.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/l2norm.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/leaky_relu_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/log_softmax.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logical.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logical_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logistic.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/logistic_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/lstm_eval.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/maximum_minimum.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/micro_tensor_utils.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/mirror_pad.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/mul_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/neg.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pack.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pad.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/pooling_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/prelu.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/prelu_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/quantize.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/quantize_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/read_variable.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/reduce.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/reduce_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/reshape.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/resize_bilinear.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/round.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/shape.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/slice.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/softmax_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/space_to_depth.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/split.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/split_v.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/squared_difference.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/squeeze.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/strided_slice.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/sub.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/sub_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/svdf_common.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/tanh.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/transpose.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/transpose_conv.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/unpack.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/var_handle.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/while.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/kernels/zeros_like.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_helpers.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_allocation_info.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_context.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_error_reporter.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_graph.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_profiler.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_resource_variable.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_string.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_time.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/micro_utils.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/mock_micro_graph.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/recording_micro_allocator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/system_setup.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/schema/schema_utils.cc
)

add_library_m7(libs_tensorflow-m7 STATIC
    classification.cc
    detection.cc
    posenet.cc
    posenet_decoder.cc
    posenet_decoder_op.cc
    utils.cc
    audio_models.cc
    ${libs_tensorflow_SOURCES}
)

target_compile_definitions(libs_tensorflow-m7 PUBLIC
    CMSIS_NN
)

target_include_directories(libs_tensorflow-m7 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro
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
    TF_LITE_STRIP_ERROR_STRINGS
)

target_include_directories(libs_tensorflow-m4 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro
)

target_link_libraries(libs_tensorflow-m4
    libs_CMSIS-m4
    libs_flatbuffers
    libs_gemmlowp
    libs_kissfft-m4
    libs_ruy
)
