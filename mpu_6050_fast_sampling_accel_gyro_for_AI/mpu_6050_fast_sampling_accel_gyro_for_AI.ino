#include <Wire.h>

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;

int16_t time = 0;

const byte Address = 0x68;

void setup() {
  Serial.begin(2000000);
  Wire.begin();
  Wire.setClock(1000000000);
  setupMPU();
}

void loop() {
  //printData();
  if (getFIFOSize() >= 512) {
    printData();
    clearFIFO();
  }
}

void setupMPU() {
  Wire.beginTransmission(Address);  //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B);                 //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000010);           //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();

  Wire.beginTransmission(Address);  //I2C address of the MPU
  Wire.write(0x1C);                 //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000);           //Setting the accel to +/- 2g
  Wire.endTransmission();

  Wire.beginTransmission(Address);  //I2C address of the MPU
  Wire.write(0x1B);                 //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4)
  Wire.write(0x00000000);           //Setting the gyro to +/- 250deg./s
  Wire.endTransmission();

  // set sample rate divider to 200
  Wire.beginTransmission(Address);
  Wire.write(0x19);  // sample rate divider register
  Wire.write(4);     // set divider to 4 (200Hz)
  Wire.endTransmission();

  clearFIFO();
}
/*
gyroX = Wire.read() << 8 | Wire.read();  //Store first two bytes into accelX
gyroY = Wire.read() << 8 | Wire.read();  //Store middle two bytes into accelY
gyroZ = Wire.read() << 8 | Wire.read();  //Store last two bytes into accelZ

rotX = gyroX / 131.0;
rotY = gyroY / 131.0;
rotZ = gyroZ / 131.0;

accelX = Wire.read() << 8 | Wire.read();  //Store first two bytes into accelX
accelY = Wire.read() << 8 | Wire.read();  //Store middle two bytes into accelY
accelZ = Wire.read() << 8 | Wire.read();  //Store last two bytes into accelZ
gForceX = accelX / 16384.0;
gForceY = accelY / 16384.0;
gForceZ = accelZ / 16384.0;
*/

void printData() {
  long _ = getFIFOSize();
  long __ =(millis() - time);
  float x = (__)/(_/12.0);
  Serial.println(1.0/x);
  //Serial.println((millis() - time));
  time = millis();
}

int16_t getFIFOSize() {
  Wire.beginTransmission(Address);
  Wire.write(0x72);  // FIFO_COUNT_H register
  Wire.endTransmission(false);

  Wire.requestFrom(Address, 2);                           // request 2 bytes of data
  int16_t fifo_count = (Wire.read() << 8) | Wire.read();  // combine the two bytes to form the FIFO count
  return fifo_count;
}

void clearFIFO() {
  // Disable sensors from writing to the FIFO
  Wire.beginTransmission(Address);
  Wire.write(0x23);  // FIFO_EN register
  Wire.write(0x00);  // disable all sensors from writing to the FIFO
  Wire.endTransmission(true);

  // Reset the FIFO buffer
  Wire.beginTransmission(Address);
  Wire.write(0x6A);  // USER_CTRL register
  Wire.write(0x44);  // reset the FIFO buffer
  Wire.endTransmission(true);

  Wire.beginTransmission(Address);
  Wire.write(0x23);       // FIFO_EN register
  Wire.write(0b1111000);  // enable the accelerometer and gyro to write to the FIFO
  Wire.endTransmission();
}
