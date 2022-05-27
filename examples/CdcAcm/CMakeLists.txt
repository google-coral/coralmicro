add_executable_m7(CdcAcm
    cdc_acm.cc
)

target_link_libraries(CdcAcm
    libs_base-m7_freertos
)
