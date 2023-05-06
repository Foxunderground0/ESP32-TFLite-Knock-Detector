#include <Wire.h>


float gForceX, gForceY, gForceZ;

float rotX, rotY, rotZ;

int16_t time = 0;

const byte Address = 0x68;

#include <SPI.h>
#include <SD.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
File myFile;

void setup() {
  Serial.begin(2000000);
  //Serial.println("Begin");
  Wire.setClock(40000);
  Wire.begin();
  setupMPU();

  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1)
      ;
  }
  Serial.println("initialization done.");
  myFile = SD.open("test.txt", FILE_WRITE);


  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Writing to test.txt...");
    // close the file:
    //Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }


  
  time = millis();

  int a = 0;
  while ((millis() - time) < 1000) {

    if (getFIFOSize() >= 1) {
      int size = getFIFOSize();
      a+= size/12;
      //printData();
      getFIFOData(size / 12, myFile);
     // Serial.print(" ");
      //Serial.print(" ");
      Serial.println(size / 12);

      clearFIFO();
    }
  }
  Serial.println(a);

  myFile.close();

  //delay(5000);
  /*
  for(int i = 0; i < buffer_head; i++){
     Serial.println(buffer[i]);
  }*/
}





void loop() {
  //printData();
}


void getFIFOData(uint16_t samples, File myFile) {
  // Read samples from the FIFO buffer
  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(0x68);
    Wire.write(0x74);  // FIFO count register
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 12, true);  // Read accelerometer and gyroscope data from the FIFO buffer
    long aX = Wire.read() << 8 | Wire.read();
    long aY = Wire.read() << 8 | Wire.read();
    long aZ = Wire.read() << 8 | Wire.read();
    long gX = Wire.read() << 8 | Wire.read();
    long gY = Wire.read() << 8 | Wire.read();
    long gZ = Wire.read() << 8 | Wire.read();

    rotX = gX / 131.0;
    rotY = gY / 131.0;
    rotZ = gZ / 131.0;

    gForceX = aX / 16384.0;
    gForceY = aY / 16384.0;
    gForceZ = aZ / 16384.0;

    myFile.print(rotX, 5);
    myFile.print(",");
    myFile.print(rotY, 5);
    myFile.print(",");
    myFile.print(rotZ, 5);
    myFile.print(",");

    myFile.print(gForceX, 5);
    myFile.print(",");
    myFile.print(gForceY, 5);
    myFile.print(",");
    myFile.println(gForceZ, 5);


    // Add the accelerometer and gyroscope data to the circular buffer
    //buffer[buffer_head] = aX;
    //buffer[(buffer_head + 1) % BUFFER_SIZE] = aY;
    //buffer[(buffer_head + 2) % BUFFER_SIZE] = aZ;
    //buffer[(buffer_head + 3) % BUFFER_SIZE] = gX;
    //buffer[(buffer_head + 4) % BUFFER_SIZE] = gY;
    //buffer[(buffer_head + 5) % BUFFER_SIZE] = gZ;
    //buffer_head = (buffer_head + 6) % BUFFER_SIZE;
  }
}

uint16_t getFIFOSize() {
  Wire.beginTransmission(Address);
  Wire.write(0x72);  // FIFO_COUNT_H register
  Wire.endTransmission(false);

  Wire.requestFrom(Address, 2);                            // request 2 bytes of data
  uint16_t fifo_count = (Wire.read() << 8) | Wire.read();  // combine the two bytes to form the FIFO count
  return fifo_count;
}

void printData() {
  float __ = (millis() - time);
  float _ = getFIFOSize();
  float x = (__) / (_ / 12.0);
  Serial.println(1.0 / (x / 1000.0));
  //Serial.println((millis() - time));
  time = millis();
}

void setupMPU() {
  Wire.beginTransmission(Address);  //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B);                 //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000);           //Setting SLEEP register to 0. (Required; see Note on p. 9)
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
  Wire.write(0);     // set divider to 4 (200Hz)
  Wire.endTransmission();

  clearFIFO();
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