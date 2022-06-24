add_executable_m7(http_server
    http_server.cc
)

target_link_libraries(http_server
    libs_base-m7_freertos
    libs_base-m7_http_server
)
