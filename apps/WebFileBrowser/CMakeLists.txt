cmake_minimum_required(VERSION 3.13)
project(WebFileBrowser)

include_directories(.)

add_executable_m7(WebFileBrowser
    web_file_browser.cc
)

target_include_directories(WebFileBrowser PRIVATE
    .
)

target_link_libraries(WebFileBrowser
    libs_base-m7_freertos
)
