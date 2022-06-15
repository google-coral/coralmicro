add_executable_m7(tflm_hello_world
    hello_tensorflow_freertos.cc
    hello_world_model.cc
)

target_link_libraries(tflm_hello_world
    libs_base-m7_freertos
    libs_tensorflow-m7
)
