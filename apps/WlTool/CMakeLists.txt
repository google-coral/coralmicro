add_executable_m7(WlTool
    wl_tool.cc
)

target_link_libraries(WlTool
    libs_base-m7_freertos
    libs_RPCServerIOHTTP
    libs_testlib
    libs_nxp_rt1176-sdk_wiced_mfgtest
)
