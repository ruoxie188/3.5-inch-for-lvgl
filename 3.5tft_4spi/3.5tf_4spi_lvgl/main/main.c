#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_hpy.h"

// 引入 LVGL Port 和 LVGL 核心库
#include "esp_lvgl_port.h"
#include "lvgl.h"

static const char *TAG = "MAIN_APP";

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
        .rotation = {
            .swap_xy = true,
            .mirror_x = false,
            .mirror_y = true,
        },
        // 【最终修正】移除所有字节交换标志，因为问题已由 Menuconfig 解决
        .flags = {
            .buff_dma = true,
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
    lvgl_port_lock(0);
    create_example_ui();
    lvgl_port_unlock();

    ESP_LOGI(TAG, "LVGL setup done. UI is now running.");
}

static void create_example_ui(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 200, 80);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Hello, SPI LVGL!");
    lv_obj_center(label);
    // 确保字体可用，如果编译不过，请在menuconfig中启用LV_FONT_MONTSERRAT_22
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
}

