#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp32/rom/gpio.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_nn.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

#include "model_data.h"
#include "i2c_functions.h"

#define ONBOARD_LED  GPIO_NUM_0

bool led_state = false;
bool sample_ready = false;

float input_data[1][1000][1][1];

#define SAMPLES_COUNT 2000
float buffer[SAMPLES_COUNT]; //Circular Buffer
uint16_t buffer_head = 0;

// Define the model data, tensor arena, and their respective sizes
constexpr int kTensorArenaSize = 150000; // Define the size of the tensor arena buffer

// Create a buffer for the interpreter tensor arena
uint8_t EXT_RAM_BSS_ATTR tensor_arena[kTensorArenaSize];

tflite::MicroInterpreter* interpreter = nullptr;


IRAM_ATTR void data_collector(void* parameter) {
	while (1) {
		buffer_head = 0;
		uint64_t microseconds1 = esp_timer_get_time();
		uint64_t last_sample = esp_timer_get_time();
		while (buffer_head < 1700) {
			if (esp_timer_get_time() - last_sample > 588) {
				last_sample = esp_timer_get_time();
				uint8_t* data_pointer = MPU_read(0x3f, 2);
				buffer[buffer_head] = ((float)((uint16_t)(data_pointer[0] << 8) | (data_pointer[1])) / 32768.0);
				buffer_head++;

				//printf("Timer triggred");
				free(data_pointer);
				sample_ready = false;
			}
		}

		uint64_t microseconds2 = esp_timer_get_time();
		uint64_t diff = microseconds2 - microseconds1;

		//printf("Time in microseconds: %lld, head at: %d\n", diff, buffer_head);

		// Reset the Watchdog Timer
		vTaskDelay(1);
	}

}

extern "C" IRAM_ATTR void app_main() {
	i2c_master_init();
	MPU_write(0x6b, 0x00); //Set Power Registers

	gpio_pad_select_gpio(ONBOARD_LED);
	gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);
	gpio_set_level(ONBOARD_LED, led_state);

	led_state = !led_state;

	// Set up the model
	const tflite::Model* model = tflite::GetModel(model_data);
	tflite::MicroErrorReporter micro_error_reporter;
	constexpr int kOpResolverSize = 7; // Define the number of supported operations
	tflite::MicroMutableOpResolver<kOpResolverSize> micro_op_resolver;

	micro_op_resolver.AddAveragePool2D();
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddFullyConnected();
	micro_op_resolver.AddRelu();
	micro_op_resolver.AddReshape(); //  https://stackoverflow.com/questions/62580548/is-the-keras-function-flatten-supported-by-tensorflow-lite
	micro_op_resolver.AddSoftmax();
	micro_op_resolver.AddLogistic();

	tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
	interpreter = &static_interpreter;

	// Allocate memory from the tensor_arena for the model's tensors.
	TfLiteStatus allocate_status = interpreter->AllocateTensors();
	if (allocate_status != kTfLiteOk) {
		printf("allocateTensors() failed");
		return;
	}
	printf("Success \n");

	TfLiteTensor* input_tensor = interpreter->input_tensor(0);
	TfLiteTensor* output_tensor = interpreter->output_tensor(0);

	xTaskCreatePinnedToCore(data_collector, "data_collector", configMINIMAL_STACK_SIZE, NULL, 1, NULL, 1);

	while (1) {
		uint64_t microseconds1 = esp_timer_get_time();
		uint16_t tail = buffer_head;

		for (int i = 1000; i >= 0; i--) {
			input_data[0][i][0][0] = buffer[tail];
			tail--;
			if (tail > 1800) {
				tail = 1700;
			}
			//printf("%d ", tail);
		}
		memcpy(input_tensor->data.f, input_data, sizeof(input_data));

		TfLiteStatus invoke_status = interpreter->Invoke();
		if (invoke_status != kTfLiteOk) {
			printf("Invoke Failed");
		}

		// Access the output tensor data
		float* output_data = output_tensor->data.f;

		// Print the output values
		// Get the size of the output tensor
		int output_size = 1;
		for (int i = 0; i < output_tensor->dims->size; i++) {
			output_size *= output_tensor->dims->data[i];
		}

		// Print the output values
		printf("Output: ");
		for (int i = 0; i < output_size; i++) {
			printf("%f ", output_data[i]);
		}

		if (output_data[0] > 0.85) {
			gpio_set_level(ONBOARD_LED, led_state);
			led_state = !led_state;
		}

		uint64_t microseconds2 = esp_timer_get_time();
		uint64_t diff = microseconds2 - microseconds1;

		printf("Time in microseconds: %lld \n", diff);
		vTaskDelay(1);

	}
};