#ifndef i2c_functions
#define i2c_functions

#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22    // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO GPIO_NUM_21    // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define MPU6050_ADDR 0x68       // I2C address of the MPU-6050

void i2c_master_init();
unsigned char* MPU_read(unsigned char reg, unsigned char number_of_bytes);
void MPU_write(unsigned char reg, unsigned char data);

#endif  // i2c_functions
