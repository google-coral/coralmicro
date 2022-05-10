set(libs_cdc_eem_SOURCES
    cdc_eem.cc
)

# add_library_m7(libs_cdc_eem_bm STATIC
#     ${libs_cdc_eem_SOURCES}
# )
# target_link_libraries(libs_cdc_eem_bm
#     libs_nxp_rt1176-sdk_bm
# )

add_library_m7(libs_cdc_eem_freertos STATIC
    ${libs_cdc_eem_SOURCES}
)
target_link_libraries(libs_cdc_eem_freertos
    libs_base-m7_freertos
)
