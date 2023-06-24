#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "mpu9250.h"

int main() {
    stdio_init_all();

    sleep_ms(5000);

    printf("Hello, MPU9250! Reading raw data from registers via SPI...\n");

    start_spi();  //Starts the mpu

    int16_t acceleration[3], gyro[3], mag[3];
    int16_t accCal[3], gyroCal[3], magCal[3];

    calibrate_acc(accCal, 2000); // Calibrates the acc
    printf("Offset accl. X=%d, Y=%d, Z=%d\n", accCal[0], accCal[1], accCal[2]);

    calibrate_mag(magCal, 2000); // Calibrates the mag
    printf("Offset  mag. X=%d, Y=%d, Z=%d\n", magCal[0], magCal[1], magCal[2]);

    calibrate_gyro(gyroCal, 2000);  //Calibrates the gyro
    printf("Offset gyro. X=%d, Y=%d, Z=%d\n", gyroCal[0], gyroCal[1], gyroCal[2]);
}
