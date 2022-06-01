add_library_m7(libs_coremark-m7 STATIC
    core_coral_micro.c
    core_main.c
    core_portme.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_util.c
)

target_link_libraries(libs_coremark-m7
    libs_base-m7_freertos
)

target_include_directories(libs_coremark-m7 PRIVATE
    .
    ${PROJECT_SOURCE_DIR}/third_party/coremark
)

target_compile_definitions(libs_coremark-m7 PRIVATE
    main=coremark_main
)

add_library_m4(libs_coremark-m4 STATIC
    core_coral_micro.c
    core_main.c
    core_portme.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_util.c
)

target_link_libraries(libs_coremark-m4
    libs_base-m4_freertos
)

target_include_directories(libs_coremark-m4 PRIVATE
    .
    ${PROJECT_SOURCE_DIR}/third_party/coremark
)

target_compile_definitions(libs_coremark-m4 PRIVATE
    main=coremark_main
)
