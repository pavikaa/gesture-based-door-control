
Gesture-based Door Control
==========================

Introduction
------------

This project enables you to control a virtual door using an Arduino Nano 33 BLE Sense and a web interface. It involves capturing and classifying data from an accelerometer and gyroscope, and connecting to the Arduino via Bluetooth to open or close the door based on recognized gestures.

Components
----------

### IMU Capture

The `imu_capture` folder contains the code for capturing data from the IMU sensor and printing it on the Serial port. This data, including accelerometer and gyroscope readings, is used for training and classification. The Serial output should be saved in .csv format for model training.

-   `imu_capture.ino`: Arduino sketch that captures IMU data and prints it out on the Serial port.

### IMU Classifier

The `imu_classifier` folder contains the code for classifying gestures using the captured IMU data. It utilizes a machine learning model to classify the performed gestures based on the sensor data. When the correct gesture is detected, a "true" message is sent via the Bluetooth connection, triggering the door opening on the web page.

-   `imu_classifier.ino`: Arduino sketch that runs a machine learning model to classify gestures using the recorded IMU data and informs connected devices via Bluetooth if the correct gesture has been detected.

### Model training
The `model_training` folder contains a Jupyter notebook with a clear step-by-step process of training a model for gesture recognition. To run the notebook open it with [Google Colab](https://colab.research.google.com/). Also included are two sample .csv files used for training the `model.h` file included in the `imu_classifier` folder.

 - `arduino_gesture_model_training.ipynb`: Jupyter notebook that trains a gesture model based on the recorded `.csv` files and encodes it into an Arduino header file.
 

### Web Page

The web page allows you to connect to the Arduino and control the door. It communicates with the Arduino via Bluetooth to receive commands to open or close the door. The door automatically closes after 10 seconds when opened.

-   `index.html`: HTML file that provides the web interface for connecting to the Arduino and controlling the door.
-   `images`: Folder containing open and closed door images.

Usage
-----

1.  Set up the Arduino IDE environment and prepare your Arduino Nano 33 BLE Sense.

2.  Upload the `imu_capture.ino` sketch to the Arduino using the Arduino IDE.

3.  Capture the IMU data by performing gestures (it is recommended to repeat each gesture at least 10 times for better model accuracy).

4.  Save the captured data for later use in training or classification (export the Serial output to a .csv file).

5.  Train a machine learning model using the captured IMU data and export the model to `model.h` by opening the `arduino_gesture_model_training.ipynb` file included in `model_training` folder with [Google Colab](https://colab.research.google.com/) and following the steps inside. Place the `model.h` file in the `imu_classification` folder.

6.  Upload the `imu_classification.ino` sketch to the Arduino.

7.  Host the web page files (`index.html` and the `images` folder) on a web server or open the `index.html` file locally in a web browser.

8.  Open the web page in a web browser.

9.  Click on the "Connect to Arduino" button to establish a Bluetooth connection with the Arduino.

10. The web page will indicate a successful connection by changing the door image opacity and transforming the Connect button into a Disconnect button.

11. Perform gestures, and if the correct gesture is recognized, the door will open. After 10 seconds, the door will automatically close.

Note: Make sure to adjust the Bluetooth service and characteristic UUIDs in the web page code (`index.html`) and the Arduino sketch (`imu_classification.ino`) to match your specific setup. Also, modify `imu_classification.ino` to reflect your trained model.

Dependencies
------------

This project relies on the following technologies, hardware, and libraries:

-   Arduino IDE
-   Arduino Nano 33 BLE board
-   Arduino_LSM9DS1 library
-   TensorFlow Lite Micro library
-   Bluetooth Low Energy (BLE) library for Arduino
-   Web browser with JavaScript and Bluetooth support
