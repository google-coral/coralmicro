add_executable_m7(CameraTest
    camera_test.cc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/httpd.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/fs.c
)

target_compile_definitions(CameraTest PUBLIC
    LWIP_HTTPD_CUSTOM_FILES
    LWIP_HTTPD_DYNAMIC_HEADERS
)

target_link_libraries(CameraTest
    libs_base-m7_freertos
)
