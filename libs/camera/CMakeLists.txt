add_library_m7(libs_camera_freertos STATIC
    camera.cc
)
target_link_libraries(libs_camera_freertos
    libs_base-m7_freertos
    libs_tasks_PmicTask_freertos
)

add_library_m4(libs_camera_freertos-m4 STATIC
    camera.cc
)
target_link_libraries(libs_camera_freertos-m4
    libs_base-m4_freertos
    libs_tasks_PmicTask_freertos-m4
)
