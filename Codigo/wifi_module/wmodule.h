#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#ifndef wmodule_h
#define wmodule_h

void uart_init_wifi();
bool send_sensor_values(const char *data);
bool sendCMD(const char *cmd, const char *act);
void connectToWifi();

#endif