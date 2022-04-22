set(libs_usb_host_edgetpu_SOURCES
    usb_host_edgetpu.c
)

add_library_m7(libs_usb_host_edgetpu_bm STATIC
    ${libs_usb_host_edgetpu_SOURCES}
)
target_link_libraries(libs_usb_host_edgetpu_bm
    libs_nxp_rt1176-sdk_bm
)

add_library_m7(libs_usb_host_edgetpu_freertos STATIC
    ${libs_usb_host_edgetpu_SOURCES}
)
target_link_libraries(libs_usb_host_edgetpu_freertos
    libs_nxp_rt1176-sdk_freertos
)
