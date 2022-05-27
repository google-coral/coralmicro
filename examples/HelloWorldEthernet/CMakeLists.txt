add_executable_m7(HelloWorldEthernet
    hello_world_ethernet.cc
)

target_link_libraries(HelloWorldEthernet
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_curl
)
