# Autonomous_Locking_Knee_Brace

### NYIT Senior Design Project 2020-21
***No responsibility disclaimer - This project is a proof of concept and is not intended as a substitute for professional use.***
## Summary
> An automatic device that locks the preset flexion on knee brace when a fall is detected. Proof of concept project aimed to prevent leg injury in elderly users when falling when used alongside their existing mobility aids.

- Sensor placement is on chest and hip.
- Uses Random Forest classifier machine learning algorithm.
- Approximately 1000 training data samples for each label (falling and nonfalling).
- Features of Random Forest classifier:
  - Mean x, y, and z acceleration and gyro
  - Mean triaxial acceleration and gyro magnitude
  - Mean x, y, and z RMS acceleration and gyro
- 2 push-pull solenoid motors attached to both sides of knee brace to lock and unlock.
- Solenoid's will stay locked after a fall event; and will unlock after 5 consecutive non-fall readings.
- Fall detection accuracy for my movements is 94.5%, false positive rate is 6.5%, false negative rate is 2.8% for a 300 ms sampling interval.


# Setup
## Get Training Data
1. Upload 'sensor' Arduino code to Uno board.
2. In MATLAB, have 'getTrainingData.m' and 'getFeatures.m' in the environment.
> Setup the circuit as shown below.
<img src="https://user-images.githubusercontent.com/84931559/120691398-697d4580-c474-11eb-9fd0-e62b2ad93697.png" width="700">

3. Run 'getTrainingData.m'
4. The collected training data is stored in your workspace variable 'trainingData'. Copy the training data to a CSV file and manually input the label as 1 for falling, or 0 for non-falling in the last column. The CSV files for my training data is included for reference.
> Note: I suggest performing two experiments for simplicity. One for falling and the other for non-falling.
5. Continue to run 'getTrainingData.m' to gather training data.

## Export Random Forest Classifier to C++
1. Upload 'rfc' Python program.
2. Replace the path with your CSV path that contains your training data.
3. Run the Python script and the output is your Random Forest classifier code in C++. 
4. Copy the output to a text editor. Remove the "#include cstdarg" statement from the output.
5. Change the data type of the fucntions 'maxVotes', 'predictLabel', and 'idxToLabel' to 'int' variable types.
6. Save the output as a .h file.
7. Repeat Steps 2-6 for multiple Random Forest classifiers.

## Standalone ML Arduino
1. Upload "arduino-rfc" Arduino code to Arduino IDE.
2. Include the Random Forest classifier .h file into the same folder as the "arduino-rfc" Arduino code.
3. Adjust the code corresponding to your variable names.
4. Setup the circuit as shown below; then upload the code to Mega board.
<img src="https://user-images.githubusercontent.com/84931559/120695492-76e8fe80-c479-11eb-82a4-1e4a17a5788e.png">
