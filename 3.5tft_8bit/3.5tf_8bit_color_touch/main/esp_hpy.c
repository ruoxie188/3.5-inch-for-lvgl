#include "esp_hpy.h" // Our header now contains all hardware definitions

// Implementation-specific headers
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lcd_st7796.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <assert.h>

static const char *TAG = "ESP_HPY";

// -- Public Handle Definitions --
esp_lcd_panel_handle_t g_lcd_panel_handle = NULL;
esp_lcd_touch_handle_t g_tp_handle = NULL;


esp_err_t bsp_display_init(void)
{
    ESP_LOGI(TAG, "Initializing display...");

    // Configure GPIOs
    gpio_config_t bk_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_config_t rd_gpio_config = {.mode = GPIO_MODE_OUTPUT, .pin_bit_mask = 1ULL << PIN_NUM_RD};
    ESP_ERROR_CHECK(gpio_config(&rd_gpio_config));
    ESP_ERROR_CHECK(gpio_set_level(PIN_NUM_RD, 1));

    // Configure i80 bus
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT, .dc_gpio_num = PIN_NUM_DC, .wr_gpio_num = PIN_NUM_PCLK,
        .data_gpio_nums = { PIN_NUM_DATA0, PIN_NUM_DATA1, PIN_NUM_DATA2, PIN_NUM_DATA3, PIN_NUM_DATA4, PIN_NUM_DATA5, PIN_NUM_DATA6, PIN_NUM_DATA7, },
        .bus_width = 8, .max_transfer_bytes = LCD_H_RES * LCD_DRAW_BUF_HEIGHT * sizeof(uint16_t),
        .psram_trans_align = PSRAM_DATA_ALIGNMENT, .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    // Configure panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS, .pclk_hz = LCD_PIXEL_CLOCK_HZ, .trans_queue_depth = 10,
        .dc_levels = {.dc_idle_level = 0, .dc_cmd_level = 0, .dc_dummy_level = 0, .dc_data_level = 1},
        .flags.swap_color_bytes = true, .lcd_cmd_bits = 8, .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    // Install panel driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST, .color_space = ESP_LCD_COLOR_SPACE_BGR, .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7796(io_handle, &panel_config, &g_lcd_panel_handle));

    // Initialize panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(g_lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(g_lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(g_lcd_panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(g_lcd_panel_handle, true, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(g_lcd_panel_handle, true));
    
    // Turn on backlight
    ESP_LOGI(TAG, "Turning on backlight");
    gpio_set_level(PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL);

    return ESP_OK;
}

esp_err_t bsp_touch_init(void)
{
    ESP_LOGI(TAG, "Initializing touch panel...");

    // Configure I2C bus
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

    // Configure touch panel IO
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)TOUCH_I2C_NUM, &tp_io_config, &tp_io_handle));
    
    // Install touch driver
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

    for (int i = 0; i < LCD_H_RES * LCD_DRAW_BUF_HEIGHT; i++) {
        buffer[i] = color;
    }

    for (int y = 0; y < LCD_V_RES; y += LCD_DRAW_BUF_HEIGHT) {
        int h = (y + LCD_DRAW_BUF_HEIGHT) > LCD_V_RES ? (LCD_V_RES - y) : LCD_DRAW_BUF_HEIGHT;
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(g_lcd_panel_handle, 0, y, LCD_H_RES, y + h, buffer));
    }
    
    free(buffer);
}

