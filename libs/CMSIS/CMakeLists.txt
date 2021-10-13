cmake_minimum_required(VERSION 3.13)

set(libs_CMSIS_SOURCES
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/BasicMathFunctions/arm_elementwise_add_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/BasicMathFunctions/arm_elementwise_mul_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_convolve_1_x_n_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_convolve_1x1_s8_fast.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_convolve_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_convolve_wrapper_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_depthwise_conv_3x3_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_depthwise_conv_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_depthwise_conv_s8_opt.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_depthwise_conv_wrapper_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/ConvolutionFunctions/arm_nn_mat_mult_kernel_s8_s16.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/FullyConnectedFunctions/arm_fully_connected_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/NNSupportFunctions/arm_nn_mat_mult_nt_t_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/NNSupportFunctions/arm_nn_vec_mat_mult_t_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/NNSupportFunctions/arm_q7_to_q15_with_offset.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/PoolingFunctions/arm_avgpool_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/PoolingFunctions/arm_max_pool_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/SVDFunctions/arm_svdf_s8.c
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Source/SoftmaxFunctions/arm_softmax_s8.c
)

set(libs_CMSIS_INCLUDE_DIRECTORIES
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/Core/Include
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/DSP/Include
    ${VALIANT_SOURCE_DIR}/third_party/CMSIS/CMSIS/NN/Include
)

add_library_m7(libs_CMSIS-m7 STATIC
    ${libs_CMSIS_SOURCES}
)

target_include_directories(libs_CMSIS-m7 PUBLIC
    ${libs_CMSIS_INCLUDE_DIRECTORIES}
)

add_library_m4(libs_CMSIS-m4 STATIC
    ${libs_CMSIS_SOURCES}
)

target_include_directories(libs_CMSIS-m4 PUBLIC
    ${libs_CMSIS_INCLUDE_DIRECTORIES}
)
