/*
  Gesture Recognition and Door Control

  This Arduino sketch demonstrates gesture recognition using an IMU (Inertial Measurement Unit) sensor and
  TensorFlow Lite for microcontrollers. It waits for a significant motion event based on a threshold
  acceleration and then collects sensor data (acceleration and gyroscope) for a fixed number of samples.
  The collected data is then processed by a pre-trained machine learning model to recognize gestures.
  If the recognized gesture matches the expected gesture a "true" message is sent via the Bluetooth 
  connection, triggering the door opening on the web page.

  The code assumes the following libraries are installed:
  - Arduino_LSM9DS1: For interfacing with the LSM9DS1 IMU sensor
  - ArduinoBLE: For Bluetooth Low Energy (BLE) communication
  - TensorFlowLite Micro: For running the machine learning model

  Library: Arduino_LSM9DS1
  Author: Arduino
  Version: 1.1.1
  More information: https://github.com/arduino-libraries/Arduino_LSM9DS1

  Library: ArduinoBLE
  Author: Arduino
  Version: 1.3.2
  More information: https://github.com/arduino-libraries/ArduinoBLE

  Library: TensorFlowLite Micro
  Author: TensorFlow
  Version: 2.1.0-ALPHA
  More information: https://github.com/tensorflow/tflite-micro

  Model: model.h
  Description: Pre-trained machine learning model for gesture recognition

*/

#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>
#include "model.h"

const float ACCELERATION_THRESHOLD = 2.5;  // Threshold of significance in G's
const int NUM_SAMPLES = 100;               // Number of samples to read

int samplesRead = NUM_SAMPLES;

// Global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;
tflite::AllOpsResolver tflOpsResolver;
const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM
constexpr int TENSOR_ARENA_SIZE = 8 * 1024;
byte tensorArena[TENSOR_ARENA_SIZE] __attribute__((aligned(16)));

// Array to map gesture index to a name
const char* gestureNames[] = {
  "c_up_push",
  "other"
};

// Index of the correct gesture
constexpr int CORRECT_GESTURE_INDEX = 0;

#define NUM_GESTURES (sizeof(gestureNames) / sizeof(gestureNames[0]))

// String to calculate the local and device name
String name;

BLEService* openDoorService = nullptr;
BLEUnsignedCharCharacteristic* openDoorCharacteristic = nullptr;

void setup() {
  Serial.begin(9600);

  // Initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("Failed to initialize Bluetooth!");
    while (1);
  }

  openDoorService = new BLEService("be28eab6-8cc7-4d63-a396-134e1e26fb1d");
  openDoorCharacteristic = new BLEUnsignedCharCharacteristic("e04ff209-6ae4-45e3-8a38-0baefdba9b53", BLERead | BLENotify);

  String address = BLE.address();

  Serial.print("address = ");
  Serial.println(address);

  address.toUpperCase();

  name = "BLESense-";
  name += address[address.length() - 5];
  name += address[address.length() - 4];
  name += address[address.length() - 2];
  name += address[address.length() - 1];

  Serial.print("name = ");
  Serial.println(name);

  BLE.setLocalName(name.c_str());
  BLE.setAdvertisedService(*openDoorService);
  openDoorService->addCharacteristic(*openDoorCharacteristic);
  BLE.addService(*openDoorService);
  openDoorCharacteristic->writeValue(0);

  BLE.advertise();

  // Get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, TENSOR_ARENA_SIZE, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    // Print the central's BT address
    Serial.println(central.address());

    while (central.connected()) {
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
          } else {
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

          // Normalize the IMU data between 0 and 1, and store it in the model's input tensor
          tflInputTensor->data.f[samplesRead * 6 + 0] = (accelerometerX + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 1] = (accelerometerY + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 2] = (accelerometerZ + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 3] = (gyroscopeX + 2000.0) / 4000.0;
          tflInputTensor->data.f[samplesRead * 6 + 4] = (gyroscopeY + 2000.0) / 4000.0;
          tflInputTensor->data.f[samplesRead * 6 + 5] = (gyroscopeZ + 2000.0) / 4000.0;

          samplesRead++;

          if (samplesRead == NUM_SAMPLES) {
            // Run inference
            TfLiteStatus invokeStatus = tflInterpreter->Invoke();
            if (invokeStatus != kTfLiteOk) {
              Serial.println("Invoke failed!");
              while (1);
              return;
            }

            // Loop through the output tensor values from the model
            for (int i = 0; i < NUM_GESTURES; i++) {
              Serial.print(gestureNames[i]);
              Serial.print(": ");
              Serial.println(tflOutputTensor->data.f[i], 6);
              // Check if the read gesture is the correct one
              if (i == CORRECT_GESTURE_INDEX && tflOutputTensor->data.f[i] > 0.90) {
                Serial.println("Correct gesture detected.");
                openDoorCharacteristic->writeValue(1);
              }
            }
            Serial.println();
          }
        }
      }
    }
  }
}
