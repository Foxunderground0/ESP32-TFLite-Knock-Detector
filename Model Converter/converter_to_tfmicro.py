import subprocess

# Path to the .tflite model
tflite_model_path = 'model_quantized.tflite'

# Path to the output .cc file
output_cc_path = 'D:\Programing\Projects\ESP32-TFLite-Knock-Detector\Train\model_data.cc'

# Convert .tflite to .cc using xxd
subprocess.run(['xxd', '-i', tflite_model_path, output_cc_path])
