#include <stdio.h>
#include <stdlib.h> // 用于 rand() 函数
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h" // I2C 驱动头文件
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h" // 触摸通用头文件
#include "esp_lcd_touch_ft5x06.h" // FT5x06 驱动头文件
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include <assert.h>
#include "esp_heap_caps.h"

// 引入 ST7796 驱动头文件
#include "esp_lcd_st7796.h"

static const char *TAG = "TOUCH_EXAMPLE";

// ====================================================================
// 您的引脚定义 (Your Pin Definitions)
// ====================================================================
// 显示部分
#define LCD_PIXEL_CLOCK_HZ     (10 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  1
#define PIN_NUM_RD             41
#define PIN_NUM_DATA0          15
#define PIN_NUM_DATA1          16
#define PIN_NUM_DATA2          17
#define PIN_NUM_DATA3          18
#define PIN_NUM_DATA4          21
#define PIN_NUM_DATA5          40
#define PIN_NUM_DATA6          39
#define PIN_NUM_DATA7          38
#define PIN_NUM_PCLK           42
#define PIN_NUM_CS             46
#define PIN_NUM_DC             45
#define PIN_NUM_RST            -1
#define PIN_NUM_BK_LIGHT       47
#define LCD_H_RES              480
#define LCD_V_RES              320

// 触摸部分 (根据您的 J4 原理图)
#define TOUCH_I2C_NUM          (I2C_NUM_0)
#define TOUCH_I2C_SCL          (GPIO_NUM_2)
#define TOUCH_I2C_SDA          (GPIO_NUM_1)
#define TOUCH_INT              (GPIO_NUM_3)
#define TOUCH_RST              (-1) // 与系统复位绑定

// 其他配置
#define PSRAM_DATA_ALIGNMENT   64
#define LCD_DRAW_BUF_HEIGHT    (50)

// 全局变量，方便在循环中访问
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_touch_handle_t tp_handle = NULL;

// 一个简单的函数，用于用指定颜色填充整个屏幕
void fill_screen(uint16_t color)
{
    uint16_t *buffer = heap_caps_aligned_alloc(PSRAM_DATA_ALIGNMENT, 
                                               LCD_H_RES * LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t), 
                                               MALLOC_CAP_SPIRAM);
    assert(buffer);

    for (int i = 0; i < LCD_H_RES * LCD_DRAW_BUF_HEIGHT; i++) {
        buffer[i] = color;
    }

    for (int y = 0; y < LCD_V_RES; y += LCD_DRAW_BUF_HEIGHT) {
        int h = (y + LCD_DRAW_BUF_HEIGHT) > LCD_V_RES ? (LCD_V_RES - y) : LCD_DRAW_BUF_HEIGHT;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_H_RES, y + h, buffer));
    }
    
    free(buffer);
}


void app_main(void)
{
    // --- 1. 显示部分初始化 (与之前成功的代码一致) ---
    gpio_config_t bk_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_config_t rd_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << PIN_NUM_RD};
    ESP_ERROR_CHECK(gpio_config(&rd_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_RD, 1));
    
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT, .dc_gpio_num = PIN_NUM_DC, .wr_gpio_num = PIN_NUM_PCLK,
        .data_gpio_nums = { PIN_NUM_DATA0, PIN_NUM_DATA1, PIN_NUM_DATA2, PIN_NUM_DATA3, PIN_NUM_DATA4, PIN_NUM_DATA5, PIN_NUM_DATA6, PIN_NUM_DATA7, },
        .bus_width = 8, .max_transfer_bytes = LCD_H_RES * LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t),
        .psram_trans_align = PSRAM_DATA_ALIGNMENT, .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS, .pclk_hz = LCD_PIXEL_CLOCK_HZ, .trans_queue_depth = 10,
        .dc_levels = {.dc_idle_level = 0, .dc_cmd_level = 0, .dc_dummy_level = 0, .dc_data_level = 1},
        .flags.swap_color_bytes = true, .lcd_cmd_bits = 8, .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST, .color_space = ESP_LCD_COLOR_SPACE_BGR, .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    ESP_LOGI(TAG, "开启背光");
    gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);
    fill_screen(0x001F); // 初始显示为蓝色
    
    // --- 2. 触摸部分初始化 ---
    ESP_LOGI(TAG, "初始化 I2C 总线");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400 * 1000, // 400KHz
    };
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_I2C_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0));

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle));
    
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = TOUCH_RST,
        .int_gpio_num = TOUCH_INT,
        .levels = { .reset = 0, .interrupt = 0, },
        .flags = { .swap_xy = true, .mirror_x = true, .mirror_y = true, }, // 与显示保持一致
    };
    ESP_LOGI(TAG, "安装 FT5x06 触摸驱动");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp_handle));

    // --- 3. 主循环：检测触摸并改变颜色 ---
    ESP_LOGI(TAG, "进入主循环，请触摸屏幕...");
    while(1) {
        // 读取触摸数据
        esp_lcd_touch_read_data(tp_handle);
        
        uint16_t touch_x[1];
        uint16_t touch_y[1];
        uint16_t touch_strength[1];
        uint8_t touch_cnt = 0;

        // 获取触摸点信息
        bool touched = esp_lcd_touch_get_coordinates(tp_handle, touch_x, touch_y, touch_strength, &touch_cnt, 1);
        
        if (touched && touch_cnt > 0) {
            ESP_LOGI(TAG, "触摸! 坐标: x=%d, y=%d", touch_x[0], touch_y[0]);
            
            // 生成一个随机的 16 位颜色
            uint16_t random_color = rand() & 0xFFFF;
            
            // 用随机颜色填充屏幕
            fill_screen(random_color);

            // 短暂延时以避免颜色变化过快
            vTaskDelay(pdMS_TO_TICKS(200)); 
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // 每次循环延时20ms
    }
}

