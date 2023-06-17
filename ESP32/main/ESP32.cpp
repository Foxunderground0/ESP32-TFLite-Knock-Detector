#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp32/rom/gpio.h"
#include "esp_timer.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "model_data.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_22    // GPIO number for I2C master clock
#define I2C_MASTER_SDA_IO GPIO_NUM_21    // GPIO number for I2C master data
#define I2C_MASTER_NUM I2C_NUM_0 // I2C port number for master dev
#define MPU6050_ADDR 0x68       // I2C address of the MPU-6050
#define ONBOARD_LED  GPIO_NUM_0

bool led_state = false;
bool sample_ready = false;

static const char* TAG = "main_task";

#define SAMPLES_COUNT 2000
float buffer[SAMPLES_COUNT]; //Circular Buffer
uint16_t buffer_head = 0;

#define INTERRUPT_FREQ 1700
#define TIMER_DIVIDER (TIMER_BASE_CLK / INTERRUPT_FREQ)

#define TIMER_GROUP TIMER_GROUP_0
#define TIMER_NUM TIMER_0

// Define the model data, tensor arena, and their respective sizes
extern const unsigned char model_data[];
extern const int model_data_len;
constexpr int kTensorArenaSize = 200000; // Define the size of the tensor arena buffer

// Create a buffer for the interpreter tensor arena
uint8_t IRAM_ATTR tensor_arena[kTensorArenaSize];

void i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 500000; // Set I2C master clock frequency
    conf.clk_flags = 0;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
};

uint8_t* MPU_read(uint8_t reg, uint8_t number_of_bytes) {
    uint8_t* data = (uint8_t*)malloc(number_of_bytes);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true); // Who Am I reg address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    for (int i = 0; i < number_of_bytes; i++) {
        i2c_master_read_byte(cmd, &data[i], I2C_MASTER_ACK);
    }
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS); // Filling in the missing part
    i2c_cmd_link_delete(cmd);
    return data;
};

void MPU_write(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
};

// Interrupt service routine (ISR)
bool IRAM_ATTR timer_isr(gptimer_t* gptimer, const gptimer_alarm_event_data_t* event_data, void* arg) {
    sample_ready = true;
    gpio_set_level(ONBOARD_LED, led_state);
    led_state = !led_state;
    return true;
}

extern "C" void app_main() {
    i2c_master_init();

    MPU_write(0x6b, 0x00);

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

    // Set up the model
    const tflite::Model* model = tflite::GetModel(model_quantized_tflite);
    tflite::MicroErrorReporter micro_error_reporter;
    constexpr int kOpResolverSize = 6; // Define the number of supported operations
    tflite::MicroMutableOpResolver<kOpResolverSize> micro_op_resolver;

    micro_op_resolver.AddAveragePool2D();
    micro_op_resolver.AddConv2D();
    micro_op_resolver.AddFullyConnected();
    micro_op_resolver.AddRelu();
    micro_op_resolver.AddReshape(); //  https://stackoverflow.com/questions/62580548/is-the-keras-function-flatten-supported-by-tensorflow-lite
    micro_op_resolver.AddSoftmax();

    //micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());

    tflite::MicroInterpreter interpreter(model, micro_error_reporter, tensor_arena, kTensorArenaSize);

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
};