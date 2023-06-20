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

const float a[] = { 0.51635742,
0.515502925,
0.515502925,
0.51611328,
0.51611328,
0.52026367,
0.51623535,
0.51623535,
0.517578125,
0.517578125,
0.518432615,
0.518432615,
0.51733398,
0.515625,
0.515625,
0.52001953,
0.52001953,
0.51708984,
0.51586914,
0.51586914,
0.518798825,
0.518798825,
0.515380855,
0.514404295,
0.514404295,
0.515380855,
0.515380855,
0.514160155,
0.514160155,
0.519042965,
0.51623535,
0.51623535,
0.515136715,
0.515136715,
0.515014645,
0.517822265,
0.517822265,
0.51306152,
0.51306152,
0.51635742,
0.51635742,
0.51599121,
0.51721191,
0.51721191,
0.515136715,
0.515136715,
0.517578125,
0.5201416,
0.5201416,
0.517700195,
0.517700195,
0.514526365,
0.514526365,
0.514282225,
0.515625,
0.515625,
0.515014645,
0.515014645,
0.518188475,
0.51647949,
0.51647949,
0.515136715,
0.515136715,
0.515258785,
0.515258785,
0.51586914,
0.518066405,
0.518066405,
0.519042965,
0.519042965,
0.51745605,
0.5135498,
0.5135498,
0.511596675,
0.511596675,
0.515380855,
0.515380855,
0.51989746,
0.518920895,
0.518920895,
0.52026367,
0.52026367,
0.522216795,
0.515380855,
0.515380855,
0.52099609,
0.52099609,
0.518432615,
0.518432615,
0.515258785,
0.518066405,
0.518066405,
0.51977539,
0.51977539,
0.519165035,
0.519409175,
0.519409175,
0.522949215,
0.522949215,
0.518798825,
0.518798825,
0.51708984,
0.519042965,
0.519042965,
0.518676755,
0.518676755,
0.51977539,
0.52111816,
0.52111816,
0.514160155,
0.514160155,
0.513793945,
0.513793945,
0.515502925,
0.513793945,
0.513793945,
0.51708984,
0.51708984,
0.518798825,
0.519042965,
0.519042965,
0.52368164,
0.52368164,
0.51586914,
0.51586914,
0.518920895,
0.51647949,
0.51647949,
0.518920895,
0.518920895,
0.51586914,
0.513671875,
0.513671875,
0.517822265,
0.517822265,
0.518798825,
0.518798825,
0.514770505,
0.51257324,
0.51257324,
0.515014645,
0.515014645,
0.514404295,
0.5168457,
0.5168457,
0.51635742,
0.51635742,
0.51574707,
0.51574707,
0.518798825,
0.51989746,
0.51989746,
0.51660156,
0.51660156,
0.518676755,
0.518432615,
0.518432615,
0.518310545,
0.518310545,
0.51745605,
0.51745605,
0.518188475,
0.510620115,
0.510620115,
0.517700195,
0.517700195,
0.518188475,
0.517578125,
0.517578125,
0.51696777,
0.51696777,
0.51733398,
0.51733398,
0.51745605,
0.517700195,
0.517700195,
0.513793945,
0.513793945,
0.510864255,
0.514526365,
0.514526365,
0.51977539,
0.51977539,
0.513916015,
0.513916015,
0.517944335,
0.517578125,
0.517578125,
0.518432615,
0.518432615,
0.514526365,
0.51220703,
0.51220703,
0.51745605,
0.51745605,
0.515380855,
0.515380855,
0.515136715,
0.51318359,
0.51318359,
0.51708984,
0.51708984,
0.51733398,
0.514648435,
0.514648435,
0.51574707,
0.51574707,
0.517700195,
0.517700195,
0.51623535,
0.513793945,
0.513793945,
0.515136715,
0.515136715,
0.51293945,
0.51611328,
0.51611328,
0.5168457,
0.5168457,
0.51623535,
0.51623535,
0.514404295,
0.513916015,
0.513916015,
0.5213623,
0.5213623,
0.52050781,
0.51672363,
0.51672363,
0.517822265,
0.517822265,
0.51647949,
0.51647949,
0.51635742,
0.51696777,
0.51696777,
0.517944335,
0.517944335,
0.518798825,
0.51574707,
0.51574707,
0.51696777,
0.51696777,
0.51672363,
0.51672363,
0.515258785,
0.51721191,
0.51721191,
0.5168457,
0.5168457,
0.51977539,
0.51696777,
0.51696777,
0.515258785,
0.515258785,
0.519287105,
0.519287105,
0.51586914,
0.51293945,
0.51293945,
0.51635742,
0.51635742,
0.51733398,
0.51721191,
0.51721191,
0.513671875,
0.513671875,
0.510009765,
0.510009765,
0.51611328,
0.514770505,
0.514770505,
0.51245117,
0.51245117,
0.514526365,
0.51672363,
0.51672363,
0.517700195,
0.517700195,
0.514160155,
0.514160155,
0.515502925,
0.514648435,
0.514648435,
0.51269531,
0.51269531,
0.510498045,
0.51293945,
0.51293945,
0.51623535,
0.51623535,
0.514282225,
0.514282225,
0.514892575,
0.515014645,
0.515014645,
0.51599121,
0.51599121,
0.5168457,
0.518798825,
0.518798825,
0.515380855,
0.515380855,
0.515258785,
0.515258785,
0.515502925,
0.517578125,
0.517578125,
0.51721191,
0.51721191,
0.51623535,
0.518310545,
0.518310545,
0.517944335,
0.517944335,
0.51647949,
0.515136715,
0.515136715,
0.514648435,
0.514648435,
0.515136715,
0.515136715,
0.51269531,
0.514282225,
0.514282225,
0.51635742,
0.51635742,
0.51611328,
0.515625,
0.515625,
0.515258785,
0.515258785,
0.51708984,
0.51708984,
0.518188475,
0.51733398,
0.51733398,
0.51293945,
0.51293945,
0.510498045,
0.513916015,
0.513916015,
0.518066405,
0.518066405,
0.51672363,
0.51672363,
0.51647949,
0.51696777,
0.51696777,
0.51647949,
0.51647949,
0.51708984,
0.514038085,
0.514038085,
0.513793945,
0.513793945,
0.51171875,
0.51171875,
0.517822265,
0.51672363,
0.51672363,
0.5168457,
0.5168457,
0.51745605,
0.51733398,
0.51733398,
0.51660156,
0.51660156,
0.5168457,
0.5168457,
0.522094725,
0.513793945,
0.513793945,
0.514038085,
0.514038085,
0.51623535,
0.515014645,
0.515014645,
0.51257324,
0.51257324,
0.51342773,
0.51342773,
0.515258785,
0.515258785,
0.515258785,
0.515136715,
0.515136715,
0.51586914,
0.51745605,
0.51745605,
0.515258785,
0.515258785,
0.513916015,
0.513916015,
0.51733398,
0.518066405,
0.518066405,
0.517944335,
0.517944335,
0.518798825,
0.517578125,
0.517578125,
0.51586914,
0.51586914,
0.519042965,
0.519042965,
0.51708984,
0.515502925,
0.515502925,
0.514770505,
0.514770505,
0.515136715,
0.519042965,
0.519042965,
0.517578125,
0.517578125,
0.519287105,
0.519287105,
0.51599121,
0.518310545,
0.518310545,
0.517822265,
0.517822265,
0.514526365,
0.514038085,
0.514038085,
0.517944335,
0.517944335,
0.518310545,
0.518310545,
0.51672363,
0.518432615,
0.518432615,
0.51611328,
0.51611328,
0.513671875,
0.514160155,
0.514160155,
0.518066405,
0.518066405,
0.51574707,
0.51574707,
0.51672363,
0.518920895,
0.518920895,
0.518554685,
0.518554685,
0.518310545,
0.511474605,
0.511474605,
0.514526365,
0.514526365,
0.51574707,
0.51574707,
0.511230465,
0.514160155,
0.514160155,
0.51599121,
0.51599121,
0.515380855,
0.514404295,
0.514404295,
0.518310545,
0.518310545,
0.518188475,
0.518188475,
0.522094725,
0.52099609,
0.52099609,
0.51660156,
0.51660156,
0.514282225,
0.515136715,
0.515136715,
0.51171875,
0.51171875,
0.51196289,
0.51196289,
0.514038085,
0.518310545,
0.518310545,
0.511596675,
0.511596675,
0.5123291,
0.51318359,
0.51318359,
0.51647949,
0.51647949,
0.51965332,
0.51965332,
0.519409175,
0.514648435,
0.514648435,
0.515258785,
0.515258785,
0.514526365,
0.517944335,
0.517944335,
0.514526365,
0.514526365,
0.51635742,
0.51635742,
0.51623535,
0.517700195,
0.517700195,
0.51745605,
0.51745605,
0.52001953,
0.518188475,
0.518188475,
0.517700195,
0.517700195,
0.51989746,
0.51989746,
0.51611328,
0.51611328,
0.51611328,
0.51660156,
0.51660156,
0.515625,
0.51647949,
0.51647949,
0.517944335,
0.517944335,
0.517822265,
0.517822265,
0.51733398,
0.51672363,
0.51672363,
0.514648435,
0.514648435,
0.514892575,
0.52062988,
0.52062988,
0.51647949,
0.51647949,
0.5168457,
0.5168457,
0.51696777,
0.517822265,
0.517822265,
0.521484375,
0.521484375,
0.515625,
0.51611328,
0.51611328,
0.514404295,
0.514404295,
0.511474605,
0.511474605,
0.517700195,
0.513793945,
0.513793945,
0.51306152,
0.51306152,
0.51708984,
0.51721191,
0.51721191,
0.515258785,
0.515258785,
0.51733398,
0.51733398,
0.518920895,
0.514770505,
0.514770505,
0.515380855,
0.515380855,
0.51245117,
0.514770505,
0.514770505,
0.52050781,
0.52050781,
0.51696777,
0.51696777,
0.51574707,
0.514770505,
0.514770505,
0.51672363,
0.51672363,
0.518432615,
0.51306152,
0.51306152,
0.51269531,
0.51269531,
0.514038085,
0.514038085,
0.51721191,
0.518676755,
0.518676755,
0.518676755,
0.518676755,
0.518554685,
0.5135498,
0.5135498,
0.51623535,
0.51623535,
0.51623535,
0.51623535,
0.51660156,
0.51623535,
0.51623535,
0.514770505,
0.514770505,
0.514038085,
0.518676755,
0.518676755,
0.517578125,
0.517578125,
0.515014645,
0.515014645,
0.515380855,
0.51745605,
0.51745605,
0.518554685,
0.518554685,
0.514160155,
0.514770505,
0.514770505,
0.5135498,
0.5135498,
0.519287105,
0.519287105,
0.51965332,
0.517944335,
0.517944335,
0.51586914,
0.51586914,
0.51696777,
0.515502925,
0.515502925,
0.52099609,
0.52099609,
0.515380855,
0.515380855,
0.514892575,
0.5135498,
0.5135498,
0.514038085,
0.514038085,
0.518554685,
0.51672363,
0.51672363,
0.51977539,
0.51977539,
0.51953125,
0.51953125,
0.5201416,
0.51672363,
0.51672363,
0.517700195,
0.517700195,
0.518310545,
0.52026367,
0.52026367,
0.519165035,
0.519165035,
0.51611328,
0.517578125,
0.517578125,
0.51745605,
0.51745605,
0.510253905,
0.510253905,
0.514404295,
0.514770505,
0.514770505,
0.51733398,
0.51733398,
0.517944335,
0.517944335,
0.517700195,
0.51599121,
0.51599121,
0.517944335,
0.517944335,
0.514892575,
0.518432615,
0.518432615,
0.513916015,
0.513916015,
0.513916015,
0.519409175,
0.519409175,
0.518066405,
0.518066405,
0.513793945,
0.513793945,
0.510620115,
0.51708984,
0.51708984,
0.52026367,
0.52026367,
0.51623535,
0.51696777,
0.51696777,
0.519287105,
0.519287105,
0.51989746,
0.51989746,
0.519165035,
0.515625,
0.515625,
0.51306152,
0.51306152,
0.511108395,
0.515258785,
0.515258785,
0.51599121,
0.51599121,
0.514770505,
0.514770505,
0.519042965,
0.51672363,
0.51672363,
0.514526365,
0.514526365,
0.51672363,
0.514038085,
0.514038085,
0.513916015,
0.513916015,
0.51660156,
0.51660156,
0.518554685,
0.518432615,
0.518432615,
0.5123291,
0.5123291,
0.51257324,
0.514648435,
0.514648435,
0.51635742,
0.51635742,
0.51623535,
0.51623535,
0.514892575,
0.517700195,
0.517700195,
0.518676755,
0.518676755,
0.52087402,
0.514892575,
0.514892575,
0.517578125,
0.517578125,
0.51989746,
0.51989746,
0.517700195,
0.513916015,
0.513916015,
0.50878906,
0.50878906,
0.514404295,
0.515380855,
0.515380855,
0.510620115,
0.510620115,
0.514770505,
0.514770505,
0.515625,
0.521850585,
0.521850585,
0.517578125,
0.517578125,
0.513793945,
0.514160155,
0.514160155,
0.515625,
0.515625,
0.518676755,
0.518676755,
0.513671875,
0.514892575,
0.514892575,
0.51647949,
0.51647949,
0.514404295,
0.51635742,
0.51635742,
0.518310545,
0.518310545,
0.51660156,
0.51660156,
0.514770505,
0.514160155,
0.514160155,
0.515258785,
0.515258785,
0.517822265,
0.518188475,
0.518188475,
0.51635742,
0.51635742,
0.515014645,
0.515014645,
0.515502925,
0.511474605,
0.511474605,
0.51269531,
0.51269531,
0.51586914,
0.51745605,
0.51745605,
0.51611328,
0.51611328,
0.515258785,
0.515258785,
0.514160155,
0.510742185,
0.510742185,
0.51574707,
0.51574707,
0.51721191,
0.517944335,
0.517944335,
0.515258785,
0.515258785,
0.51293945,
0.51293945,
0.51745605,
0.51733398,
0.51733398,
0.51330566,
0.51330566,
0.51245117,
0.514648435,
0.514648435,
0.51733398,
0.51733398,
0.517944335,
0.517944335,
0.51647949,
0.51586914,
0.51586914,
0.51623535,
0.51623535,
0.517822265,
0.51696777,
0.51696777,
0.515136715,
0.515136715,
0.514648435,
0.514648435,
0.51293945,
0.50964355,
0.50964355,
0.514892575,
0.514892575,
0.519042965,
0.52062988,
0.52062988,
0.518920895,
0.518920895,
0.517700195,
0.517700195,
0.51574707,
0.51611328,
0.51611328,
0.515258785,
0.515258785,
0.514282225,
0.5168457,
0.5168457,
0.515502925,
0.515502925,
0.51245117,
0.51245117,
0.51306152,
0.513793945,
0.513793945,
0.518676755,
0.518676755,
0.51721191,
0.51269531,
0.51269531,
0.51660156,
0.51660156,
0.51647949,
0.51647949,
0.51623535,
0.51599121,
0.51599121,
0.51611328,
0.51611328,
0.51708984,
0.514404295,
0.514404295,
0.514770505,
0.514770505,
0.513793945,
0.513793945,
0.51306152,
0.514038085,
0.514038085,
0.51708984,
0.51708984,
0.514892575,
0.51574707,
0.51574707,
0.52124023,
0.52124023,
0.52038574,
0.52038574,
0.513793945,
0.515380855,
0.515380855,
0.51708984,
0.51708984,
0.51586914,
0.517578125,
0.517578125,
0.51220703,
0.51220703,
0.5123291,
0.5123291,
0.51708984,
0.514648435,
0.514648435,
0.514404295,
0.514404295,
0.515625,
0.51708984,
0.51708984,
0.514892575,
0.514892575,
0.51611328,
0.51611328,
0.51696777,
0.515014645,
0.515014645,
0.515380855,
0.515380855,
0.514526365,
0.52026367,
0.52026367,
0.521850585,
0.521850585,
0.519165035,
0.519165035,
0.51989746,
0.522460935,
0.522460935,
0.51635742,
0.51635742,
0.51660156,
0.52001953,
0.52001953,
0.514526365,
0.514526365,
0.514038085,
0.514038085,
0.51647949,
0.51599121,
0.51599121,
0.51708984,
0.51708984,
0.518554685,
0.51733398,
0.51733398,
0.52026367,
0.52026367,
0.515014645,
0.515014645,
0.51196289,
0.51220703,
0.51220703,
0.514404295,
0.514404295,
0.51586914,
0.517578125,
0.517578125,
0.51672363,
0.51672363,
0.513916015,
0.51672363,
0.51672363,
0.517578125,
0.517578125,
0.51586914,
0.51586914,
0.51708984,
0.51708984,
0.51708984,
0.515014645,
0.515014645,
0.51574707,
0.51293945,
0.51293945,
0.51574707,
0.51574707,
0.51965332,
0.51965332,
0.518310545,
0.5135498,
0.5135498,
0.51672363,
0.51672363,
0.517944335,
0.5135498,
0.5135498,
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
    /*
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
        input_data[0][i][0][0] = 0.5;
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