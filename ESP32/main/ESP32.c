#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp32/rom/gpio.h"
#include "driver/i2c.h"

#define ONBOARD_LED  GPIO_NUM_21

void app_main(void) {
    char* rasl = pcTaskGetName(NULL);
    ESP_LOGI(rasl, "Hello, Starting UP!");

    gpio_pad_select_gpio(ONBOARD_LED);
    gpio_set_direction(ONBOARD_LED, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(ONBOARD_LED, 1);
        vTaskDelay(2);
        gpio_set_level(ONBOARD_LED, 0);
        vTaskDelay(2);
    }
}
