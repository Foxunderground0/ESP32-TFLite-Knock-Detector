import os
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import os
import numpy as np
from matplotlib import pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix
import tensorflow as tf
import keras.layers as L
from keras import optimizers, losses, metrics, Model
from keras.callbacks import EarlyStopping
from keras.models import Sequential, load_model
import pandas as pd
from sklearn.preprocessing import OneHotEncoder
import tensorflow_model_optimization as tfmot
import tensorflow_addons as tfa
import serial
import time
import matplotlib.pyplot as plt
import os
import threading
from matplotlib import animation
from scipy.interpolate import interp1d

# Open the serial port at 2000000 baud rate
ser = serial.Serial('COM4', 2000000)

global data
global accuracyArray
accuracyArray = np.array([])

data = np.array(np.zeros(1000), dtype=np.float32)

pruned_model = tf.keras.models.load_model('Train/pruned_model')
pruned_model.compile(loss=losses.BinaryFocalCrossentropy(),
                     metrics=[metrics.BinaryAccuracy()])


def serial_data_reader():
    global data

    while 1:
        line = ser.readline().decode('utf-8').rstrip()
        try:
            dataPoint = float(line)/2
            if (dataPoint >= -1) and (dataPoint <= 1):
                data = np.append(data, [dataPoint])
        except:
            print("Error in reading")


def plot_data():
    global accuracyArray, data
    # Clear the previous plot
    plt.clf()

    # Plot the prediction
    plt.subplot(2, 1, 1)
    plt.plot(accuracyArray[-50:])
    plt.xlabel('Sample')
    plt.ylabel('Value')
    plt.title('Prediction')
    plt.ylim([0.0, 1])

    # Plot the last 1000 elements of the data array
    plt.subplot(2, 1, 2)
    plt.plot(upsampled_array)
    plt.xlabel('Sample')
    plt.ylabel('Value')
    plt.title('Last 1000 Elements')
    plt.ylim([0.25, 1])

    # Modify the y-axis limits according to your data range

    # Adjust the layout to prevent overlapping
    plt.tight_layout()

    # Pause to update the plot
    plt.pause(0.0001)


if __name__ == "__main__":
    # Create an empty array to store data

    # Start the serial data reader in a separate thread
    # Replace "COM1" with your serial port
    serial_thread = threading.Thread(target=serial_data_reader)
    # Set the thread as a daemon so it exits when the main program ends
    serial_thread.daemon = True
    serial_thread.start()

    time.sleep(2)

    # Enable interactive mode
    plt.ion()

    while 1:
        dataa = data[-588:]
        # print(len(data))

        original_array = dataa  # Replace with your original array

        # Define the old and new indices
        old_indices = np.arange(len(original_array))
        new_indices = np.linspace(0, len(original_array) - 1, 1000)

        # Perform cubic interpolation
        f = interp1d(old_indices, original_array, kind='cubic')
        upsampled_array = f(new_indices)
        dataa = upsampled_array
        dataa = np.expand_dims(dataa, axis=-1)
        dataa = np.expand_dims(dataa, axis=0)

        accuracy = pruned_model.predict(dataa, verbose=0)

        accuracyArray = np.append(accuracyArray, accuracy)

        print(accuracy)
        print(len(data))

        plot_data()

        dataa = []

        if (len(data) > 2000):
            data = data[1000:]
        # time.sleep(0.05)
