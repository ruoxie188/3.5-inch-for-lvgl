#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_hpy.h" // 引入我们自己的封装库

// 引入 LVGL Port 和 LVGL 核心库
#include "esp_lvgl_port.h"
#include "lvgl.h"

// 【新增】引入 LVGL Demos 的头文件
#include "demos/lv_demos.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    // --- 1. 初始化硬件 (保持不变) ---
    ESP_LOGI(TAG, "Initializing BSP...");
    ESP_ERROR_CHECK(bsp_display_init());
    ESP_ERROR_CHECK(bsp_touch_init());

    // --- 2. 初始化 LVGL Port (保持不变) ---
    ESP_LOGI(TAG, "Initializing LVGL port...");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
    
    // --- 3. 将显示设备添加到 LVGL Port (保持不变) ---
    ESP_LOGI(TAG, "Adding display to LVGL port...");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = g_io_handle,
        .panel_handle = g_lcd_panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUF_HEIGHT,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = true,
            .mirror_x = true,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
        }
    };
    lv_disp_t * disp = lvgl_port_add_disp(&disp_cfg);

    // --- 4. 将触摸设备添加到 LVGL Port (保持不变) ---
    ESP_LOGI(TAG, "Adding touch to LVGL port...");
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = g_tp_handle,
    };
    lvgl_port_add_touch(&touch_cfg);

    // --- 5. 创建 LVGL UI (这是唯一需要修改的部分) ---
    ESP_LOGI(TAG, "Creating LVGL Demo UI...");
    
    // 使用 lvgl_port_lock/unlock 来确保线程安全
    lvgl_port_lock(0);

    // 【修改点】调用官方的 Widgets Demo 初始化函数
    // 您可以删除之前自己写的 create_example_ui() 函数了
    lv_demo_benchmark();

    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL setup done. Demo is now running.");
}

