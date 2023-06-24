/**
  @file gps.c
  @brief Universidad De Antioquia - Departamento de Ingenieria Electronica - Laboratorio N°3 - Telemetria - Electronica Digital III 
  Este archivo contiene las funciones para decodificar la trama del GPS y enviarla al servidor
  @author Sebastian Bernal Cuaspa & Kevin David Martinez Zapata.
  @date 8/05/2023

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

// Define uart properties for the GPS
const uint8_t DATABITS = 8;
const uint8_t STARTBITS = 1;
const uint8_t STOPBITS = 1;
const uint32_t GPS_BAUDRATE = 9600;

// Calculate the delay between bytes
const uint8_t BITS_PER_BYTE = STARTBITS + DATABITS + STOPBITS;
const uint32_t MICROSECONDS_PER_SECOND = 1000000;
const uint32_t GPS_DELAY = BITS_PER_BYTE * MICROSECONDS_PER_SECOND / GPS_BAUDRATE;

/**
 * @brief Decodifica la trama del GPS.
 *
 * Esta función decodifica la trama del GPS y obtiene la latitud y longitud. Y no retorna nada. por lo que se debe usar punteros para obtener los valores.
 *
 * @param char gpsString[256] este es el arreglo que contiene la trama del GPS.
 * @param float * latitud este es el puntero a la variable que contiene la latitud.
 * @param float * longitud este es el puntero a la variable que contiene la longitud.
 */
// Define uart properties for the ESP
void decode(char gpsString[256], float * latitud, float * longitud) { // Function to decode the GPS string
  char *token = strtok(gpsString, ","); // Split the string by commas
  char *token_old, *latitude = 0, *longitude = 0; // Variables to store the latitude and longitude
/**
 * @brief itera sobre la trama del GPS.
 *
 * Este while itera sobre la trama del GPS y obtiene la latitud y longitud.
 *
 */
  while (token != NULL) { // Iterate over the string
    if (strcmp(token, "N") == 0) { // If the token is N, the next token is the latitude
      latitude = token_old; // Store the latitude
    }
    else if (strcmp(token, "W") == 0) { // If the token is W, the next token is the longitude
      longitude = token_old; // Store the longitude
    }
    token_old = token; // Store the current token
    token = strtok(NULL, ","); // Get the next token
  }

  if (latitude != NULL && longitude != NULL){ // If the latitude and longitude are not null, decode them
    float lat = atof(latitude); // Convert the latitude and longitude to float
    int aux = (int)lat/100; // Get the integer part of the latitude and longitude
    *latitud = (lat-(100*aux))/60 + (float)aux; // Convert the latitude and longitude to decimal degrees
    
    float lon = -atof(longitude); // Convert the latitude and longitude to float
    int aux2 = (int)lon/100; // Get the integer part of the latitude and longitude
    *longitud = (lon-(100*aux2))/60 + (float)aux2; // Convert the latitude and longitude to decimal degrees
  }
  
}

/**
 * @brief Lee una linea del uart.
 *
 * Esta función lee una linea del uart y la guarda en un arreglo. 
 *
 * @param uart_inst_t *uart este es el uart que se va a leer.
 * @param char *buffer este es el arreglo donde se va a guardar la linea.
 * @param const size_t max_length este es el tamaño maximo del arreglo.
 * @return i retorna el tamaño de la linea.
 */
// Function to read a line from the uart
size_t uart_read_line(uart_inst_t *uart, char *buffer, const size_t max_length){ // Function to read a line from the uart
    size_t i;
    // Receive the bytes with as much delay as possible without missing data
    buffer[0] = uart_getc(uart);
    for(i = 1;i < max_length - 1 && buffer[i - 1] != '\n';i++){
        sleep_us(GPS_DELAY);
        buffer[i] = uart_getc(uart);
    }

    // End the string with a terminating 0 and return the length
    buffer[i] = '\0';
    return i;
}

/**
 * @brief Si la trama es correcta.
 *
 * Esta función verifica si la trama del GPS es correcta.
 *
 * @param const char *message este es el arreglo que contiene la trama.
 * @param const size_t length este es el tamaño de la trama.
 * @return strncmp(checksum, message + i + 1, 2) == 0 retorna true si la trama es correcta.
 */

bool is_correct(const char *message, const size_t length){  // Function to check if the message is correct
    char sum = 0;
    char checksum[3];
    size_t i;

    // The message should start with $ and end with \r\n
    if(message[0] != '$' || message[length - 1] != '\n' || message[length - 2] != '\r'){
        return false;
    }

    // Calculate the checksum    
    for(i = 1;i < length && message[i] != '*';i++){
        sum ^= message[i];
    }

    // If the current character isn't *, the message doesn't contain it and is invalid
    if(message[i] != '*'){
        return false;
    }

    // Convert the checksum to a hexadecimal string
    for(size_t i = 0;i < 2;i++){
        if(sum % 16 < 10){
            checksum[1 - i] = '0' + sum % 16;
        }else{
            checksum[1 - i] = 'A' + sum % 16 - 10;
        }
        sum >>= 4;
    }
    checksum[2] = '\0';

    // Return whether the checksum is equal to the found checksum
    return strncmp(checksum, message + i + 1, 2) == 0;
}

/**
 * @brief Envia un mensaje con checksum.
 *
 * Esta función envia un mensaje con checksum para verificar que la trama sea correcta. Y no retorna nada. por lo que se debe usar punteros para obtener los valores.
 *
 * @param uart_inst_t *uart este es el uart que se va a usar.
 * @param const char *message este es el arreglo que contiene la trama.
 * @param const size_t length este es el tamaño de la trama.
 */
// Function to send a message with checksum
void send_with_checksum(uart_inst_t *uart, const char *message, const size_t length){ 
    char sum = 0;
    char checksum[3];

    // Calcute the checksum
    for(size_t i = 0;i < length && message[i] != '*';i++){
        sum ^= message[i];
    }

    // Convert the checksum to a hexadecimal string
    for(size_t i = 0;i < 2;i++){
        if(sum % 16 < 10){
            checksum[1 - i] = '0' + sum % 16;
        }else{
            checksum[1 - i] = 'A' + sum % 16 - 10;
        }
        sum >>= 4;
    }
    checksum[2] = '\0';

    // Send the message to the GPS in the expected format
    uart_putc_raw(uart, '$');
    uart_puts(uart, message);
    uart_putc(uart, '*');
    uart_puts(uart, checksum);
    uart_puts(uart, "\r\n");
}
