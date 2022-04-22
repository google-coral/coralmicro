set(libs_tasks_EdgeTpuTask_SOURCES
    edgetpu_task.cc
)

add_library_m7(libs_tasks_EdgeTpuTask_freertos STATIC
    ${libs_tasks_EdgeTpuTask_SOURCES}
)
target_link_libraries(libs_tasks_EdgeTpuTask_freertos
    libs_base-m7_freertos
    libs_tpu_freertos
    libs_usb_host_edgetpu_freertos
)
