add_executable_m7(web_file_browser
    web_file_browser.cc
)

target_link_libraries(web_file_browser
    libs_base-m7_freertos
    libs_base-m7_http_server
)
