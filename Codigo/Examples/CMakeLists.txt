add_executable(telemetria telemetria.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(telemetria PRIVATE pico_stdlib pico_mpu_9250 gps_module wifi_module)

pico_enable_stdio_usb(telemetria 1)
pico_enable_stdio_uart(telemetria 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(telemetria)

# add url via pico_set_program_url
example_auto_set_url(telemetria)

add_executable(calibrateSensors calibrateSensors.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(calibrateSensors PRIVATE pico_stdlib pico_mpu_9250)

pico_enable_stdio_usb(calibrateSensors 1)
pico_enable_stdio_uart(calibrateSensors 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(calibrateSensors)

# add url via pico_set_program_url
example_auto_set_url(calibrateSensors)


add_executable(objectExample objectExample.cpp)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(objectExample PRIVATE pico_stdlib pico_mpu_9250_object)

pico_enable_stdio_usb(objectExample 1)
pico_enable_stdio_uart(objectExample 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(objectExample)

# add url via pico_set_program_url
example_auto_set_url(objectExample)