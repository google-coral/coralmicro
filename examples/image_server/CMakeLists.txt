set(IMAGE_SERVER_SRC image_server.cc)

set(IMAGE_SERVER_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
    libs_testlib
)

# USB
add_executable_m7(image_server_usb
    ${IMAGE_SERVER_SRC}
)

target_link_libraries(image_server_usb
    ${IMAGE_SERVER_LINK_LIBRARIES}
)

# POE
add_executable_m7(image_server_ethernet
    ${IMAGE_SERVER_SRC}
)

target_compile_definitions(image_server_ethernet PUBLIC
    IMAGE_SERVER_ETHERNET
)

target_link_libraries(image_server_ethernet
    ${IMAGE_SERVER_LINK_LIBRARIES}
    libs_base-m7_ethernet
)
