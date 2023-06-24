/**
  @file wifi.c
  @brief Universidad De Antioquia - Departamento de Ingenieria Electronica - Laboratorio N°3 - Telemetria - Electronica Digital III 
  Este archivo contiene las funciones para enviar los datos al servidor por medio del modulo wifi ESP8266-01S
  @author Sebastian Bernal Cuaspa & Kevin David Martinez Zapata.
  @date 8/05/2023

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "wmodule.h"

#define UART_ID2 uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE


// Define uart properties for the WIFI
#define UART_ID uart0
#define UART_TX_PIN 0
#define UART_RX_PIN 1

char SSID[] = "GLORIA";
char password[] = "43G495E336Z";
char ServerIP[] = "192.168.1.8";
char Port[] = "80";
char uart_command[50] = "";
char buf[1024] = {0};

/**
 * @brief Iiniciliza el modulo wifi.
 *
 * Esta función inicializa el modulo wifi, configura el UART no tiene parametros de entrada ni de salida.
 *
 */

void uart_init_wifi()
{
    uart_init(UART_ID, BAUD_RATE); // Set up UART

    // Set the TX and RX pins using the function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

/**
 * @brief Envia los datos del GPS y del acelerometro/gyroscopio al servidor.
 *
 * Esta función envía los valores de los sensores al servidor usando una conexión TCP.
 * Se conecta al servidor, envía los datos y cierra la conexión.
 *
 * @param char pos[] este es el arreglo que contiene los datos del GPS y del acelerometro/gyroscopio.
 * @return bool retorna un valor booleano que indica si se pudo enviar los datos o no.
 */
bool send_sensor_values(const char *data)
{
    // Open connection
    sprintf(uart_command, "AT+CIPSTART=\"TCP\",\"%s\",%s", ServerIP, Port);
    sendCMD(uart_command, "OK");

    // Send data
    sendCMD("AT+CIPMODE=1", "OK");
    sleep_ms(100);
    sendCMD("AT+CIPSEND", ">");
    sleep_ms(200);


    uart_puts(UART_ID2, data);
    sleep_ms(100);
    uart_puts(UART_ID2, "+++"); // Look for ESP docs

    // Close connection
    // Close connection
    sleep_ms(100);
    sendCMD("AT+CIPCLOSE", "OK");
    sleep_ms(100);
    sendCMD("AT+CIPMODE=0", "OK");

    return true;
}

/**
 * @brief Envia comando al ESP.
 *
 * Esta función envía un comando al ESP y espera una respuesta. 
 * 
 *
 * @param const char *cmd este es el comando que se desea enviar al ESP.
 * @param const char *act este es la respuesta que se espera del ESP.
 * @return bool retorna un valor booleano que indica si se pudo enviar el comando o no.
 */

bool sendCMD(const char *cmd, const char *act)
{
    int i = 0;
    uint64_t t = 0;

    uart_puts(UART_ID2, cmd);
    uart_puts(UART_ID2, "\r\n");

    t = time_us_64();
    while (time_us_64() - t < 2500 * 1000)
    {
        while (uart_is_readable_within_us(UART_ID2, 2000))
        {
            buf[i++] = uart_getc(UART_ID2);
        }
        if (i > 0)
        {
            buf[i] = '\0';
            printf("%s\r\n", buf);
            if (strstr(buf, act) != NULL)
            {
                return true;
            }
            else
            {
                i = 0;
            }
        }
    }
    //printf("false\r\n");
    return false;
}

/**
 * @brief Conecta el ESP a la red wifi.
 *
 * Esta función conecta el ESP a la red wifi. No tiene parámetros ni retorna nada.
 * 
 */
void connectToWifi()
{
    sendCMD("AT", "OK");
    sendCMD("AT+CWMODE=3", "OK");
    sprintf(uart_command, "AT+CWJAP=\"%s\",\"%s\"", SSID, password);
    sendCMD(uart_command, "OK");
}