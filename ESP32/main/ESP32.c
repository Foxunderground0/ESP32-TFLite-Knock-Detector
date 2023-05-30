#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22    // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO GPIO_NUM_21    // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define MPU6050_ADDR 0x68       // I2C address of the MPU-6050

void i2c_master_init()
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000; // Set I2C master clock frequency
    conf.clk_flags = 0;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void MPU_read(uint8_t reg)
{
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true); // Who Am I reg address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS); // Filling in the missing part
    i2c_cmd_link_delete(cmd);

    printf("Data: 0x%X at reg: 0x%X \n", data, reg);
}

void app_main()
{
    i2c_master_init();
    for (uint8_t i = 0x00; i < 0x76; i++) {
        MPU_read(i);
    }
}
