import serial
import time
import matplotlib.pyplot as plt
import os

# Open the serial port at 2000000 baud rate
ser = serial.Serial('COM4', 2000000)

data = []

# Wait for 2 seconds for the Arduino to initialize
time.sleep(1)

# Define a function to plot the data


def plot_data():
    time.sleep(0.25)

    # Send the "a" character with a newline character to trigger the Arduino to start sending data
    ser.write(b'a\n')

    # Read 1000 data points from the serial port
    global data
    data = []
    for i in range(1000):
        line = ser.readline().decode('utf-8').rstrip()
        data.append(float(line))

    # Plot the data
    plt.plot(data)

    # Set the axis labels and title
    plt.xlabel('Sample')
    plt.ylabel('Value')
    plt.title('Serial Data Plot')

    # Set the y-axis limits
    plt.ylim([0.2, 1.8])

    # Show the plot
    plt.show(block=False)

# Define a function to handle key presses


def on_key_press(event):
    global data

    # Check if the pressed key is the spacebar
    if event.key == ' ':
        # Clear the current plot
        plt.clf()

        # Plot the new data
        plot_data()

    # Check if the pressed key is the enter key
    if event.key == 'enter':
        # Get the current date and time for the filename
        timestamp = time.strftime('%Y%m%d_%H%M%S')

        # Save the data to a text file in the "True" directory
        directory = "True"
        file_path = os.path.join(directory, f'data_{timestamp}.txt')
        with open(file_path, 'w') as f:
            for d in data:
                f.write(f'{d:.8f}\n')

    # Check if the pressed key is the shift key
    if event.key == 'shift':
        # Get the current date and time for the filename
        timestamp = time.strftime('%Y%m%d_%H%M%S')

        # Save the data to a text file in the "False" directory
        directory = "False"
        file_path = os.path.join(directory, f'data_{timestamp}.txt')
        with open(file_path, 'w') as f:
            for d in data:
                f.write(f'{d:.8f}\n')


# Connect the key press event to the on_key_press function
plt.connect('key_press_event', on_key_press)

# Plot the initial data
plot_data()

# Show the plot and wait for key presses
plt.show()

# Close the serial port
ser.close()
