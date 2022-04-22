add_executable_m7(BlinkLEDM4FromM7
    main_app_m7.cc
    M4_EXECUTABLE
    BlinkLEDM4
)

target_link_libraries(BlinkLEDM4FromM7
    libs_base-m7_freertos
)

add_executable_m4(BlinkLEDM4
    blink_led_m4.cc
)

target_link_libraries(BlinkLEDM4
    libs_base-m4_freertos
)
