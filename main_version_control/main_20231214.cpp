#include "mbed.h"
#include "math.h"
#include "drivers/LCD_DISCO_F429ZI.h"
#define BACKGROUND 1
#define FOREGROUND 0
#define GRAPH_PADDING 5
#include <stdio.h>


LCD_DISCO_F429ZI lcd;
//buffer for holding displayed text strings
char display_buf[2][60];
uint32_t graph_width=lcd.GetXSize()-2*GRAPH_PADDING;
uint32_t graph_height=graph_width;

// Define SPI pins for STM32F429 Discovery Board
SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1);

// Define the read/write buffers
uint8_t write_buf[32]; 
uint8_t read_buf[32];

// Gyroscope initialization function
void initGyro() {
    // Assuming the gyroscope has a specific register to be written for initialization
    cs = 0; // Select the device by setting chip select low
    write_buf[0] = 0x20; // Address of the control register
    write_buf[1] = 0x0F; // Value to be written to the control register
    spi.write((const char *)write_buf, 2, nullptr, 0); // Blocking write to the gyroscope
    cs = 1; // Deselect the device
}

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