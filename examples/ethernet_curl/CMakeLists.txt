add_executable_m7(ethernet_curl
    ethernet_curl.cc
)

target_link_libraries(ethernet_curl
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_curl
)
