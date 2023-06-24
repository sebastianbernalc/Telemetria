add_library(gps_module gps.c gps.h)

target_link_libraries(gps_module pico_stdlib hardware_spi)

target_include_directories(gps_module PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
