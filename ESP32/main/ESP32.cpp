#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp32/rom/gpio.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_log.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

#include "model_data.h"
#include "i2c_functions.h"

#define ONBOARD_LED  GPIO_NUM_0

bool led_state = false;
bool sample_ready = false;

static const char* TAG = "main_task";

const float a[] = { 0.513793945,
0.513793945,
0.51672363,
0.5168457,
0.5168457,
0.51611328,
0.51611328,
0.517578125,
0.517578125,
0.51672363,
0.5135498,
0.5135498,
0.51245117,
0.51245117,
0.51647949,
0.515258785,
0.515258785,
0.514282225,
0.514282225,
0.51708984,
0.51708984,
0.518066405,
0.51647949,
0.51647949,
0.51623535,
0.51623535,
0.52026367,
0.513916015,
0.513916015,
0.515258785,
0.515258785,
0.514526365,
0.514526365,
0.51611328,
0.514648435,
0.514648435,
0.51306152,
0.51306152,
0.51342773,
0.51745605,
0.51745605,
0.5168457,
0.5168457,
0.515014645,
0.51318359,
0.51318359,
0.51269531,
0.51269531,
0.51208496,
0.51208496,
0.51672363,
0.515136715,
0.515136715,
0.518798825,
0.518798825,
0.521606445,
0.51745605,
0.51745605,
0.514892575,
0.514892575,
0.52111816,
0.52111816,
0.52099609,
0.51171875,
0.51171875,
0.514160155,
0.514160155,
0.51623535,
0.51293945,
0.51293945,
0.518066405,
0.518066405,
0.5234375,
0.5234375,
0.51708984,
0.517944335,
0.517944335,
0.517578125,
0.517578125,
0.51599121,
0.51599121,
0.51599121,
0.51721191,
0.51721191,
0.51977539,
0.51977539,
0.522705075,
0.52111816,
0.52111816,
0.51745605,
0.51745605,
0.518432615,
0.51245117,
0.51245117,
0.51306152,
0.51306152,
0.51623535,
0.51623535,
0.514404295,
0.51196289,
0.51196289,
0.51342773,
0.51342773,
0.51586914,
0.51269531,
0.51269531,
0.51184082,
0.51184082,
0.513793945,
0.518432615,
0.518432615,
0.514282225,
0.514282225,
0.514038085,
0.514038085,
0.515258785,
0.513793945,
0.513793945,
0.511230465,
0.511230465,
0.5123291,
0.515625,
0.515625,
0.51220703,
0.51220703,
0.515380855,
0.515380855,
0.51220703,
0.5123291,
0.5123291,
0.5168457,
0.5168457,
0.518066405,
0.60961914,
0.60961914,
0.82165527,
0.82165527,
-0.452758785,
-0.452758785,
0.99996948,
-0.79370117,
-0.79370117,
-0.267944335,
-0.267944335,
0.99996948,
0.830810545,
0.830810545,
0.99996948,
0.99996948,
0.638793945,
0.638793945,
0.588867185,
0.725830075,
0.725830075,
0.342285155,
0.342285155,
0.99996948,
0.53259277,
0.53259277,
0.264648435,
0.264648435,
0.350097655,
0.350097655,
0.2668457,
0.429443355,
0.429443355,
0.635986325,
0.635986325,
0.99996948,
0.30578613,
0.30578613,
-0.382568355,
-0.382568355,
0.40771484,
0.82543945,
0.82543945,
0.55639648,
0.55639648,
0.447631835,
0.447631835,
0.4576416,
0.459228515,
0.459228515,
0.34533691,
0.34533691,
0.3125,
0.373168945,
0.373168945,
0.514282225,
0.514282225,
0.49389648,
0.49389648,
0.62634277,
0.577148435,
0.577148435,
0.6385498,
0.6385498,
0.72436523,
0.640380855,
0.640380855,
0.677978515,
0.677978515,
0.674560545,
0.674560545,
0.53991699,
0.51196289,
0.51196289,
0.48901367,
0.48901367,
0.53662109,
0.49694824,
0.49694824,
0.459228515,
0.459228515,
0.432495115,
0.432495115,
0.401367185,
0.42675781,
0.42675781,
0.514160155,
0.514160155,
0.54846191,
0.53186035,
0.53186035,
0.495239255,
0.495239255,
0.436889645,
0.45422363,
0.45422363,
0.47375488,
0.47375488,
0.45056152,
0.45056152,
0.494995115,
0.476196285,
0.476196285,
0.495727535,
0.495727535,
0.52905273,
0.523315425,
0.523315425,
0.565307615,
0.565307615,
0.578002925,
0.578002925,
0.576782225,
0.57189941,
0.57189941,
0.545166015,
0.545166015,
0.550659175,
0.534423825,
0.534423825,
0.5213623,
0.5213623,
0.52392578,
0.52392578,
0.51257324,
0.501953125,
0.501953125,
0.501953125,
0.501953125,
0.50500488,
0.52111816,
0.52111816,
0.52380371,
0.52380371,
0.518920895,
0.518920895,
0.502441405,
0.479003905,
0.479003905,
0.4765625,
0.4765625,
0.483642575,
0.487304685,
0.487304685,
0.48498535,
0.48498535,
0.486572265,
0.486572265,
0.498168945,
0.51647949,
0.51647949,
0.53210449,
0.53210449,
0.550048825,
0.548950195,
0.548950195,
0.54370117,
0.54370117,
0.537719725,
0.53515625,
0.53515625,
0.529418945,
0.529418945,
0.52026367,
0.52026367,
0.517578125,
0.51660156,
0.51660156,
0.509887695,
0.509887695,
0.514648435,
0.517578125,
0.517578125,
0.514282225,
0.514282225,
0.510620115,
0.510620115,
0.510009765,
0.510253905,
0.510253905,
0.50939941,
0.50939941,
0.505981445,
0.5057373,
0.5057373,
0.49780273,
0.49780273,
0.494995115,
0.494995115,
0.50524902,
0.502075195,
0.502075195,
0.506469725,
0.506469725,
0.517700195,
0.51574707,
0.51574707,
0.51696777,
0.51696777,
0.52502441,
0.52502441,
0.521972655,
0.52392578,
0.52392578,
0.521484375,
0.521484375,
0.5213623,
0.51721191,
0.51721191,
0.514892575,
0.514892575,
0.5168457,
0.5168457,
0.522094725,
0.522460935,
0.522460935,
0.5201416,
0.5201416,
0.518920895,
0.51953125,
0.51953125,
0.52502441,
0.52502441,
0.52514648,
0.518554685,
0.518554685,
0.51696777,
0.51696777,
0.518188475,
0.518188475,
0.514404295,
0.511108395,
0.511108395,
0.505859375,
0.505859375,
0.5090332,
0.51196289,
0.51196289,
0.515014645,
0.515014645,
0.50964355,
0.50964355,
0.51318359,
0.514892575,
0.514892575,
0.515380855,
0.515380855,
0.5168457,
0.52124023,
0.52124023,
0.52392578,
0.52392578,
0.5168457,
0.5168457,
0.518676755,
0.521850585,
0.521850585,
0.514160155,
0.514160155,
0.515502925,
0.51342773,
0.51342773,
0.507324215,
0.507324215,
0.507446285,
0.507446285,
0.51184082,
0.514648435,
0.514648435,
0.51721191,
0.51721191,
0.52001953,
0.51721191,
0.51721191,
0.519042965,
0.519042965,
0.476196285,
0.99996948,
0.99996948,
0.238159175,
0.238159175,
-1,
-1,
0.61450195,
0.45349121,
0.45349121,
0.84069824,
0.84069824,
0.63830566,
0.62963867,
0.62963867,
0.98913574,
0.98913574,
0.948486325,
0.948486325,
0.276855465,
0.82849121,
0.82849121,
0.569213865,
0.569213865,
0.186889645,
0.530395505,
0.530395505,
0.89611816,
0.89611816,
0.495849605,
0.495849605,
0.47839355,
0.35620117,
0.35620117,
0.550537105,
0.550537105,
0.91479492,
0.109130855,
0.109130855,
0.351318355,
0.351318355,
0.718261715,
0.718261715,
0.726196285,
0.58251953,
0.58251953,
0.01611328,
0.01611328,
0.56286621,
0.32495117,
0.32495117,
0.33776855,
0.33776855,
0.40673828,
0.49230957,
0.49230957,
0.59521484,
0.59521484,
0.537841795,
0.537841795,
0.57043457,
0.690917965,
0.690917965,
0.577148435,
0.577148435,
0.580566405,
0.59765625,
0.59765625,
0.623046875,
0.623046875,
0.5625,
0.5625,
0.514770505,
0.545532225,
0.545532225,
0.52099609,
0.52099609,
0.453125,
0.43762207,
0.43762207,
0.45361328,
0.45361328,
0.502441405,
0.502441405,
0.558105465,
0.53527832,
0.53527832,
0.53613281,
0.53613281,
0.495483395,
0.460083005,
0.460083005,
0.49609375,
0.49609375,
0.48913574,
0.48913574,
0.47668457,
0.459838865,
0.459838865,
0.45703125,
0.45703125,
0.499511715,
0.49401855,
0.49401855,
0.507568355,
0.507568355,
0.525756835,
0.507324215,
0.507324215,
0.526489255,
0.526489255,
0.54736328,
0.54736328,
0.556884765,
0.549804685,
0.549804685,
0.518432615,
0.518432615,
0.510131835,
0.522949215,
0.522949215,
0.513916015,
0.513916015,
0.521484375,
0.521484375,
0.52062988,
0.514160155,
0.514160155,
0.51672363,
0.51672363,
0.53186035,
0.53308105,
0.53308105,
0.52368164,
0.52368164,
0.502441405,
0.502441405,
0.49609375,
0.499755855,
0.499755855,
0.498168945,
0.498168945,
0.49243164,
0.486938475,
0.486938475,
0.50500488,
0.50500488,
0.507324215,
0.507324215,
0.51733398,
0.534545895,
0.534545895,
0.5390625,
0.5390625,
0.534667965,
0.530273435,
0.530273435,
0.53295898,
0.53295898,
0.530517575,
0.530517575,
0.521728515,
0.510498045,
0.510498045,
0.511474605,
0.511474605,
0.518188475,
0.526367185,
0.526367185,
0.52429199,
0.52429199,
0.519287105,
0.517944335,
0.517944335,
0.518676755,
0.518676755,
0.5168457,
0.5168457,
0.514160155,
0.51196289,
0.51196289,
0.513793945,
0.513793945,
0.506469725,
0.506591795,
0.506591795,
0.510498045,
0.510498045,
0.506469725,
0.506469725,
0.50134277,
0.50524902,
0.50524902,
0.50927734,
0.50927734,
0.510620115,
0.515258785,
0.515258785,
0.517822265,
0.517822265,
0.52062988,
0.52062988,
0.51586914,
0.514892575,
0.514892575,
0.51306152,
0.51306152,
0.5135498,
0.513671875,
0.513671875,
0.515380855,
0.515380855,
0.514770505,
0.514770505,
0.51330566,
0.521484375,
0.521484375,
0.522094725,
0.522094725,
0.522949215,
0.51708984,
0.51708984,
0.5168457,
0.5168457,
0.52075195,
0.52075195,
0.51721191,
0.514770505,
0.514770505,
0.514648435,
0.514648435,
0.51269531,
0.51220703,
0.51220703,
0.513671875,
0.513671875,
0.51293945,
0.51293945,
0.51171875,
0.514038085,
0.514038085,
0.51672363,
0.51672363,
0.51745605,
0.521850585,
0.521850585,
0.521728515,
0.521728515,
0.518310545,
0.51696777,
0.51696777,
0.521606445,
0.521606445,
0.522460935,
0.522460935,
0.51733398,
0.51599121,
0.51599121,
0.51708984,
0.51708984,
0.514770505,
0.510986325,
0.510986325,
0.51733398,
0.51733398,
0.51733398,
0.51733398,
0.5135498,
0.515625,
0.515625,
0.518920895,
0.518920895,
0.517822265,
0.52001953,
0.52001953,
0.522094725,
0.522094725,
0.51977539,
0.51977539,
0.515502925,
0.50854492,
0.50854492,
0.50524902,
0.50524902,
0.50952148,
0.50964355,
0.50964355,
0.507202145,
0.507202145,
0.509887695,
0.509887695,
0.50463867,
0.50964355,
0.50964355,
0.515014645,
0.515014645,
0.51977539,
0.51672363,
0.51672363,
0.519165035,
0.519165035,
0.52124023,
0.52124023,
0.515502925,
0.51318359,
0.51318359,
0.514648435,
0.514648435,
0.51660156,
0.513916015,
0.513916015,
0.51611328,
0.51611328,
0.51586914,
0.51306152,
0.51306152,
0.51330566,
0.51330566,
0.51696777,
0.51696777,
0.52038574,
0.52062988,
0.52062988,
0.52050781,
0.52050781,
0.522216795,
0.517822265,
0.517822265,
0.517822265,
0.517822265,
0.51953125,
0.51953125,
0.521484375,
0.519042965,
0.519042965,
0.51745605,
0.51745605,
0.51623535,
0.51623535,
0.51623535,
0.51733398,
0.51733398,
0.519287105,
0.519287105,
0.518676755,
0.51965332,
0.51965332,
0.52087402,
0.52087402,
0.518554685,
0.5168457,
0.5168457,
0.517700195,
0.517700195,
0.518798825,
0.518798825,
0.5135498,
0.51574707,
0.51574707,
0.51586914,
0.51586914,
0.515502925,
0.513671875,
0.513671875,
0.51733398,
0.51733398,
0.51733398,
0.51733398,
0.51257324,
0.518432615,
0.518432615,
0.5168457,
0.5168457,
0.515136715,
0.51293945,
0.51293945,
0.51586914,
0.51586914,
0.51708984,
0.515625,
0.515625,
0.514526365,
0.514526365,
0.515014645,
0.515014645,
0.515258785,
0.51623535,
0.51623535,
0.51318359,
0.51318359,
0.51171875,
0.514404295,
0.514404295,
0.51623535,
0.51623535,
0.514770505,
0.514770505,
0.51721191,
0.513671875,
0.513671875,
0.515258785,
0.515258785,
0.514770505,
0.515258785,
0.515258785,
0.514038085,
0.514038085,
0.513916015,
0.513916015,
0.513916015,
0.518188475,
0.518188475,
0.51965332,
0.51965332,
0.518432615,
0.513916015,
0.513916015,
0.514282225,
0.514282225,
0.52001953,
0.52001953,
0.51623535,
0.515258785,
0.515258785,
0.51293945,
0.51293945,
0.52124023,
0.522705075,
0.522705075,
0.522094725,
0.522094725,
0.52038574,
0.52038574,
0.514404295,
0.511108395,
0.511108395,
0.51257324,
0.51257324,
0.517822265,
0.52026367,
0.52026367,
0.519287105,
0.519287105,
0.5201416,
0.5168457,
0.5168457,
0.51696777,
0.51696777,
0.519287105,
0.519287105,
0.51647949,
0.518676755,
0.518676755,
0.513671875,
0.513671875,
0.518188475,
0.514160155,
0.514160155,
0.51318359,
0.51318359,
0.515136715,
0.515136715,
0.513793945,
0.51293945,
0.51293945,
0.513916015,
0.513916015,
0.51672363,
0.51330566,
0.51330566,
0.51293945,
0.51293945,
0.51623535,
0.51623535,
0.514892575,
0.51342773,
0.51342773,
0.51306152,
0.51306152,
0.51953125,
0.51647949,
0.51647949,
0.517944335,
0.517944335,
0.518310545,
0.518310545,
0.51599121,
0.515380855,
0.515380855,
0.515625,
0.515625,
0.515014645,
0.510864255,
0.510864255,
0.511474605,
0.511474605,
0.51672363,
0.51672363,
0.521606445,
0.51647949,
0.51647949,
0.514892575,
0.514892575,
0.518432615,
0.513793945,
0.513793945,
0.514160155,
0.514160155,
0.514526365,
0.515380855,
0.515380855,
0.518554685,
0.518554685,
0.515625,
0.515625,
0.52062988,
0.517700195,
0.517700195,
0.514648435,
0.514648435,
0.51953125,
0.518066405,
0.518066405,
0.50964355,
0.50964355,
0.51281738,
0.51281738,
0.519165035,
0.518554685,
0.518554685,
0.522094725,
0.522094725,
0.522338865,
0.518676755,
0.518676755,
0.51672363,
0.51672363,
0.518554685,
0.518554685,
0.5201416,
0.51977539,
0.51977539,
0.514892575,
0.514892575,
0.514282225,
0.515380855,
0.515380855,
0.515014645,
0.515014645,
0.515625,
0.515625,
0.51660156,
0.517822265,
0.517822265,
0.510864255,
0.510864255,
0.51257324,
0.51660156,
0.51660156,
0.51208496,
0.51208496,
0.51208496,
0.51208496,
0.51293945,
0.507202145,
0.507202145,
0.51208496,
0.51208496,
0.5201416,
0.518188475,
0.518188475,
0.51586914,
0.51586914,
0.511474605,
0.511474605,
0.5123291,
0.511230465,
0.511230465,
0.50476074,
0.50476074,
0.511230465,
0.514892575,
0.514892575,
0.514648435,
0.514648435,
0.515502925,
0.513916015,
0.513916015,
0.51623535,
0.51623535,
0.51330566,
};

float input_data[1][1000][1][1];


#define SAMPLES_COUNT 2000
float buffer[SAMPLES_COUNT]; //Circular Buffer
uint16_t buffer_head = 0;

#define INTERRUPT_FREQ 1700
#define TIMER_DIVIDER (TIMER_BASE_CLK / INTERRUPT_FREQ)

#define TIMER_GROUP TIMER_GROUP_0
#define TIMER_NUM TIMER_0

// Define the model data, tensor arena, and their respective sizes
constexpr int kTensorArenaSize = 150000; // Define the size of the tensor arena buffer

// Create a buffer for the interpreter tensor arena
uint8_t EXT_RAM_BSS_ATTR tensor_arena[kTensorArenaSize];

tflite::MicroInterpreter* interpreter = nullptr;

// Interrupt service routine (ISR)
bool IRAM_ATTR timer_isr(gptimer_t* gptimer, const gptimer_alarm_event_data_t* event_data, void* arg) {
	sample_ready = true;
	gpio_set_level(ONBOARD_LED, led_state);
	led_state = !led_state;
	return true;
}

extern "C" void app_main() {
	i2c_master_init();
	MPU_write(0x6b, 0x00); //Set Power Registers

	gpio_pad_select_gpio(ONBOARD_LED);
	gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);
	gpio_set_level(ONBOARD_LED, led_state);
	led_state = !led_state;

	ESP_LOGI(TAG, "Create timer handle");

	gptimer_handle_t gptimer = NULL;

	gptimer_config_t timer_config = {
	.clk_src = GPTIMER_CLK_SRC_DEFAULT,
	.direction = GPTIMER_COUNT_UP,
	.resolution_hz = 20000000, // 10MHz, 1 tick=1us
	.flags = {0}
	};

	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

	gptimer_event_callbacks_t cbs = {
		.on_alarm = timer_isr,
	};

	ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
	ESP_LOGI(TAG, "Enable timer");
	ESP_ERROR_CHECK(gptimer_enable(gptimer));
	ESP_LOGI(TAG, "Start timer, stop it at alarm event");

	gptimer_alarm_config_t alarm_config1 = {
		.alarm_count = 11764,// 1700 times a second
		.reload_count = 0,
		.flags = {
			.auto_reload_on_alarm = true
		}
	};
	*/

		// Set up the model
		const tflite::Model * model = tflite::GetModel(model_data);
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

	//micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());

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


	// Assuming input_data is your input data array with shape (1, 1000, 1, 1)
	for (int i = 0; i < 1000; i++) {
		input_data[0][i][0][0] = a[i];
	}

	memcpy(input_tensor->data.f, input_data, sizeof(input_data));

	TfLiteStatus invoke_status = interpreter->Invoke();
	if (invoke_status != kTfLiteOk) {
		printf("Invoke Failed");
	}

	// Access the output tensor data
	float* output_data = output_tensor->data.f;

	// Print the output values
	printf("Output 0: %f\n", output_data[0]);
	printf("Output 1: %f\n", output_data[1]);

	/*
	int inputTensorIndex = 0;  // Adjust the index based on your model
	int outputTensorIndex = 0;  // Adjust the index based on your model

	const TfLiteTensor* inputTensor = interpreter->input_tensor(inputTensorIndex);
	if (inputTensor->type != kTfLiteUInt8) {
		// Handle error: Input tensor is not quantized to 8 bits
	}

	const float inputScale = inputTensor->params.scale;
	const int inputZeroPoint = inputTensor->params.zero_point;

	printf("Input Scale: %f, Zero Point: %d\n", inputScale, inputZeroPoint);
	*/

	/*
		ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
		ESP_ERROR_CHECK(gptimer_start(gptimer));


		uint64_t microseconds1 = esp_timer_get_time();

		while (buffer_head < 1700) {
			if (sample_ready) {
				uint8_t* data_pointer = MPU_read(0x3f, 2);
				buffer[buffer_head] = (float)((uint16_t)(data_pointer[0] << 8) | (data_pointer[1]));
				buffer_head++;
				//printf("Timer triggred");
				free(data_pointer);
				sample_ready = false;
			}
		}

		uint64_t microseconds2 = esp_timer_get_time();
		uint64_t diff = microseconds2 - microseconds1;

		printf("Time in microseconds: %lld, head at: %d\n", diff, buffer_head);
	*/
};