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
- Accuracy for fall detection is 94.5%, false positive rate is 6.5%, false negative rate is 2.8% for a 300 ms sampling interval.
  - Look for high accuracy with less false positive/negative rates for different sampling intervals. In particular, 300 ms is best performing for me.
