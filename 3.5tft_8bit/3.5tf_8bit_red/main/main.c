#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include <assert.h>

// 引入 ST7796 驱动头文件和 heap_caps 相关头文件
#include "esp_lcd_st7796.h"
#include "esp_heap_caps.h"

static const char *TAG = "ST7796_SUCCESS_EXAMPLE";

// 您的引脚定义
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_PIN_NUM_RD             41
#define EXAMPLE_PIN_NUM_DATA0          15
#define EXAMPLE_PIN_NUM_DATA1          16
#define EXAMPLE_PIN_NUM_DATA2          17
#define EXAMPLE_PIN_NUM_DATA3          18
#define EXAMPLE_PIN_NUM_DATA4          21
#define EXAMPLE_PIN_NUM_DATA5          40
#define EXAMPLE_PIN_NUM_DATA6          39
#define EXAMPLE_PIN_NUM_DATA7          38
#define EXAMPLE_PIN_NUM_PCLK           42
#define EXAMPLE_PIN_NUM_CS             46
#define EXAMPLE_PIN_NUM_DC             45
#define EXAMPLE_PIN_NUM_RST            -1
#define EXAMPLE_PIN_NUM_BK_LIGHT       47

// 屏幕分辨率
#define EXAMPLE_LCD_H_RES              480
#define EXAMPLE_LCD_V_RES              320

// ===================== 关键配置：从成功案例中提取 =====================
// PSRAM 数据对齐
#define EXAMPLE_PSRAM_DATA_ALIGNMENT   64
// 分块传输以保证稳定性
#define EXAMPLE_LCD_DRAW_BUF_HEIGHT    (50)
// =================================================================


void app_main(void)
{
    // 初始化背光和 RD 引脚
    gpio_config_t bk_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_config_t rd_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_RD};
    ESP_ERROR_CHECK(gpio_config(&rd_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(EXAMPLE_PIN_NUM_RD, 1));
    
    ESP_LOGI(TAG, "配置 i80 总线");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = EXAMPLE_PIN_NUM_DC,
        .wr_gpio_num = EXAMPLE_PIN_NUM_PCLK,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0, EXAMPLE_PIN_NUM_DATA1, EXAMPLE_PIN_NUM_DATA2, EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4, EXAMPLE_PIN_NUM_DATA5, EXAMPLE_PIN_NUM_DATA6, EXAMPLE_PIN_NUM_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t),
        // ===================== 关键配置：从成功案例中提取 =====================
        .psram_trans_align = EXAMPLE_PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
        // =================================================================
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    ESP_LOGI(TAG, "配置 i80 面板 IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = EXAMPLE_PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {.dc_idle_level = 0, .dc_cmd_level = 0, .dc_dummy_level = 0, .dc_data_level = 1},
        .flags.swap_color_bytes = true, 
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "安装 ST7796 LCD 驱动");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    // 设置方向和镜像
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI(TAG, "开启背光");
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);

    ESP_LOGI(TAG, "开始绘制全屏红色...");
    // ===================== 关键配置：从成功案例中提取 =====================
    // 使用 heap_caps_aligned_alloc 来分配对齐的内存
    uint16_t *buffer = heap_caps_aligned_alloc(EXAMPLE_PSRAM_DATA_ALIGNMENT, 
                                               EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t), 
                                               MALLOC_CAP_SPIRAM);
    // =================================================================
    assert(buffer);

    // 填充缓冲区为红色
    for (int i = 0; i < EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUF_HEIGHT; i++) {
        buffer[i] =  0xF800; // Red
    }

    // 分块绘制全屏
    for (int y = 0; y < EXAMPLE_LCD_V_RES; y += EXAMPLE_LCD_DRAW_BUF_HEIGHT) {
        int h = (y + EXAMPLE_LCD_DRAW_BUF_HEIGHT) > EXAMPLE_LCD_V_RES ? (EXAMPLE_LCD_V_RES - y) : EXAMPLE_LCD_DRAW_BUF_HEIGHT;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, y, EXAMPLE_LCD_H_RES, y + h, buffer));
    }
    
    // 使用 free() 来释放 heap_caps_aligned_alloc 分配的内存
    free(buffer);
    ESP_LOGI(TAG, "绘制完成！");

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

