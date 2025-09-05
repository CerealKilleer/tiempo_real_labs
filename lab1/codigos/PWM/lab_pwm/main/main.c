#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ledc_timer_config_t lec_timer_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 100,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_channel_config_t ledc_channel_cfg = {
        .duty = 4096,
        .gpio_num = 21,
        .sleep_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .channel = LEDC_CHANNEL_0,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&lec_timer_cfg));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_cfg));

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}