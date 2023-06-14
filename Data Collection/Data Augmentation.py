import os
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
from datetime import datetime


class DataPlotter:
    def __init__(self, ax, data):
        self.ax = ax
        self.data = data

        # Initial plot
        self.x = list(range(len(self.data)))
        self.line, = self.ax.plot(self.x, self.data)

        # Slider properties
        ax_color = 'lightgoldenrodyellow'
        ax_shift = plt.axes([0.2, 0.1, 0.65, 0.03], facecolor=ax_color)
        self.slider_shift = Slider(
            ax_shift, 'Shift', -len(self.data), len(self.data), valinit=0, valstep=1)

        self.slider_shift.on_changed(self.update_plot)

    def update_plot(self, val):
        shift_amount = int(self.slider_shift.val)
        shifted_data = self.data[-shift_amount:] + self.data[:-shift_amount]
        self.line.set_ydata(shifted_data)
        self.ax.figure.canvas.draw_idle()


def read_data_from_file(file_path):
    with open(file_path, 'r') as file:
        data = [float(line.strip()) for line in file]
    return data


def save_rotated_data(data, output_directory, file_name):
    timestamp = datetime.now().strftime("%Y%m%d%H%M%S%f")
    output_file_name = f"{timestamp}_{file_name}"
    output_path = os.path.join(output_directory, output_file_name)
    with open(output_path, 'w') as file:
        for point in data:
            file.write(str(point) + '\n')


def browse_files(directory):
    file_list = sorted([f for f in os.listdir(directory)
                       if os.path.isfile(os.path.join(directory, f))])
    num_files = len(file_list)
    current_index = 0

    fig, ax = plt.subplots()
    plt.subplots_adjust(bottom=0.25)

    plotter = None  # Initialize plotter variable

    def on_key_press(event):
        nonlocal current_index, plotter

        if event.key == 'w':
            # 'w' key pressed
            current_index = (current_index + 1) % num_files
            update_plot()

        elif event.key == ' ':
            # Space key pressed
            if plotter is not None:
                file_name = file_list[current_index]
                file_path = os.path.join(directory, file_name)
                data = read_data_from_file(file_path)
                shifted_data = data[-int(plotter.slider_shift.val):] + \
                    data[:-int(plotter.slider_shift.val)]
                save_rotated_data(shifted_data, 'rotated_data', file_name)
                print(f"Rotated data saved for file: {file_name}")

    def update_plot():
        nonlocal plotter

        file_name = file_list[current_index]
        file_path = os.path.join(directory, file_name)
        data = read_data_from_file(file_path)

        ax.clear()
        ax.set_title(file_name)
        ax.set_xlabel('X')
        ax.set_ylabel('Y')

        shifted_data = data[:]
        plotter = DataPlotter(ax, shifted_data)
        plt.draw()

    fig.canvas.mpl_connect('key_press_event', on_key_press)
    update_plot()
    plt.show()


# Specify the path to your directory
directory_path = r'D:\Programing\Projects\ESP32-TFLite-Knock-Detector\Data Collection\Data\New Data\False'

# Create the output directory if it doesn't exist
output_directory = 'rotated_data'
os.makedirs(output_directory, exist_ok=True)

# Plot and browse through files
browse_files(directory_path)
