#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h" 

#ifndef gps_h
#define gps_h

void decode(char gpsString[256], float * latitud, float * longitud);
size_t uart_read_line(uart_inst_t *uart, char *buffer, const size_t max_length);
bool is_correct(const char *message, const size_t length);
void send_with_checksum(uart_inst_t *uart, const char *message, const size_t length);

#endif