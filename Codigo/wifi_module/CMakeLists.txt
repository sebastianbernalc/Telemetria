add_library(wifi_module wmodule.c wmodule.h)

target_link_libraries(wifi_module pico_stdlib hardware_spi)

target_include_directories(wifi_module PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
