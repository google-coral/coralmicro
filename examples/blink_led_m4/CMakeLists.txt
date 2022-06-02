add_executable_m7(blink_led_m4
    main_app_m7.cc
    M4_EXECUTABLE
    blink_led_m4_m4
)

target_link_libraries(blink_led_m4
    libs_base-m7_freertos
)

add_executable_m4(blink_led_m4_m4
    blink_led_m4.cc
)

target_link_libraries(blink_led_m4_m4
    libs_base-m4_freertos
)
