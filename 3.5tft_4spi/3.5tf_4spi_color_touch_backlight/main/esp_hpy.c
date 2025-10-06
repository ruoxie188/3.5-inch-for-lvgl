#include "esp_hpy.h"

// Implementation-specific headers
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lcd_st7796.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <assert.h>
#include <byteswap.h>

#ifndef __bswap_16
#define __bswap_16(x) __builtin_bswap16(x)
#endif

static const char *TAG = "ESP_HPY_BSP";

// -- Global Handles --
esp_lcd_panel_handle_t g_lcd_panel_handle = NULL;
esp_lcd_touch_handle_t g_tp_handle = NULL;

esp_err_t bsp_display_init(void)
{
    ESP_LOGI(TAG, "Initializing display...");

    // 1. 初始化背光PWM
    ESP_LOGI(TAG, "Configuring backlight PWM...");
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LCD_BK_LIGHT_LEDC_MODE,
        .timer_num        = LCD_BK_LIGHT_LEDC_TIMER,
        .duty_resolution  = LCD_BK_LIGHT_LEDC_DUTY_RES,
        .freq_hz          = LCD_BK_LIGHT_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LCD_BK_LIGHT_LEDC_MODE,
        .channel        = LCD_BK_LIGHT_LEDC_CHANNEL,
        .timer_sel      = LCD_BK_LIGHT_LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_NUM_BK_LIGHT,
        .duty           = 0, // 初始化时先关闭
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // 2. 初始化SPI总线
    ESP_LOGI(TAG, "Initializing SPI bus...");
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_SCLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 3. 配置LCD的SPI IO句柄
    ESP_LOGI(TAG, "Installing panel IO...");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS,
        .dc_gpio_num = PIN_NUM_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    // 4. 安装ST7796驱动
    ESP_LOGI(TAG, "Installing ST7796 driver...");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &g_lcd_panel_handle));

    // 5. 初始化LCD面板
    ESP_ERROR_CHECK(esp_lcd_panel_reset(g_lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(g_lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(g_lcd_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(g_lcd_panel_handle, false, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(g_lcd_panel_handle, true));

    // 6. 使用PWM打开背光到100%亮度
    ESP_LOGI(TAG, "Turning on backlight to 100%%");
    bsp_display_set_backlight(100);

    return ESP_OK;
}

void bsp_display_set_backlight(uint32_t brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    
    uint32_t max_duty = (1 << LCD_BK_LIGHT_LEDC_DUTY_RES) - 1;
    uint32_t duty_cycle = (max_duty * brightness_percent) / 100;

    ESP_ERROR_CHECK(ledc_set_duty(LCD_BK_LIGHT_LEDC_MODE, LCD_BK_LIGHT_LEDC_CHANNEL, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(LCD_BK_LIGHT_LEDC_MODE, LCD_BK_LIGHT_LEDC_CHANNEL));
}

esp_err_t bsp_touch_init(void)
{
    ESP_LOGI(TAG, "Initializing touch panel...");

    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400 * 1000,
    };
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_I2C_NUM, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_I2C_NUM, i2c_conf.mode, 0, 0, 0));

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES, .y_max = LCD_V_RES,
        .rst_gpio_num = TOUCH_RST, .int_gpio_num = TOUCH_INT,
        .levels = { .reset = 0, .interrupt = 0, },
        .flags = { .swap_xy = true, .mirror_x = true, .mirror_y = true, },
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &g_tp_handle));

    return ESP_OK;
}

void bsp_fill_screen(uint16_t color)
{
    if (g_lcd_panel_handle == NULL) {
        ESP_LOGE(TAG, "Display not initialized!");
        return;
    }

    uint16_t *buffer = heap_caps_aligned_alloc(PSRAM_DATA_ALIGNMENT,
                                               LCD_H_RES * LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t),
                                               MALLOC_CAP_SPIRAM);
    assert(buffer);

    // 手动进行颜色字节序交换，以适配您的IDF版本
    uint16_t swapped_color = __bswap_16(color);

    for (int i = 0; i < LCD_H_RES * LCD_DRAW_BUF_HEIGHT; i++) {
        buffer[i] = swapped_color;
    }

    for (int y = 0; y < LCD_V_RES; y += LCD_DRAW_BUF_HEIGHT) {
        int h = (y + LCD_DRAW_BUF_HEIGHT) > LCD_V_RES ? (LCD_V_RES - y) : LCD_DRAW_BUF_HEIGHT;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(g_lcd_panel_handle, 0, y, LCD_H_RES, y + h, buffer));
    }

    free(buffer);
}

