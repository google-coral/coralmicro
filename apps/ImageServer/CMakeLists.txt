cmake_minimum_required(VERSION 3.13)
project(ImageServer)

include_directories(.)

set(IMAGE_SERVER_SRC image_server.cc)

set(IMAGE_SERVER_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_RPCServerIOHTTP
    libs_mjson
    libs_testlib
)

# USB
add_executable_m7(ImageServerUSB
    ${IMAGE_SERVER_SRC}
)

target_link_libraries(ImageServerUSB
    ${IMAGE_SERVER_LINK_LIBRARIES}
)

# POE
add_executable_m7(ImageServerEthernet
    ${IMAGE_SERVER_SRC}
)

target_compile_definitions(ImageServerEthernet PUBLIC
    IMAGE_SERVER_ETHERNET
)

target_link_libraries(ImageServerEthernet
    ${IMAGE_SERVER_LINK_LIBRARIES}
    libs_base-m7_ethernet
)
