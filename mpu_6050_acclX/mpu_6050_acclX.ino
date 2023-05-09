#include <Wire.h>

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;

long head = 0;

float buffer[350];

void setup() {
  Serial.begin(2000000);
  Wire.setClock(400000);
  Wire.begin();
  setupMPU();
  uint16_t time = millis();
  int a = 0;
  while (head < 500) { //millis()-time < 1000
    recordAccelRegisters();
    //printData();
    a++;
  }

  Serial.println(a);
  for (long b = 0; a <= head; b++) {
    Serial.println(buffer[b]);
  }
}

void loop() {
}

void setupMPU() {
  Wire.beginTransmission(0b1101000);  //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B);                   //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000);             //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000);  //I2C address of the MPU
  Wire.write(0x1C);                   //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000);             //Setting the accel to +/- 2g
                                      // Wire.write(0b00011000); //Setting the accel to +/- 16g
  Wire.endTransmission();
  Wire.beginTransmission(0b1101000);  //I2C address of the MPU
  Wire.write(0x1A);                   // CONFIG register
  Wire.write(0x00);                   // set to zero (disables DLPF)
  Wire.endTransmission();
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000);  //I2C address of the MPU
  Wire.write(0x3f);                   //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 2);  //Request Accel Registers (3B - 40)
  while (Wire.available() < 2)
    ;
  //accelX = Wire.read() << 8 | Wire.read();  //Store first two bytes into accelX
  //accelY = Wire.read() << 8 | Wire.read();  //Store middle two bytes into accelY
  accelZ = Wire.read() << 8 | Wire.read();  //Store last two bytes into accelZ
  //gForceX = accelX / 16384.0;               //lowest detect range 2 g
  //gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;

  // gForceX = accelX / 2048.0;
  // gForceY = accelY / 2048.0;
  // gForceZ = accelZ / 2048.0;

  buffer[head] = gForceZ;
  head++;
}

void printData() {
  Serial.print("0 ");
  Serial.println(gForceZ, 8);



  //Serial.print(atan(gForceX/gForceZ));
  //Serial.print(" ");
  //Serial.println(atan(gForceY));
  //  Serial.print(" ");
  //  Serial.println(gForceZ);
}
