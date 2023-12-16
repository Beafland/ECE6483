#include <mbed.h>
#include <stdio.h>
//#include <LCD_DISCO_F429ZI.h>
#include <iostream>
//#include <l3gd20.h>

// ****** Macros ******//
#define CTRL_REG1 0x20 // First configure to start the sensor
#define CTRL_REG1_CONFIG 0x6F // setup ODR, Cutoff, Power Mode etc
#define SPI_FLAG 1
#define CTRL_REG4 0x23 // Second configure to set the DPS
#define CTRL_REG4_CONFIG 0b0'01'0'00'0 // setup the DPS of the sensor
#define OUT_X_L 0x28 // X_low address
#define STATUS_REG 0x27
#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)


// ****** Object Declaration ******* //
SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel); // spi object (mosi, miso, sclk)
EventFlags flags;  //flags
Ticker timer; //Timer Instance
//LCD_DISCO_F429ZI lcd; // LCD instance


// ****** Global variables *******//
int8_t write_buf[32], read_buf[32]; //spi read write buffers
int16_t raw_gx = 0,raw_gy = 0, raw_gz = 0;
float gx = 0, gy = 0, gz = 0; 
float gx_buf[100], gy_buf[100], gz_buf[100];
//unsigned char demo_str[10] = "HI ...";


// ****** user methods ****** //
void spi_cb(int event)  //spi callback
{
    flags.set(SPI_FLAG);
}

void get_data(int samples, int interval)
{
    if(samples > 100)
    {
        printf("Invalid sample size. It should be less then 101");
    }

    for(int i = samples; i > 0;i--)
    {
        write_buf[0] = STATUS_REG | 0x80;

        do
        {
            spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
            flags.wait_all(SPI_FLAG);
            printf("Data Available: 0x%X\n", read_buf[1]);
        } while ((read_buf[1] & 0b0000'1000) == 0);

        write_buf[0] = OUT_X_L | 0x80 | 0x40;

        spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
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

    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;

    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

}

// void readGyroAndDisplay() {


//     uint8_t x_l[2] =  "H";
//     uint8_t x_h[2] =  {data[1]};
//     uint8_t y_l[2] =  {data[2]};
//     uint8_t y_h[2] =  {data[3]};
//     uint8_t z_l[2] =  {data[4]};
//     uint8_t z_h[2] =  {data[5]};


//     // lcd.Clear(LCD_COLOR_BLACK);
//     // lcd.SetBackColor(LCD_COLOR_BLACK);
//     // lcd.SetTextColor(LCD_COLOR_WHITE);
 
//     lcd.DisplayStringAtLine(1,x_l);
//     lcd.DisplayStringAtLine(2,x_h);
//     lcd.DisplayStringAtLine(3,y_l);
//     lcd.DisplayStringAtLine(4,y_h);
//     lcd.DisplayStringAtLine(5,z_l);
//     lcd.DisplayStringAtLine(6,z_h);
//     lcd.DisplayStringAtLine(7,demo_str);


// }

int main() {
    // lcd.Init();
    // lcd.DisplayOn();
    // lcd.DisplayStringAtLine(6,demo_str);
    Gyro_Init();
    get_data(20,500);

    while (true) {
        // Main loop can be used for other tasks
        ThisThread::sleep_for(500ms);

    }

    //return 0;
}
