#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp32/rom/gpio.h"
#include "esp_timer.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22    // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO GPIO_NUM_21    // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define MPU6050_ADDR 0x68       // I2C address of the MPU-6050
#define ONBOARD_LED  GPIO_NUM_0

#define SAMPLES_COUNT 2000
float buffer[SAMPLES_COUNT]; //Circular Buffer
uint16_t buffer_head = 0;

void i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000; // Set I2C master clock frequency
    conf.clk_flags = 0;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

uint8_t* MPU_read(uint8_t reg, uint8_t number_of_bytes) {
    uint8_t* data = (uint8_t*)malloc(number_of_bytes);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true); // Who Am I reg address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    for (int i = 0; i < number_of_bytes; i++) {
        i2c_master_read_byte(cmd, &data[i], I2C_MASTER_ACK);
    }
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS); // Filling in the missing part
    i2c_cmd_link_delete(cmd);
    return data;
}

void MPU_write(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

void app_main() {
    i2c_master_init();

    MPU_write(0x6b, 0x00);

    gpio_pad_select_gpio(ONBOARD_LED);
    gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);

    uint64_t microseconds1 = esp_timer_get_time();
    while (buffer_head <= 1000) {
        uint8_t* pointer = MPU_read(0x3f, 2);
        buffer[buffer_head] = (float)((uint16_t)(pointer[0] << 8) | (pointer[1]));
        buffer_head++;
        //printf("Val: %f\n", (float)accel / 16384.0);
        free(pointer);
    }
    uint64_t microseconds2 = esp_timer_get_time();
    uint64_t diff = microseconds2 - microseconds1;
    printf("Time in microseconds: %lld, head at: %d\n", diff, buffer_head);
}