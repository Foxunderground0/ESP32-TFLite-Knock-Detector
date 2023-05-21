# ESP32-TFLite-Knock-Detector

## Introduction

The ESP32 Knock Detector project aims to create a self-contained system using an ESP32 WROVER microcontroller that can detect knocks and trigger an action, such as turning on a desk lamp. The project utilizes an accelerometer (MPU6050) to sense vibrations and classify them as knock or non-knock events. This write-up provides an overview of the project's development process and key components.

## Project Timeline

-   ### Data Collection

    Data collection is a crucial step in training the knock detection model. The project utilizes an Arduino Mega and an MPU6050 accelerometer to capture accelerometer data. The Arduino Mega reads and stores the data from the MPU6050 in the Z-axis into a buffer. This buffered data is then transferred to a Python program for visualization and further processing.

    The Python program allows selecting and storing data files in the 'True' and 'False' directories, representing knock and non-knock events, respectively. Each data file contains 1000 samples collected from the accelerometer.

-   ### Data Visualization and Verification

    To verify the collected data and identify any mislabeled samples, another Python script is used. This script displays nine data files simultaneously, allowing for quick visualization and inspection. This visual inspection helps ensure the accuracy of the labeling process and reduces the chance of false positives.

    <video width="100%" controls>
    <source src="Visualiser.mp4"   type="video/mp4">
    Your browser does not support the video tag.
    </video>

-   ### AI Model Training

    The collected and labeled accelerometer data is used to train an AI model for knock classification. The training data consists of the 1000-sample data files stored in the 'True' and 'False' directories. A Sequential model is created using Keras and TensorFlow, comprising multiple Conv1D and Dense layers. The model architecture includes dilated convolutions to capture temporal dependencies effectively.

    The trained AI model achieves an accuracy of 99.71% and a validation accuracy of 99.29%, demonstrating its ability to accurately classify knock events.

-   ### Model Pruning

    To optimize the trained model and reduce its size, model pruning is applied. The TensorFlow Model Optimization library is used to prune the model by applying sparsity to the model's parameters. Pruning the model reduces its size while maintaining a high level of accuracy. The pruned model achieves a significant reduction in size, reducing the model file from 1.82 MB to 673 KB.

-   ### Model Quantization

    To prepare the model for deployment on the ESP32 WROVER microcontroller, quantization is applied. Quantization converts the model to use 8-bit integer weights, reducing the memory requirements and improving inference speed on resource-constrained devices. The quantized model further reduces the model size to 182 KB while maintaining reasonable accuracy.

    <video width="100%" controls>
    <source src="Prediction.mp4"   type="video/mp4">
    Your browser does not support the video tag.
    </video>

-   ### ESP32 Integration
    The next step of the project involves integrating the pruned and quantized model with the ESP32 WROVER microcontroller. Once the microcontroller arrives, it will be programmed to load the model and process accelerometer data in real-time. When the model detects two consecutive knocks, it will trigger a specific action, such as turning on a relay to control a desk lamp or other devices.

## Conclusion

The ESP32 Knock Detector project aims to create a reliable and efficient system for detecting knocks and triggering actions using an ESP32 WROVER microcontroller. By combining data collection, AI model training, model optimization, and microcontroller integration, the project enables the detection of knock events in real-time. The ultimate goal is to create a self-contained system that can be easily deployed and used for various applications, such as home automation or security systems.

Please note that the project specifically focuses on using the Z-axis data from the accelerometer and requires two consecutive knocks to activate the system.
