#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_hpy.h" // 引入我们自己的封装库

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    // --- 1. 初始化硬件 ---
    ESP_LOGI(TAG, "Initializing BSP...");
    ESP_ERROR_CHECK(bsp_display_init());
    ESP_ERROR_CHECK(bsp_touch_init());

    // --- 2. 初始显示 ---
    ESP_LOGI(TAG, "Drawing initial screen (BLUE)");
    bsp_fill_screen(0x001F); // 初始显示为蓝色

    // --- 3. 主循环：检测触摸并改变颜色 ---
    ESP_LOGI(TAG, "Entering main loop, please touch the screen...");
    while(1) {
        esp_lcd_touch_read_data(g_tp_handle);
        
        uint16_t touch_x[1];
        uint16_t touch_y[1];
        uint16_t touch_strength[1];
        uint8_t touch_cnt = 0;

        bool touched = esp_lcd_touch_get_coordinates(g_tp_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
        
        if (touched && touch_cnt > 0) {
            ESP_LOGI(TAG, "TOUCHED! Coordinates: x=%d, y=%d", touch_x[0], touch_y[0]);
            
            uint16_t random_color = rand() & 0xFFFF;
            
            bsp_fill_screen(random_color);

            vTaskDelay(pdMS_TO_TICKS(200)); 
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

