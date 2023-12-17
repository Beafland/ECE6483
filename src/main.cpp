#include <mbed.h>
#include <stdio.h>
#include <iostream>
#include "drivers/LCD_DISCO_F429ZI.h"

// ****** Macros ******//
#define CTRL_REG1 0x20 // First configure to start the sensor
#define CTRL_REG1_CONFIG 0x6F // setup ODR, Cutoff, Power Mode etc
#define SPI_FLAG 1
#define CTRL_REG4 0x23 // Second configure to set the DPS
#define CTRL_REG4_CONFIG 0b0'01'0'00'0 // Setup the DPS of the sensor
#define OUT_X_L 0x28 // X_low address
#define STATUS_REG 0x27
#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)

// ****** Object Declaration ******* //
SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel); // SPI object (mosi, miso, sclk)
EventFlags flags;  //Flags
Ticker timer; // Timer Instance
LCD_DISCO_F429ZI lcd; // LCD instance

// ****** Global variables *******//
uint8_t write_buf[32]; // SPI write buffers
uint8_t read_buf[32]; // SPI read buffers
int16_t raw_gx = 0,raw_gy = 0, raw_gz = 0; // Unprocessed angular velocity
float gx = 0, gy = 0, gz = 0; // Angular velocity
float gx_buf[100], gy_buf[100], gz_buf[100]; // Buffer to hold the angular velocity
float total_distance = 0; // Total distance travelled
char lcd_buffer[100]; // Buffer to hold the formatted string
//unsigned char demo_str[10] = "HELLO WORLD";

// ****** User methods ****** //
void spi_callback(int event)  //spi callback
{
    flags.set(SPI_FLAG);
}

float distance_cal() {
    float radius = 0.9; // Leg length
    float arc_velocity = sqrt(gx * gx + gy * gy + gz * gz); // w = sqrt(wx^2 + wy^2 + wz^2)
    float linear_velocity = arc_velocity * radius;  // v = w * r
    float distance_interval = linear_velocity * 0.5; // d = v * t

    return distance_interval;
}

void get_data(int samples, int interval)
{
    if(samples > 100) {
        printf("Invalid sample size. It should be less then 101");
    }

    for (int i = samples, line = 0; i > 0;i--, line++) {
        write_buf[0] = STATUS_REG | 0x80;

        do
        {
            spi.transfer(write_buf, 2, read_buf, 2, spi_callback);
            flags.wait_all(SPI_FLAG);
            printf("Data Available: 0x%X\n", read_buf[1]);
        } while ((read_buf[1] & 0b0000'1000) == 0);

        write_buf[0] = OUT_X_L | 0x80 | 0x40;

        spi.transfer(write_buf, 7, read_buf, 7, spi_callback);
        flags.wait_all(SPI_FLAG);

        raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
        raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
        raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);
        printf("RAW -> \t\t gx: %d \t gy: %d \t gz: %d \t\n", raw_gx, raw_gy, raw_gz); // Print the unprocessed angular velocity

        gx = ((float)raw_gx) * SCALING_FACTOR;
        gy = ((float)raw_gy) * SCALING_FACTOR;
        gz = ((float)raw_gz) * SCALING_FACTOR;

        printf("Actual -> \t gx: %4.5f \t gy: %4.5f \t gz: %4.5f \t\n", gx, gy, gz); // Print the actual angular velocity

        gx_buf[samples - i] = gx;
        gy_buf[samples - i] = gy;
        gz_buf[samples - i] = gz;

        thread_sleep_for(interval); // Sleep for 0.5s

        float current_distance = distance_cal(); // Calculate the distance for this interval
        total_distance += current_distance; // Distance = velocity * time

        printf("Distance for 0.5s: %4.5f \t\n\n\n", current_distance);
        sprintf(lcd_buffer, "%d Distance: %4.5f", line, current_distance); // Format the string
        lcd.DisplayStringAtLine(line, (uint8_t *)lcd_buffer); // Display on LCD
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
void readGyroAndDisplay() {
    uint8_t x_l[2] =  "H";
    uint8_t x_h[2] =  {data[1]};
    uint8_t y_l[2] =  {data[2]};
    uint8_t y_h[2] =  {data[3]};
    uint8_t z_l[2] =  {data[4]};
    uint8_t z_h[2] =  {data[5]};

    // lcd.Clear(LCD_COLOR_BLACK);
    // lcd.SetBackColor(LCD_COLOR_BLACK);
    // lcd.SetTextColor(LCD_COLOR_WHITE);
 
    lcd.DisplayStringAtLine(1, x_l);
    lcd.DisplayStringAtLine(2, x_h);
    lcd.DisplayStringAtLine(3, y_l);
    lcd.DisplayStringAtLine(4, y_h);
    lcd.DisplayStringAtLine(5, z_l);
    lcd.DisplayStringAtLine(6, z_h);
    lcd.DisplayStringAtLine(7, demo_str);
}
*/

int main() {
    // lcd.Init();
    // lcd.DisplayOn();
    // lcd.DisplayStringAtLine(6, demo_str);
    Gyro_Init();
    get_data(20, 500);
    // lcd.Clear(LCD_COLOR_BLACK); // Clear the screen to black

    printf("Total Distance: %4.5f \t\n", total_distance);
    lcd.Clear(LCD_COLOR_BLACK); // Clear the screen to black
    sprintf(lcd_buffer, "Total Distance: %4.5f \t\n", total_distance); // Format the string
    lcd.DisplayStringAtLine(0, (uint8_t *)lcd_buffer); // Display on LCD
}