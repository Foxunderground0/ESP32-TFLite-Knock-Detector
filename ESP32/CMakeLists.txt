# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# list(APPEND CMAKE_PREFIX_PATH "/home/fox/tflite-micro-esp-examples/components/tflite-lib/third_party/flatbuffers/include/")

#include_directories("/home/fox/tflite-micro-esp-examples/components/tflite-lib/third_party/flatbuffers/include/")
#include_directories("/home/fox/tflite-micro-esp-examples/components/tflite-lib/tensorflow/lite/c/")
#include_directories("/home/fox/tflite-micro-esp-examples/components/tflite-lib/tensorflow/lite/core/c/")
#include_directories("/home/fox/tflite-micro-esp-examples/components/tflite-lib/")
#include_directories("/home/fox/tflite-micro-esp-examples/components/tflite-lib/tensorflow/")

set(EXTRA_COMPONENT_DIRS $ENV{HOME}/tflite-micro-esp-examples/components/)

#set(EXTRA_COMPONENT_DIRS components)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ESP32)
