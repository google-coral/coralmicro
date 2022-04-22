add_executable_m7(mbedTLS
    mbedtls.cc
)

target_link_libraries(mbedTLS
    libs_base-m7_freertos
    libs_a71ch
)
