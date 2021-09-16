cmake_minimum_required(VERSION 3.13)
add_library_m7(libs_testlib STATIC
	test_lib.cc
)

target_link_libraries(libs_testlib PUBLIC
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_base-m7_freertos
    libs_mjson
    libs_tensorflow-m7
    libs_testconv1
    libs_tpu_freertos
)
