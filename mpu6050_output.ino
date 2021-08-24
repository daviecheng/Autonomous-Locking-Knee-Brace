/*
Description of Code: 
Output MPU 6050 sensor triaxial acceleration and gyroscope values to serial port.

The code is provided by EEEnthusiast with my modifications.
EEEnthusiast Website: http://eeenthusiast.com/
*/

#include <Wire.h>
#include <SoftwareSerial.h>

long accelX, accelY, accelZ;  // raw accel sensor data
float gForceX, gForceY, gForceZ;  // accel in [g], g = 9.8m/s^2

long gyroX, gyroY, gyroZ; // raw gyro sensor data
float rotX, rotY, rotZ, rotX_rad, rotY_rad, rotZ_rad; // gyro in deg/s, rad/s (rad/s is used)

float w, accel; // triaxial gyro and sensor magnitude

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setupMPU();
}

void loop() {
    recordAccelRegisters(); // get sensor accel
    recordGyroRegisters();  // get sensor gyro
    printData();  // print sensor accel and gyro to serial port
    delay(128); 
}

void setupMPU(){
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}

void processAccelData(){
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
  accel = sqrt(pow(gForceX,2)+pow(gForceY,2)+pow(gForceZ,2));
}

void recordGyroRegisters() {
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(0b1101000,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}

void processGyroData() {

  // deg/s
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;

  // rad/s
  rotX_rad = (rotX/180)*PI;
  rotY_rad = (rotY/180)*PI;
  rotZ_rad = (rotZ/180)*PI;
  
  w = sqrt(pow(rotX_rad,2)+pow(rotY_rad,2)+pow(rotZ_rad,2));
}

void printData() {
  
  // Get triaxial acceleration
  Serial.print(gForceX);
  Serial.print(",");
  Serial.print(gForceY);
  Serial.print(",");
  Serial.print(gForceZ);
  Serial.print(",");

  // Get triaxial gyroscope
  Serial.print(rotX_rad);
  Serial.print(",");
  Serial.print(rotY_rad);
  Serial.print(",");
  Serial.println(rotZ_rad);
}
