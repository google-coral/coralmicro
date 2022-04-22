set(libs_tasks_CameraTask_SOURCES
    camera_task.cc
)

add_library_m7(libs_tasks_CameraTask_freertos STATIC
    ${libs_tasks_CameraTask_SOURCES}
)
target_link_libraries(libs_tasks_CameraTask_freertos
    libs_base-m7_freertos
    libs_tasks_PmicTask_freertos
)

add_library_m4(libs_tasks_CameraTask_freertos-m4 STATIC
    ${libs_tasks_CameraTask_SOURCES}
)
target_link_libraries(libs_tasks_CameraTask_freertos-m4
    libs_base-m4_freertos
    libs_tasks_PmicTask_freertos-m4
)
