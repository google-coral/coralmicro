add_executable_m7(HelloTensorflowFreeRTOS
    hello_tensorflow_freertos.cc
)

target_link_libraries(HelloTensorflowFreeRTOS
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tensorflow_model_hello_world
)
