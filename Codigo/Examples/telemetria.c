/**
  @file telemetria.c
  @brief Universidad De Antioquia - Departamento de Ingenieria Electronica - Laboratorio N°3 - Telemetria - Electronica Digital III 
  En este laboratorio se desea realizar una telemetria de los datos obtenidos por el GPS y el acelerometro/gyroscopio.
  @author Sebastian Bernal Cuaspa & Kevin David Martinez Zapata.
  @date 8/05/2023

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h" 
#include "mpu9250.h"
#include "gps.h"
#include <time.h>


#define GPS_BAUDRATE 9600 // Baudrate for the GPS

//WIFI///////////////////////////////////////////////////////////////////////////
#define UART_ID uart1 
#define BAUD_RATE 115200 

// Uart pins
#define UART_TX_PIN 8
#define UART_RX_PIN 9

char SSID[] = "Sebas";  // Wifi name
char password[] = "12345678"; // Wifi password
char ServerIP[] = "192.168.70.109"; // IP of the server
char Port[] = "8080"; // Port of the server
char uart_command[50] = ""; // Buffer for the uart commands
char buf[256] = {0}; // Buffer for the uart response

bool send_sensor_values( char pos[]); // Function to send the sensor values to the server
bool sendCMD(const char *cmd, const char *act); // Function to send commands to the ESP
void connectToWifi(); //    Function to connect to the wifi



/**
 * @brief Envia los datos del GPS y del acelerometro/gyroscopio al servidor.
 *
 * Esta función envía los valores de los sensores al servidor usando una conexión TCP.
 * Se conecta al servidor, envía los datos y cierra la conexión.
 *
 * @param char pos[] este es el arreglo que contiene los datos del GPS y del acelerometro/gyroscopio.
 * @return bool retorna un valor booleano que indica si se pudo enviar los datos o no.
 */
bool send_sensor_values( char pos[])
{
    // Open connection
    sprintf(uart_command, "AT+CIPSTART=\"TCP\",\"%s\",%s", ServerIP, Port); // Look for ESP docs
    sendCMD(uart_command, "OK"); // Connect to the server

    // Send data
    sendCMD("AT+CIPMODE=1", "OK");  // Look for ESP docs
    sleep_ms(10);
    sendCMD("AT+CIPSEND", ">"); // Look for ESP docs
    sleep_ms(20);

    //printf("%s", (char*)buf);
    uart_puts(UART_ID, pos);

    sleep_ms(50);            // Seems like this is critical
    uart_puts(UART_ID, "+++"); // Look for ESP docs

    // Close connection
    //sleep_ms(1000);
    //sendCMD("AT+CIPCLOSE", "OK");
    //sleep_ms(1000);
    //sendCMD("AT+CIPMODE=0", "OK");

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
bool sendCMD(const char *cmd, const char *act) // Function to send commands to the ESP
{
    int i = 0;
    uint64_t t = 0;

    uart_puts(UART_ID, cmd); // Send the command
    uart_puts(UART_ID, "\r\n"); // Send a new line

    t = time_us_64();
    while (time_us_64() - t < 2500 * 1000) // Wait for the response
    {
        while (uart_is_readable_within_us(UART_ID, 2000)) // Read the response
        {
            buf[i++] = uart_getc(UART_ID); // Save the response in the buffer
        }
        if (i > 0)  // If there is a response
        {
            buf[i] = '\0'; // Add the end of string character
            printf("%s\r\n", buf); // Print the response
            if (strstr(buf, act) != NULL) // If the response contains the expected response
            {
                return true;    // Return true
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
 *
 *
 */
void connectToWifi() //    Function to connect to the wifi
{
    sendCMD("AT", "OK"); // Check if the ESP is working
    sendCMD("AT+CWMODE=3", "OK"); // Set the ESP as a client
    sprintf(uart_command, "AT+CWJAP=\"%s\",\"%s\"", SSID, password); // Set the SSID and password
    sendCMD(uart_command, "OK"); // Connect to the wifi
}

//WIFI///////////////////////////////////////////////////////////////////////////

// Define uart properties for the GPS
const uint8_t GPS_TX = 16, GPS_RX = 17;

/**
 * @brief Punto de entrada del programa.
 *
 * Esta funcion es el punto de entrada del programa. Inicializa los sensores y el GPS. 
 * Posteriormente, entra en un ciclo infinito donde se leen los datos del GPS y de los sensores y se envían al servidor.
 */
int main() {
    /*GPS//////////////////////////////////////////////////////////////////////////////////*/
    char message[256];
    const char CONFIGURATIONS[] = "PMTK103*30<CR><LF>"; 
    uart_inst_t *gps_uart = uart0;

    // Initialize the standard IO and the uart for the gps
    stdio_init_all(); 
    uart_init(gps_uart, GPS_BAUDRATE);

    // Don't convert \n to \r\n
    uart_set_translate_crlf(gps_uart, false);

    // Enable the uart functionality for the pins connected to the GPS
    gpio_set_function(GPS_TX, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX, GPIO_FUNC_UART);

    // Disable all types except GPRMC
    send_with_checksum(gps_uart, CONFIGURATIONS, sizeof(CONFIGURATIONS));
    /*///////////////////////////////////////////////////////////////////////////////////GPS*/
    
    //stdio_init_all();

    printf("Hello, MPU9250! Reading raw data from registers via SPI...\n");

    start_spi();  //Starts the mpu

    int16_t acceleration[3], gyro[3], eulerAngles[2], fullAngles[2], mag[3]; //Declares the variables required for calculations
    float acc_f[3], gyro_f[3], mag_f[3];
    absolute_time_t timeOfLastCheck;

    mpu9250_read_raw_accel(acceleration);  //Sets the absolute angle using the direction of gravity
    calculate_angles_from_accel(eulerAngles, acceleration);
    timeOfLastCheck = get_absolute_time();

    //WIFI///////////////////////////////////////////////////////////////////////////
    printf("\nProgram start\n");

    uart_init(UART_ID, BAUD_RATE); // Set up UART

    // Set the TX and RX pins using the function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_puts(UART_ID, "+++"); // Look for ESP docs
    sleep_ms(10);
    while (uart_is_readable(UART_ID))       // Read the response
        uart_getc(UART_ID);                // Save the response in the buffer
    sleep_ms(20);

    // Close connection
    
    connectToWifi(); 
    sleep_ms(2000);
    

    // Uncomment to get the IP address of the ESP
    // sendCMD("AT+CIFSR", "OK"); // ASK IP

    // Example values

    //WIFI///////////////////////////////////////////////////////////////////////////
    /**
 * @brief Bucle infinito.
 *
 * Este bucle while se ejecuta indefinidamente, creando un bucle infinito en el programa.
 * En este bucle se calibran los sensores, se leen los datos del GPS y de los sensores y se almacenan en variables.
 * 
 */
    while (1) {

        /*GPS//////////////////////////////////////////////////////////////////////////////////*/
        // Read a line from the GPS data
        const size_t length = uart_read_line(gps_uart, message, sizeof(message));

        // Skip to the next iteration, if the data is not correct or not of the correct type
        if(!is_correct(message, length)){
            continue;
        }
          
        // Print the received line of data
        //Compara trama para solo usar GNRMC y extraer datos
        //printf("%s", message);
        if (strncmp(message, "$GNRMC", strlen("$GNRMC")) == 0 || strncmp(message, "$GNGGA", strlen("$GNGGA")) == 0){
            float latitud = 0, longitud = 0;
            char pos[256];
            decode(message, &latitud, &longitud);

        /*///////////////////////////////////////////////////////////////////////////////////GPS*/
        
            mpu9250_read_raw_mag(mag);  //Reads the accel and gyro
            mag[0] -= 1;  //Applies the calibration
            mag[1] -= 4;
            mag[2] -= 0;

            mag_f[0] = ((float)mag[0]/32768.0) * 4800.0;
            mag_f[1] = ((float)mag[1]/32768.0) * 4800.0;
            mag_f[2] = ((float)mag[2]/32768.0) * 4800.0;

            mpu9250_read_raw_accel(acceleration);  //Reads the accel and gyro
            acceleration[0] -= 1;  //Applies the calibration
            acceleration[1] -= 8;
            acceleration[2] -= 9;

            acc_f[0] = ((float)acceleration[0]/32768.0) * 2.0;
            acc_f[1] = ((float)acceleration[1]/32768.0) * 2.0;
            acc_f[2] = ((float)acceleration[2]/32768.0) * 2.0;

            mpu9250_read_raw_gyro(gyro);
            gyro[0] -= -11;  //Applies the calibration
            gyro[1] -= -6;
            gyro[2] -= -15;

            gyro_f[0] = ((float)gyro[0]/32768.0) * 250.0;
            gyro_f[1] = ((float)gyro[1]/32768.0) * 250.0;
            gyro_f[2] = ((float)gyro[2]/32768.0) * 250.0;

  

            sprintf(pos, "%.6f,%.6f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f", latitud, longitud, acc_f[0], acc_f[1], acc_f[2], gyro_f[0], gyro_f[1], gyro_f[2], mag_f[0], mag_f[1], mag_f[2]); 
            printf("%s\n", pos);
    
            send_sensor_values(pos);
            sleep_ms(1000);
        }
    }
}


