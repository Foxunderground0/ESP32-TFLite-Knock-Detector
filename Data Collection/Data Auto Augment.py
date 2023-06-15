import os
from datetime import datetime
import random


def read_data_from_file(file_path):
    with open(file_path, 'r') as file:
        data = [float(line.strip()) for line in file]
    return data


def save_rotated_data(rotated_data_list, output_directory, file_name):
    os.makedirs(output_directory, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d%H%M%S%f")

    for i, rotated_data in enumerate(rotated_data_list):
        output_file_name = f"{timestamp}_{file_name}_{i}.txt"
        output_path = os.path.join(output_directory, output_file_name)

        with open(output_path, 'w') as file:
            for point in rotated_data:
                file.write(str(point) + '\n')


def rotate_data(data, shift_amount):
    shifted_data = data[-shift_amount:] + data[:-shift_amount]
    return shifted_data


def process_files(directory):
    file_list = sorted([f for f in os.listdir(directory)
                       if os.path.isfile(os.path.join(directory, f))])

    # Create the output directory if it doesn't exist
    output_directory = 'rotated_data'

    for file_name in file_list:
        file_path = os.path.join(directory, file_name)
        data = read_data_from_file(file_path)

        rotated_data_list = []
        for _ in range(100):
            # Generate a random shift amount
            shift_amount = random.randint(1, len(data))
            shifted_data = rotate_data(data, shift_amount)
            rotated_data_list.append(shifted_data)

        save_rotated_data(rotated_data_list, output_directory, file_name)
        print(f"Rotated data saved for file: {file_name}")


# Specify the path to your directory
directory_path = r'D:\Programing\Projects\ESP32-TFLite-Knock-Detector\Data Collection\Data\New Data\True'

# Process files
process_files(directory_path)
