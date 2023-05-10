#include <Wire.h>

long accelZ;            // variable to store raw accelerometer data
float gForceZ;          // variable to store accelerometer data in g-forces

long long head = 0;     // variable to keep track of the array index
float buffera[1800];    // array to store accelerometer data

void setup() {
  Serial.begin(2000000);  // initialize serial communication
  Wire.setClock(400000);  // set I2C clock speed
  Wire.begin();           // initialize I2C communication
  setupMPU();             // initialize MPU6050 sensor
}

void loop() {
  if (Serial.available()) {   // check if there is serial data available
    while (head < 1000) {     // record accelerometer data until 1000 samples are collected
      recordAccelRegisters();  // call function to read accelerometer data
    }

    for (long b = 0; b != head; b++) {  // loop through the array and print each value
      Serial.println(buffera[b], 8);    // print accelerometer data with 8 decimal places
    }
    head = 0;     // reset array index to 0

    while (Serial.available()) {   // clear serial buffer
      char inChar = (char)Serial.read();
    }
  }
}

void setupMPU() {          // function to initialize MPU6050 sensor
  Wire.beginTransmission(0b1101000);  // start I2C communication with MPU6050 sensor
  Wire.write(0x6B);        // PWR_MGMT_1 register address
  Wire.write(0b00000000);  // set to 0 to wake up the sensor and use internal clock
  Wire.endTransmission();  // end I2C communication with MPU6050 sensor

  Wire.beginTransmission(0b1101000);  // start I2C communication with MPU6050 sensor
  Wire.write(0x1C);        // ACCEL_CONFIG register address
  Wire.write(0b00000000);  // set to 0 to set accelerometer range to +/- 2g
  Wire.endTransmission();  // end I2C communication with MPU6050 sensor

  Wire.beginTransmission(0b1101000);  // start I2C communication with MPU6050 sensor
  Wire.write(0x1A);        // CONFIG register address
  Wire.write(0x00);        // set to 0 to disable digital low-pass filter
  Wire.endTransmission();  // end I2C communication with MPU6050 sensor
}

void recordAccelRegisters() {   // function to read accelerometer data
  Wire.beginTransmission(0b1101000);  // start I2C communication with MPU6050 sensor
  Wire.write(0x3f);        // register address for Z-axis accelerometer data
  Wire.endTransmission();  // end I2C communication with MPU6050 sensor
  Wire.requestFrom(0b1101000, 2);     // request 2 bytes of data from MPU6050 sensor
  while (Wire.available() < 2);      // wait for the data to be available
  accelZ = Wire.read() << 8 | Wire.read();  // combine the two bytes into a single value
  gForceZ = accelZ / 16384.0;    // convert raw accelerometer data to g-forces
  buffera[head] = gForceZ;      // store g-force data in the array
  head++;                       // increment array index
}
