add_executable_m7(HelloTensorflowFreeRTOS
    hello_tensorflow_freertos.cc
    hello_world_model.cc
)

target_link_libraries(HelloTensorflowFreeRTOS
    libs_base-m7_freertos
    libs_tensorflow-m7
)
