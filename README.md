# Autonomous-Locking-Knee-Brace

## Summary
> An automatic device that distinguishes user movements as falling or non-falling; and locks the maximum flexion of the knee brace (set by user) when a fall is detected.

- Uses Random Forest classifier machine learning algorithm.
- Features of Random Forest classifier include:
  - Mean x, y, and z acceleration
  - Mean x, y, and z gyro
  - Mean triaxial acceleration and gyro magnitude
  - Mean x, y, and z RMS acceleration
  - Mean x, y, and z RMS gyro
- Approximately 1000 training data samples for each label (falling and nonfalling).
- Sensor placement is on chest and hip.
- 2 push-pull solenoid motors attached to both sides of knee brace. Push to lock maximum flexion. Pull to unlock or to allow free leg movement.
- Solenoid's push (lock) when either sensor classify user movement as falling.
- Soleonoid's pull (unlock) when both sensors classify user movement as non-falling.
- Solenoid's will stay locked after a fall event; and will unlock after 5 consecutives of non-fall readings.
- Fall detection accuracy for my movements is 94.5%, false positive rate is 6.5%, false negative rate is 2.8% for a 300 ms sampling interval.


# Setup
## Get Training Data
1. Upload 'mpu6050_output' Arduino code to Uno board.
2. In MATLAB, have 'getTrainingData.m' and 'getFeatures.m' in the environment.
> Setup the circuit as shown below.
<img src="https://user-images.githubusercontent.com/84931559/120691398-697d4580-c474-11eb-9fd0-e62b2ad93697.png" width="700">

3. Run 'getTrainingData.m'
4. The collected training data is stored in your workspace variable 'trainingData'. Copy the training data to a CSV file and manually input the label as 1 for falling, or 0 for non-falling in the last column.
5. Continue to run 'getTrainingData.m' to gather training data.

## Export Random Forest Classifier to C++
1. Upload 'rfc' Python program.
2. Replace the path with your CSV path that contains your training data.
3. Run the Python script and the output is your Random Forest classifier code in C++. 
4. Copy the output to a text editor. Remove the "#include cstdarg" statement from the output.
5. Change the data type of the fucntions 'maxVotes', 'predictLabel', and 'idxToLabel' to 'int' variable types.
6. Save the output as a .h file.
7. Repeat Steps 2-6 for multiple Random Forest classifiers.
> Put all the classifiers into one file and rename the variables. Ex: predict_c, maxVotes_c, predictLabel_c, idxToLabel_c for chest sensor

## Standalone ML Arduino
1. Upload "FallDetectionRF" Arduino code to Mega board.
2. Include the Random Forest classifier .h file into the same folder as the "FallDetectionRF" Arduino code.
3. Setup the circuit as shown below and upload the Arduino code.
<img src="https://user-images.githubusercontent.com/84931559/120695492-76e8fe80-c479-11eb-82a4-1e4a17a5788e.png">
