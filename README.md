# ESP32-TFLite-Knock-Detector

## Introduction
The ESP32 Knock Detector project aims to create a self-contained system using an ESP32 WROVER microcontroller that can detect knocks and trigger an action, such as turning on a desk lamp. The project utilizes an accelerometer (MPU6050) to sense vibrations and classify them as knock or non-knock events. This write-up provides an overview of the project's development process and key components.

## Project Timeline
* ### Data Collection
  - Utilize Arduino Mega and MPU6050 accelerometer for data capture
  - Store accelerometer data from the Z-axis into a buffer
  - Transfer buffered data to Python program for visualization and processing
  - Select and store data files representing knock and non-knock events
* ### Data Visualization and Verification
  - Develop Python script to display and verify data files
  - Visualize nine data files simultaneously for quick inspection
  - Ensure accurate labeling and reduce the chance of false positives
* ### AI Model Training
  - Train an AI model using the collected and labeled accelerometer data
  - Utilize Keras and TensorFlow to create a Sequential model
  - Implement Conv1D and Dense layers with dilated convolutions
  - Achieve high accuracy and validation accuracy for knock classification
* ### Model Pruning
  - Apply model pruning using TensorFlow Model Optimization library
  - Reduce model size by applying sparsity to model parameters
  - Maintain high accuracy while significantly reducing model file size
* ### Model Quantization
  - Quantize the pruned model to 8-bit integer weights
  - Improve memory usage and inference speed on resource-constrained devices
  - Further reduce the model size while preserving reasonable accuracy
* ### ESP32 Integration
  - Integrate the pruned and quantized model with ESP32 WROVER microcontroller
  - Program the microcontroller to load and process accelerometer data in real-time
  - Trigger specific actions, such as controlling a relay, upon detecting two consecutive knocks

## Conclusion
The ESP32 Knock Detector project aims to create a reliable and efficient system for detecting knocks and triggering actions using an ESP32 WROVER microcontroller. By combining data collection, AI model training, model optimization, and microcontroller integration, the project enables the detection of knock events in real-time. The ultimate goal is to create a self-contained system that can be easily deployed and used for various applications, such as home automation or security systems.

Please note that the project specifically focuses on using the Z-axis data from the accelerometer and requires two consecutive knocks to activate the system.
