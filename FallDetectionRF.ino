/*  Arduino Code for Standalone Fall Detection
 *   Description: For the given sampling rate, the model gathers 20 raw data samples and calculates the features
 *   for the duration. If either the chest or hip sensor predicts a fall event, the preset flexion is locked.
 *   If not, it will check if the brace is locked and wait for 5 consecutive nonfalls to unlock.
 *   
 *   The code may occasionally stop working. I presume a delay is needed to handle the amount of data being gathered.
 */


#include <Wire.h>
#include <SoftwareSerial.h>
#include "chestHipKneeRF.h" // Random Forest classifier C code
#define NUM_SAMPLES 20  // num samples per duration process


const int MPU1=0x68, MPU2=0x69; // chest and hip sensor
int consecNonfall=16;           // consecutive nonfalls until unlock (300ms*16 ~= 5s until unlock)

// Digital pins for the two push-pull solenoid motors.
int solenoidPin1 = 9;           
int solenoidPin2 = 10;

//---------------------------------------------------------------------------------------------------------------------------------------

// Chest Variables
long accelX, accelY, accelZ;  // raw chest accel sensor values
float gForceX, gForceY, gForceZ;  // chest accel converted to [g], g = 9.8 m/s^2

long gyroX, gyroY, gyroZ; // raw chest gyro sensor values
float rotX, rotY, rotZ, rotX_rad, rotY_rad, rotZ_rad; // chest gyro converted to deg/s and rad/s

float sensorVals[NUM_SAMPLES][6]; // store 20 raw data samples per sampling duration
float axSum, aySum, azSum, accelMag, accelMagSum, axMean, ayMean, azMean, accelMagMean ; // for accel mean calculation
float gxSum, gySum, gzSum, gyroMag, gyroMagSum, gxMean, gyMean, gzMean, gyroMagMean; // for gyro mean calculation

float axSquaredSum, aySquaredSum, azSquaredSum, axRMS, ayRMS, azRMS; // for accel RMS calculation
float gxSquaredSum, gySquaredSum, gzSquaredSum, gxRMS, gyRMS, gzRMS; // for gyro RMS calculation

float chestFeatures[14];  // chest features for the duration
int chestLabel; // chest prediction

//---------------------------------------------------------------------------------------------------------------------------------------

// Hip Variables
long accelX2, accelY2, accelZ2; // raw hip accel sensor values
float gForceX2, gForceY2, gForceZ2; // hip accel converted to [g], g = 9.8 m/s^2

long gyroX2, gyroY2, gyroZ2;  // raw hip gyro sensor values
float rotX2, rotY2, rotZ2, rotX_rad2, rotY_rad2, rotZ_rad2; // hip gyro converted to deg/s and rad/s

float sensorVals2[NUM_SAMPLES][6];  // store 20 raw data samples per sampling duration
float axSum2, aySum2, azSum2, accelMag2, accelMagSum2, axMean2, ayMean2, azMean2, accelMagMean2 ; // for accel mean calculation
float gxSum2, gySum2, gzSum2, gyroMag2, gyroMagSum2, gxMean2, gyMean2, gzMean2, gyroMagMean2; // for gyro mean calculation

float axSquaredSum2, aySquaredSum2, azSquaredSum2, axRMS2, ayRMS2, azRMS2; // for accel RMS calculation
float gxSquaredSum2, gySquaredSum2, gzSquaredSum2, gxRMS2, gyRMS2, gzRMS2; // for gyro RMS calculation

float hipFeatures[14];  // hip features for the duration
int hipLabel; // hip prediction
//---------------------------------------------------------------------------------------------------------------------------------------


unsigned long time; // to find sampling rate or to find delay for a sampling rate
int fall; // store number of falls (used for confuse matrix)
int nonfall;  // store number of nonfalls (used for confuse matrix)

Eloquent::ML::Port::RandomForest clf; // using Random Forest Classifier


void setup() {
  Serial.begin(115200);
  Wire.begin();

  // set the push-pull solenoids to its respective pins
  pinMode(solenoidPin1, OUTPUT);  
  pinMode(solenoidPin2, OUTPUT);

  // initialize chest and hip sensors
  setupMPU1();
  setupMPU2();
  
  time =0;

  // set number of falls and nonfalls to zero
  fall=0;
  nonfall=0;
}


void loop() {
  
  resetFeatures();  //  reset the chest and hip features from previous duration
  getDurationData();  // get duration raw sensor values

  /*Uncomment Below to check sampling time or to find a delay for sampling time.*/
//  time = millis();
//  Serial.print("Collected Duration Data Time: ");
//  Serial.println(time);

  getDurationFeatures();  // get features for duration

  /* Uncomment Below to see raw duration data and duration features*/
//  printDurationData();  
//  printFeatures();
  
  chestLabel = getChestLabel(); // chest prediction for duration
  hipLabel = getHipLabel(); // hip prediction for duration

  // See chest and hip labels for duration
  Serial.println();
  Serial.print("Chest Sensor: ");
  Serial.print(chestLabel);
  Serial.print("    Hip Sensor: ");
  Serial.print(hipLabel);

  
  // Lock or Check to Unlock
  if(chestLabel == 1 || hipLabel == 1){ // if either chest or hip predicts a fall event -> lock flexion
    Serial.println(" ==> Brace Lock");
    digitalWrite(solenoidPin1, LOW);
    digitalWrite(solenoidPin2, LOW);
    consecNonfall=0;  // reset consecutive nonfalls to zero
    fall+=1;  // serial reading to see how much fall events has occured
  }
  else{
    if(consecNonfall!=16){  // unlock after 5 seconds of consecutive nonfall readings for both chest and hip sensor
      Serial.println();
      Serial.print("Waiting for consecutive 7 consecutive nonfalls to unlock: ");
      consecNonfall+=1; // continue to wait for unlock
      Serial.println(consecNonfall);  
    }
    else{
      Serial.println(" ==> Brace Unlocked");  // unlock the set flexion
      digitalWrite(solenoidPin1, HIGH);
      digitalWrite(solenoidPin2, HIGH);
    }
    nonfall+=1; // serial reading to see how much nonfall events has occured
  }


  /* Uncomment Below and its counterpart on line 102*/
//  Serial.print("Sample Reading Finish ... ");
//  time = millis();
//  Serial.println(time);

  /* Uncomment Below to see number of falls and nonfalls*/
//  Serial.print("Fall: ");
//  Serial.print(fall);
//  Serial.print("    Nonfall: ");
//  Serial.println(nonfall);
//  Serial.println();
//  Serial.println();

}

//---------------------------------------------------------------------------------------------------------------------------------------

// MPU1 = chest sensor
void setupMPU1(){
  Wire.beginTransmission(MPU1); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(MPU1); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(MPU1); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegisters_MPU1() {
  Wire.beginTransmission(MPU1); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU1,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ

  // accel values in [g]
  gForceX = accelX / 16384.0; 
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
  accelMag = sqrt(pow(gForceX,2)+pow(gForceY,2)+pow(gForceZ,2)); // get accel magnitude

}

void recordGyroRegisters_MPU1() {
  Wire.beginTransmission(MPU1); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU1,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ

  // deg/s
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;

  // rad/s
  rotX_rad = (rotX/180)*PI;
  rotY_rad = (rotY/180)*PI;
  rotZ_rad = (rotZ/180)*PI;
  
  gyroMag = sqrt(pow(rotX_rad,2)+pow(rotY_rad,2)+pow(rotZ_rad,2));  // get gyro magnitude

  
}
//---------------------------------------------------------------------------------------------------------------------------------------

// MPU2 = hip sensor
void setupMPU2(){
  Wire.beginTransmission(MPU2); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
  Wire.endTransmission();  
  Wire.beginTransmission(MPU2); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s 
  Wire.endTransmission(); 
  Wire.beginTransmission(MPU2); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g
  Wire.endTransmission(); 
}

void recordAccelRegisters_MPU2() {
  Wire.beginTransmission(MPU2); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU2,6); //Request Accel Registers (3B - 40)
  while(Wire.available() < 6);
  accelX2 = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  accelY2 = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ2 = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ

  // accel values in [g]
  gForceX2 = accelX2 / 16384.0; 
  gForceY2 = accelY2 / 16384.0; 
  gForceZ2 = accelZ2 / 16384.0;
  accelMag2 = sqrt(pow(gForceX2,2)+pow(gForceY2,2)+pow(gForceZ2,2));  // get accel magnitude

}

void recordGyroRegisters_MPU2() {
  Wire.beginTransmission(MPU2); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU2,6); //Request Gyro Registers (43 - 48)
  while(Wire.available() < 6);
  gyroX2 = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY2 = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ2 = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ

  // deg/s
  rotX2 = gyroX2 / 131.0;
  rotY2 = gyroY2 / 131.0; 
  rotZ2 = gyroZ2 / 131.0;

  // rad/s
  rotX_rad2 = (rotX2/180)*PI;
  rotY_rad2 = (rotY2/180)*PI;
  rotZ_rad2 = (rotZ2/180)*PI;
  
  gyroMag2 = sqrt(pow(rotX_rad2,2)+pow(rotY_rad2,2)+pow(rotZ_rad2,2));  // get gyro magnitude

}
//---------------------------------------------------------------------------------------------------------------------------------------
void printData() {
  
  /* For Chest Sensor*/
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

  // -----------------------------

  /* For Hip Sensor*/
  // Get triaxial acceleration
  Serial.print(gForceX2);
  Serial.print(",");
  Serial.print(gForceY2);
  Serial.print(",");
  Serial.print(gForceZ2);
  Serial.print(",");

  // Get triaxial gyroscope
  Serial.print(rotX_rad2);
  Serial.print(",");
  Serial.print(rotY_rad2);
  Serial.print(",");
  Serial.println(rotZ_rad2);
}

void resetFeatures(){
  /* Chest Sensor*/
  axSum=0, aySum=0, azSum=0;
  gxSum=0, gySum=0, gzSum=0;
  accelMagSum=0;
  gyroMagSum=0;
  
  axSquaredSum=0, aySquaredSum=0, azSquaredSum=0;
  gxSquaredSum=0, gySquaredSum=0, gzSquaredSum=0;

  for(int i =0; i<14;i++){
    chestFeatures[i]=0;
  }

  // -----------------------------

  /* Hip Sensor*/
  axSum2=0, aySum2=0, azSum2=0;
  gxSum2=0, gySum2=0, gzSum2=0;
  accelMagSum2=0;
  gyroMagSum2=0;
  
  axSquaredSum2=0, aySquaredSum2=0, azSquaredSum2=0;
  gxSquaredSum2=0, gySquaredSum2=0, gzSquaredSum2=0;

  for(int i =0; i<14;i++){
    hipFeatures[i]=0;
  }
  
}
void getDurationData() {
  for(int i=0;i<NUM_SAMPLES;i++){
    recordAccelRegisters_MPU1();
    recordAccelRegisters_MPU2();
    
    recordGyroRegisters_MPU1();
    recordGyroRegisters_MPU2();

    for(int j=0;j<6;j++){
    
      if(j == 0){
        sensorVals[i][j]=gForceX;
        axSum+=gForceX;
        axSquaredSum+=pow(gForceX,2);

        sensorVals2[i][j]=gForceX2;
        axSum2+=gForceX2;
        axSquaredSum2+=pow(gForceX2,2);
      }
      else if(j ==1){
        sensorVals[i][j]=gForceY;
        aySum+=gForceY;
        aySquaredSum+=pow(gForceY,2);
        
        sensorVals2[i][j]=gForceY2;
        aySum2+=gForceY2;
        aySquaredSum2+=pow(gForceY2,2);
        
      }
      else if(j ==2){
        sensorVals[i][j]=gForceZ;
        azSum+=gForceZ;
        azSquaredSum+=pow(gForceZ,2);

        sensorVals2[i][j]=gForceZ2;
        azSum2+=gForceZ2;
        azSquaredSum2+=pow(gForceZ2,2);
      }
      else if(j ==3){
        sensorVals[i][j]=rotX_rad;
        gxSum+=rotX_rad;
        gxSquaredSum+=pow(rotX_rad,2);

        sensorVals2[i][j]=rotX_rad2;
        gxSum2+=rotX_rad2;
        gxSquaredSum2+=pow(rotX_rad2,2);
      }
      else if(j ==4){
        sensorVals[i][j]=rotY_rad;
        gySum+=rotY_rad;
        gySquaredSum+=pow(rotY_rad,2);

        sensorVals2[i][j]=rotY_rad2;
        gySum2+=rotY_rad2;
        gySquaredSum2+=pow(rotY_rad2,2);
      }
      else{
        sensorVals[i][j]=rotZ_rad;
        gzSum+=rotZ_rad;
        gzSquaredSum+=pow(rotZ_rad,2);

        sensorVals2[i][j]=rotZ_rad2;
        gzSum2+=rotZ_rad2;
        gzSquaredSum2+=pow(rotZ_rad2,2);
      }
          
    }
    accelMagSum+=accelMag;
    gyroMagSum+=gyroMag;

    accelMagSum2+=accelMag2;
    gyroMagSum2+=gyroMag2;

    /* Approximate delays to achieve 20 samples for sampling time 100 ms to 1 s*/
    // no delay for 100 ms
    //delay(4);   // for 200 ms
    delay(9); // for 300 ms
    //delay(14); // for 400 ms
    //delay(19); // for 500 ms
    //delay(24); // for 600 ms
    //delay(29); // for 700 ms
    //delay(34); // for 800 ms
    //delay(39); // for 900 ms
    //delay(44); // for 1 s
  }
}


void printDurationData() {
  Serial.println("Raw Chest Sensor Data");
  for(int i=0;i<NUM_SAMPLES;i++){
    for(int j=0;j<6;j++){
      Serial.print(sensorVals[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
  Serial.println("Hip Chest Sensor Data");
  for(int i=0;i<NUM_SAMPLES;i++){
    for(int j=0;j<6;j++){
      Serial.print(sensorVals2[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

void getDurationFeatures() {
  
  // getMeanAccel
  chestFeatures[0]= axSum/NUM_SAMPLES;
  chestFeatures[1] = aySum/NUM_SAMPLES;
  chestFeatures[2] = azSum/NUM_SAMPLES; 

  // getMeanGyro
  chestFeatures[3] = gxSum/NUM_SAMPLES;
  chestFeatures[4] = gySum/NUM_SAMPLES;
  chestFeatures[5] = gzSum/NUM_SAMPLES;

  // get accel and gyro mean
  chestFeatures[6] = accelMagSum/NUM_SAMPLES;
  chestFeatures[7] = gyroMagSum/NUM_SAMPLES;

  // get accel and gyro RMS
  chestFeatures[8] = sqrt(axSquaredSum/NUM_SAMPLES);
  chestFeatures[9] = sqrt(aySquaredSum/NUM_SAMPLES);
  chestFeatures[10] = sqrt(azSquaredSum/NUM_SAMPLES);
  chestFeatures[11] = sqrt(gxSquaredSum/NUM_SAMPLES);
  chestFeatures[12] = sqrt(gySquaredSum/NUM_SAMPLES);
  chestFeatures[13] = sqrt(gzSquaredSum/NUM_SAMPLES);

  // -----------------------------
  
  // getMeanAccel2
  hipFeatures[0]= axSum2/NUM_SAMPLES;
  hipFeatures[1] = aySum2/NUM_SAMPLES;
  hipFeatures[2] = azSum2/NUM_SAMPLES; 

  // getMeanGyro
  hipFeatures[3] = gxSum2/NUM_SAMPLES;
  hipFeatures[4] = gySum2/NUM_SAMPLES;
  hipFeatures[5] = gzSum2/NUM_SAMPLES;

  // get accel and gyro mean
  hipFeatures[6] = accelMagSum2/NUM_SAMPLES;
  hipFeatures[7] = gyroMagSum2/NUM_SAMPLES;

  // get accel and gyro RMS
  hipFeatures[8] = sqrt(axSquaredSum2/NUM_SAMPLES);
  hipFeatures[9] = sqrt(aySquaredSum2/NUM_SAMPLES);
  hipFeatures[10] = sqrt(azSquaredSum2/NUM_SAMPLES);
  hipFeatures[11] = sqrt(gxSquaredSum2/NUM_SAMPLES);
  hipFeatures[12] = sqrt(gySquaredSum2/NUM_SAMPLES);
  hipFeatures[13] = sqrt(gzSquaredSum2/NUM_SAMPLES);
}

void printFeatures() {
  Serial.println("Chest Features");
  for(int i =0;i<14;i++){
    Serial.print(chestFeatures[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.println("Hip Features");
  for(int i =0;i<14;i++){
    Serial.print(hipFeatures[i]);
    Serial.print(" ");
  }
}

int getChestLabel() {
    return clf.predictLabel_c(chestFeatures);
}

int getHipLabel() {
    return clf.predictLabel_h(hipFeatures);
}
