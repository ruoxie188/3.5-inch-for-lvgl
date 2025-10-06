#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_hpy.h" // 引入我们自己的封装库

// 引入 LVGL Port 和 LVGL 核心库
#include "esp_lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "MAIN_APP";

// 声明一个函数用于创建我们的UI
static void create_example_ui(void);

void app_main(void)
{
    // --- 1. 初始化硬件 ---
    ESP_LOGI(TAG, "Initializing BSP...");
    ESP_ERROR_CHECK(bsp_display_init());
    ESP_ERROR_CHECK(bsp_touch_init());

    // --- 2. 初始化 LVGL Port ---
    ESP_LOGI(TAG, "Initializing LVGL port...");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
    
    // --- 3. 将显示设备添加到 LVGL Port ---
    ESP_LOGI(TAG, "Adding display to LVGL port...");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = g_io_handle,
        .panel_handle = g_lcd_panel_handle,
        .buffer_size = LCD_H_RES * LCD_DRAW_BUF_HEIGHT,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        // 在这里配置旋转和镜像，以匹配您的屏幕物理方向和触摸
        .rotation = {
            .swap_xy = true,
            .mirror_x = true,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true, // 启用DMA对齐的缓冲区
        }
    };
    lv_disp_t * disp = lvgl_port_add_disp(&disp_cfg);

    // --- 4. 将触摸设备添加到 LVGL Port ---
    ESP_LOGI(TAG, "Adding touch to LVGL port...");
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = g_tp_handle,
    };
    lvgl_port_add_touch(&touch_cfg);

    // --- 5. 创建 LVGL UI ---
    ESP_LOGI(TAG, "Creating LVGL UI...");
    // 建议使用 lvgl_port_lock/unlock 来确保线程安全
    lvgl_port_lock(0);
    create_example_ui();
    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL setup done. UI is now running.");
    // app_main 可以结束了, LVGL 会在后台任务中持续运行
}

// 一个简单的函数来创建 LVGL UI 界面
static void create_example_ui(void)
{
    lv_obj_t *scr = lv_scr_act();

    // 创建一个按钮
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 200, 80);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0); // 居中对齐

    // 在按钮上创建一个标签
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Hello, 16-bit LVGL!");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
}

