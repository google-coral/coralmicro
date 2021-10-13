cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_coremark-m7 STATIC
    core_portme.c
    core_valiant.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_main.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_util.c
)

target_link_libraries(libs_coremark-m7
    libs_base-m7_freertos
)

target_include_directories(libs_coremark-m7 PRIVATE
    .
    ${VALIANT_SOURCE_DIR}/third_party/coremark
)

target_compile_definitions(libs_coremark-m7 PRIVATE
    main=coremark_main
)

add_library_m4(libs_coremark-m4 STATIC
    core_portme.c
    core_valiant.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_main.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${VALIANT_SOURCE_DIR}/third_party/coremark/core_util.c
)

target_link_libraries(libs_coremark-m4
    libs_base-m4_freertos
)

target_include_directories(libs_coremark-m4 PRIVATE
    .
    ${VALIANT_SOURCE_DIR}/third_party/coremark
)

target_compile_definitions(libs_coremark-m4 PRIVATE
    main=coremark_main
)
