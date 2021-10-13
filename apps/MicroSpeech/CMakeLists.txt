cmake_minimum_required(VERSION 3.13)
project(MicroSpeech)

include_directories(.)

add_executable_m7(MicroSpeech
    main_functions.cc # @fcc4b966f1265f466e82617020af93670141b009
    micro_speech.cc
    ${VALIANT_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/feature_provider.cc
    ${VALIANT_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_features_generator.cc
    ${VALIANT_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.cc
    ${VALIANT_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/micro_features/model.cc
    ${VALIANT_SOURCE_DIR}/third_party/tensorflow/tensorflow/lite/micro/examples/micro_speech/recognize_commands.cc
)

target_include_directories(MicroSpeech PRIVATE
    .
)

target_link_libraries(MicroSpeech
    libs_base-m7_freertos
    libs_tensorflow-m7
)
