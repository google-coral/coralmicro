set(libs_cdc_acm_SOURCES
    cdc_acm.cc
)

# add_library_m7(libs_cdc_acm_bm STATIC
#     ${libs_cdc_acm_SOURCES}
# )
# target_link_libraries(libs_cdc_acm_bm
#     libs_nxp_rt1176-sdk_bm
# )

add_library_m7(libs_cdc_acm_freertos STATIC
    ${libs_cdc_acm_SOURCES}
)
target_link_libraries(libs_cdc_acm_freertos
    libs_base-m7_freertos
)
