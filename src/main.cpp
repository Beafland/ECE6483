#include <mbed.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "drivers/LCD_DISCO_F429ZI.h"
//#include <l3gd20.h>
#define BACKGROUND 1
#define FOREGROUND 0
#define GRAPH_PADDING 5

// ****** Macros ******//
#define CTRL_REG1 0x20 // First configure to start the sensor
#define CTRL_REG1_CONFIG 0x6F // Setup ODR, Cutoff, Power Mode etc
#define SPI_FLAG 1
#define CTRL_REG4 0x23 // Second configure to set the DPS
#define CTRL_REG4_CONFIG 0b0'01'0'00'0 // Setup the DPS of the sensor
#define OUT_X_L 0x28 // X_low address
#define STATUS_REG 0x27
#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)


// ****** Object Declaration ******* //
LCD_DISCO_F429ZI lcd;
char display_buf[2][60];
uint32_t graph_width=lcd.GetXSize()-2*GRAPH_PADDING;
uint32_t graph_height=graph_width;

SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel); // spi object (mosi, miso, sclk)
DigitalOut cs(PC_1); // Chip select pin
EventFlags flags;  // Flags
Ticker timer; // Timer Instance

// ****** Global variables *******//
// Define the read/write buffers
uint8_t write_buf[32]; 
uint8_t read_buf[32];

int16_t raw_gx = 0,raw_gy = 0, raw_gz = 0;
float gx = 0, gy = 0, gz = 0; 
float gx_buf[100], gy_buf[100], gz_buf[100];
//unsigned char demo_str[10] = "HI ..."; // LCD test


// ****** user methods ****** //
void spi_callback(int event)  //spi callback
{
    flags.set(SPI_FLAG);
}

void get_data(int samples, int interval)
{
    if(samples > 100)
    {
        printf("Invalid sample size. Expected to be less then 101");
    }

    for(int i = samples; i > 0;i--)
    {
        write_buf[0] = STATUS_REG | 0x80;

        do
        {
            spi.transfer(write_buf, 2, read_buf, 2, spi_callback);
            flags.wait_all(SPI_FLAG);
            printf("Data available at: 0x%X\n", read_buf[1]);
        } while ((read_buf[1] & 0b0000'1000) == 0);

        write_buf[0] = OUT_X_L | 0x80 | 0x40;

        spi.transfer(write_buf, 7, read_buf, 7, spi_callback);
        flags.wait_all(SPI_FLAG);

        raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
        raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
        raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);
        printf("RAW -> \t\t gx: %d \t gy: %d \t gz: %d \t\n", raw_gx, raw_gy, raw_gz);

        gx = ((float)raw_gx) * SCALING_FACTOR;
        gy = ((float)raw_gy) * SCALING_FACTOR;
        gz = ((float)raw_gz) * SCALING_FACTOR;
        printf("Actual -> \t gx: %4.5f \t gy: %4.5f \t gz: %4.5f \t\n", gx, gy, gz);

        gx_buf[samples - i] = gx;
        gy_buf[samples - i] = gy;
        gz_buf[samples - i] = gz;

        thread_sleep_for(interval);
    }
}

void Gyro_Init() {
    spi.format(8, 3);

    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;

    spi.transfer(write_buf, 2, read_buf, 2, spi_callback);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;

    spi.transfer(write_buf, 2, read_buf, 2, spi_callback);
    flags.wait_all(SPI_FLAG);
}
/*
// Function to read angular velocity from the gyroscope
float readAngularVelocity() {
    // SPI communication to read data from gyroscope
    cs = 0; // Select the device

    write_buf[0] = 0x28 | 0x80; // Address of the data register with read command
    spi.write((const char *)write_buf, 1, nullptr, 0); // Write the address
    // Send dummy bytes to read data
    spi.write(nullptr, 0, (char *)read_buf, 2); 
    cs = 1; // Deselect the device

    // Assuming the data is 16-bit and in two's complement
    int16_t raw_data = (read_buf[1] << 8) | read_buf[0];
    float angular_velocity = (float)raw_data * 0.07; // Conversion factor based on gyroscope's sensitivity

    return angular_velocity; // Return the measured angular velocity
}

// Function to convert angular velocity to forward velocity
float convertToForwardVelocity(float angularVelocity) {
    // Conversion logic based on sensor placement and required math
    float forward_velocity = angularVelocity * 0.1; // Example conversion factor
    return forward_velocity;
}

// Main function
int main() {
    initGyro();
    float totalDistance = 0.88;
    float forwardVelocity = 4.55;

    for (int i = 0; i < 40; ++i) { // Sampling every 0.5s for 20 seconds
        ThisThread::sleep_for(500ms);

        // float angularVelocity = readAngularVelocity();
        // float forwardVelocity = convertToForwardVelocity(angularVelocity);

        // // Calculate distance for this interval
        // float distance = forwardVelocity * 0.5; // Distance = velocity * time
        // totalDistance += distance;
        // totalDistance = 0.5;

        // Display current velocity and distance
        setlocale(LC_NUMERIC, "en_US.UTF-8");  // Set locale to use period as decimal separator
        printf("Velocity: %.2f m/s, Distance: %lf m\n", forwardVelocity, totalDistance);
        printf("%f",2.2);
    }

    // Display total distance after 20 seconds
    printf("Total Distance Travelled: %f m\n", totalDistance);
    return 0;
}
*/