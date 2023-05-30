/*
  Motion Detection and Sensor Data Logging
  
  This Arduino sketch demonstrates motion detection using the LSM9DS1 IMU (Inertial Measurement Unit).
  It waits for a significant motion event, defined by a threshold acceleration, and then starts logging
  sensor data (acceleration and gyroscope) for a fixed number of samples.
  
  The logged sensor data is printed over the serial connection in CSV format, with each line representing
  a sample and containing the following values:
    accelerometerX, accelerometerY, accelerometerZ, gyroscopeX, gyroscopeY, gyroscopeZ
    
  This code assumes the Arduino LSM9DS1 library is installed for interfacing with the LSM9DS1 IMU sensor.
  
  Library: Arduino_LSM9DS1
  Author: Arduino
  Version: 1.1.1
  More information: https://github.com/arduino-libraries/Arduino_LSM9DS1
  
  To store the logged sensor data into a .csv file you can use Putty -> Session -> Logging -> Printable output
  More information: https://putty.org.ru/articles/capture-putty-session-log.html
  
*/

#include <Arduino_LSM9DS1.h>

const float ACCELERATION_THRESHOLD = 2.5;  // Threshold of significance in G's
const int NUM_SAMPLES = 100;               // Number of samples to read

int samplesRead = NUM_SAMPLES;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.println("accelerometerX,accelerometerY,accelerometerZ,gyroscopeX,gyroscopeY,gyroscopeZ");
}

void loop() {
  float accelerometerX, accelerometerY, accelerometerZ;
  float gyroscopeX, gyroscopeY, gyroscopeZ;

  // Wait for significant motion
  while (samplesRead == NUM_SAMPLES) {
    if (IMU.accelerationAvailable()) {
      // Read the acceleration data
      IMU.readAcceleration(accelerometerX, accelerometerY, accelerometerZ);

      // Sum up the absolute values
      float accelerationSum = fabs(accelerometerX) + fabs(accelerometerY) + fabs(accelerometerZ);

      // Check if it's above the threshold
      if (accelerationSum >= ACCELERATION_THRESHOLD) {
        // Reset the sample read count
        samplesRead = 0;
        break;
      }
    }
  }

  // Check if all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < NUM_SAMPLES) {
    // Check if new acceleration and gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // Read the acceleration and gyroscope data
      IMU.readAcceleration(accelerometerX, accelerometerY, accelerometerZ);
      IMU.readGyroscope(gyroscopeX, gyroscopeY, gyroscopeZ);

      samplesRead++;

      // Print the sensor data
      Serial.print(accelerometerX, 3);
      Serial.print(',');
      Serial.print(accelerometerY, 3);
      Serial.print(',');
      Serial.print(accelerometerZ, 3);
      Serial.print(',');
      Serial.print(gyroscopeX, 3);
      Serial.print(',');
      Serial.print(gyroscopeY, 3);
      Serial.print(',');
      Serial.print(gyroscopeZ, 3);
      Serial.println();

      if (samplesRead == NUM_SAMPLES) {
        Serial.println();
      }
    }
  }
}
