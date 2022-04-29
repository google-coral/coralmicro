add_executable_m7(MicroSpeech
    main_functions.cc # @fcc4b966f1265f466e82617020af93670141b009
    micro_speech.cc
    model.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/command_responder.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/feature_provider.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_features_generator.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/micro_features/micro_model_settings.cc
    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/examples/micro_speech/recognize_commands.cc
)

target_link_libraries(MicroSpeech
    libs_base-m7_freertos
    libs_tensorflow-m7
)
