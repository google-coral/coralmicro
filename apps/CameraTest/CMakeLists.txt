add_executable_m7(CameraTest
    camera_test.cc
)

target_link_libraries(CameraTest
    libs_base-m7_freertos
    libs_base-m7_http_server
)
