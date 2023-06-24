/**
  @file mpu9250.c
  @brief Universidad De Antioquia - Departamento de Ingenieria Electronica - Laboratorio N°3 - Telemetria - Electronica Digital III 
    Este archivo tiene las funciones de configuracion del MPU9250 y la lectura de los datos de los sensores
    Asegúrese de que el dispositivo pueda funcionar a 3,3 V, NO a 5 V. El Pico
    GPIO (y por lo tanto SPI) no se puede usar a 5v.

    Necesitará usar un cambiador de nivel en las líneas I2C si desea ejecutar el
    placa a 5v.

    Conexiones en placa Raspberry Pi Pico y placa genérica MPU9250, otras
    las tablas pueden variar.

    GPIO 4 (pin 6) MISO/spi0_rx-> ADO en placa MPU9250
    Selección de chip GPIO 5 (pin 7) -> NCS en placa MPU9250
    GPIO 6 (pin 9) SCK/spi0_sclk -> SCL en placa MPU9250
    GPIO 7 (pin 10) MOSI/spi0_tx -> SDA en placa MPU9250
    3.3v (pin 36) -> VCC en placa MPU9250
    GND (pin 38) -> GND en la placa MPU9250
  @author Sebastian Bernal Cuaspa & Kevin David Martinez Zapata.
  @date 8/05/2023

*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>
#include "mpu9250.h"


#define PIN_MISO 4
#define PIN_CS 5
#define PIN_SCK 6
#define PIN_MOSI 7

#define SPI_PORT spi0
#define READ_BIT 0x80

/**
 * @brief Usado para seleccionar el chip
 *
 * Esta función se usa para seleccionar el chip MPU9250 para poder enviarle datos. no tiene parametros ni retorna nada.
 *
 */

void cs_select() //Used to select the chip
{
    asm volatile("nop \n nop \n nop"); // Small delay
    gpio_put(PIN_CS, 0); // Active low
    asm volatile("nop \n nop \n nop"); // Small delay
}

/**
 * @brief Usado para deseleccionar el chip
 *
 * Esta función se usa para deseleccionar el chip MPU9250 para poder enviarle datos. no tiene parametros ni retorna nada.
 *
 * 
 */

void cs_deselect() //Used to deselect the chip
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

/**
 * @brief Resetea el MPU9250
 *
 * Esta función se usa para resetear el MPU9250. no tiene parametros ni retorna nada.
 *
 *
 */
//Used to reset the mpu
void mpu9250_reset() 
{
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buf[] = {0x6B, 0x00};
    cs_select();
    spi_write_blocking(SPI_PORT, buf, 2);
    cs_deselect();
}

/**
 * @brief Lee los registros del MPU9250
 *
 * Esta función se usa para leer los registros del MPU9250. tiene como parametros el registro que se quiere leer, un arreglo donde se guardaran los datos y la longitud de los datos que se quieren leer. no retorna nada.
 *
 * @param reg registro que se quiere leer.
 * @param buf arreglo donde se guardaran los datos.
 * @param len longitud de los datos que se quieren leer.
 */
//Used to read the registers of the mpu
void read_registers(uint8_t reg, uint8_t *buf, uint16_t len) 
{
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    sleep_ms(10);
    spi_read_blocking(SPI_PORT, 0, buf, len);
    cs_deselect();
    sleep_ms(10);
}

/**
 * @brief Obten los valores de magnetometro del MPU9250
 *
 * Esta función se usa para obtener los valores de magnetometro del MPU9250. no retorna nada.
 *
 * @param mag arreglo donde se guardaran los valores de magnetometro.
 */
//Used to get the raw acceleration values from the mpu
void mpu9250_read_raw_mag(int16_t mag[3]) 
{
    uint8_t buffer[6];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    read_registers(0x4A, buffer, 6);

    for (int i = 0; i < 3; i++) {
        mag[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

/**
 * @brief Calibra el magnetometro del MPU9250
 *
 * Esta funcion se usa para calibrar el magnetometro del MPU9250. no retorna nada.
 * Para calibrar el magentometro se debe dejar el MPU9250 quieto y plano.
 * Y hacer una media de los valores de magnetometro.
 *
 * @param magCal arreglo donde se guardaran los valores de calibracion del magnetometro.
 * @param loop numero de veces que se va a calibrar el magnetometro.
 */
void calibrate_mag(int16_t magCal[3], int loop)  //Used to calibrate the acc. The gyro must be still while calibration happens
{
    int16_t temp[3];
    for (int i = 0; i < loop; i++)
    {
        mpu9250_read_raw_mag(temp);
        magCal[0] += temp[0];
        magCal[1] += temp[1];
        magCal[2] += temp[2];
    }
    magCal[0] /= loop;
    magCal[1] /= loop;
    magCal[2] /= loop;
}

/**
 * @brief Leer los valores de aceleracion del MPU9250
 *
 * Esta función se usa para obtener los valores de magnetometro del MPU9250. no retorna nada.
 *
 * @param accel arreglo donde se guardaran los valores de aceleracion.
 */
void mpu9250_read_raw_accel(int16_t accel[3]) //Used to get the raw acceleration values from the mpu
{
    uint8_t buffer[6];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    read_registers(0x3B, buffer, 6);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

/**
 * @brief Calibra el acelerometro del MPU9250
 *
 * Esta funcion se usa para calibrar el acelerometro del MPU9250. no retorna nada.
 * Para calibrar el acelerometro se debe dejar el MPU9250 quieto y plano.
 * Y hacer una media de los valores de aceleracion.
 *
 * @param accCal arreglo donde se guardaran los valores de calibracion del acelerometro.
 * @param loop numero de veces que se va a calibrar el acelerometro.
 */
void calibrate_acc(int16_t accCal[3], int loop)  //Used to calibrate the acc. The gyro must be still while calibration happens
{
    int16_t temp[3];
    for (int i = 0; i < loop; i++)
    {
        mpu9250_read_raw_accel(temp);
        accCal[0] += temp[0];
        accCal[1] += temp[1];
        accCal[2] += temp[2];
    }
    accCal[0] /= loop;
    accCal[1] /= loop;
    accCal[2] /= loop;
}

/**
 * @brief Leer los valores de giroscopio del MPU9250
 *
 * Esta función se usa para obtener los valores de giroscopio del MPU9250. no retorna nada.
 *
 * @param gyro arreglo donde se guardaran los valores de giroscopio.
 */
void mpu9250_read_raw_gyro(int16_t gyro[3]) //Used to get the raw gyro values from the mpu
{
    uint8_t buffer[6];
    
    read_registers(0x43, buffer, 6);

    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);;
    }
}

/**
 * @brief Calibra el giroscopio del MPU9250
 *
 * Esta función se usa para calibrar el giroscopio del MPU9250. no retorna nada.
 *
 * @param gyroCal arreglo donde se guardaran los valores de calibracion del giroscopio.
 * @param loop numero de veces que se va a calibrar el giroscopio.
 */

void calibrate_gyro(int16_t gyroCal[3], int loop)  //Used to calibrate the gyro. The gyro must be still while calibration happens
{
    int16_t temp[3];
    for (int i = 0; i < loop; i++)
    {
        mpu9250_read_raw_gyro(temp);
        gyroCal[0] += temp[0];
        gyroCal[1] += temp[1];
        gyroCal[2] += temp[2];
    }
    gyroCal[0] /= loop;
    gyroCal[1] /= loop;
    gyroCal[2] /= loop;
}

/**
 * @brief Calcula los angulos del MPU9250
 *
 * Esta función se usa para calcular los angulos del MPU9250. no retorna nada.
 *
 * @param eulerAngles arreglo donde se guardaran los valores de los angulos.
 * @param accel arreglo donde se guardaran los valores de aceleracion.
 * @param gyro arreglo donde se guardaran los valores de giroscopio.
 * @param usSinceLastReading tiempo que ha pasado desde la ultima lectura.
 */
void calculate_angles(int16_t eulerAngles[2], int16_t accel[3], int16_t gyro[3], uint64_t usSinceLastReading) //Calculates angles based on the accelerometer and gyroscope. Requires usSinceLastReading to use the gyro.
{
    long hertz = 1000000/usSinceLastReading; //Calculates the frequency of the gyro
    
    if (hertz < 200) //If the gyro is too slow, use the accelerometer
    {
        calculate_angles_from_accel(eulerAngles, accel); 
        return;
    }

    long temp = 1.l/(hertz * 65.5l);  //Calculates the time between readings

    eulerAngles[0] += gyro[0] * temp; //Calculates the angles based on the gyro
    eulerAngles[1] += gyro[1] * temp;

    eulerAngles[0] += eulerAngles[1] * sin(gyro[2] * temp * 0.1f); //Corrects the angles based on the gyro
    eulerAngles[1] -= eulerAngles[0] * sin(gyro[2] * temp * 0.1f);

    int16_t accelEuler[2];
    calculate_angles_from_accel(accelEuler, accel); //Calculates the angles based on the accelerometer

    eulerAngles[0] = eulerAngles[0] * 0.9996 + accelEuler[0] * 0.0004; //Corrects the angles based on the accelerometer
    eulerAngles[1] = eulerAngles[1] * 0.9996 + accelEuler[1] * 0.0004;
}


/**
 * @brief calcula los angulos del MPU9250 usando el acelerometro
 *
 * Esta función se usa para calcular los angulos del MPU9250 usando el acelerometro. no retorna nada.
 *
 * @param eulerAngles arreglo donde se guardaran los valores de los angulos.
 * @param accel arreglo donde se guardaran los valores de aceleracion.
 */
void calculate_angles_from_accel(int16_t eulerAngles[2], int16_t accel[3]) //Uses just the direction gravity is pulling to calculate angles.
{
    float accTotalVector = sqrt((accel[0] * accel[0]) + (accel[1] * accel[1]) + (accel[2] * accel[2]));

    float anglePitchAcc = asin(accel[1] / accTotalVector) * 57.296;
    float angleRollAcc = asin(accel[0] / accTotalVector) * -57.296;

    eulerAngles[0] = anglePitchAcc;
    eulerAngles[1] = angleRollAcc;
}

/**
 * @brief Convertir los angulos del MPU9250
 *
 * Esta función se usa para convertir los angulos del MPU9250. no retorna nada.
 *
 * @param eulerAngles arreglo donde se guardaran los valores de los angulos.
 * @param accel arreglo donde se guardaran los valores de aceleracion.
 * @param fullAngles arreglo donde se guardaran los valores de los angulos convertidos.
 */

void convert_to_full(int16_t eulerAngles[2], int16_t accel[3], int16_t fullAngles[2]) //Converts from -90/90 to 360 using the direction gravity is pulling
{
    if (accel[1] > 0 && accel[2] > 0) fullAngles[0] = eulerAngles[0]; //Converts the angles from -90/90 to 360
    if (accel[1] > 0 && accel[2] < 0) fullAngles[0] = 180 - eulerAngles[0]; 
    if (accel[1] < 0 && accel[2] < 0) fullAngles[0] = 180 - eulerAngles[0]; 
    if (accel[1] < 0 && accel[2] > 0) fullAngles[0] = 360 + eulerAngles[0];     

    if (accel[0] < 0 && accel[2] > 0) fullAngles[1] = eulerAngles[1]; //Converts the angles from -90/90 to 360
    if (accel[0] < 0 && accel[2] < 0) fullAngles[1] = 180 - eulerAngles[1];
    if (accel[0] > 0 && accel[2] < 0) fullAngles[1] = 180 - eulerAngles[1];
    if (accel[0] > 0 && accel[2] > 0) fullAngles[1] = 360 + eulerAngles[1];
}

/**
 * @brief inicializa el spi del MPU9250
 *
 * Esta función se usa para inicializar el spi del MPU9250, que se usa para comunicarse con el MPU9250. no retorna nada.
 *
 */
void start_spi() //Starts the mpu and resets it
{
    spi_init(SPI_PORT, 500 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    mpu9250_reset();

        // See if SPI is working - interrograte the device for its I2C ID number, should be 0x71
    uint8_t id;
    read_registers(0x75, &id, 1);
    printf("I2C address is 0x%x\n", id);
}