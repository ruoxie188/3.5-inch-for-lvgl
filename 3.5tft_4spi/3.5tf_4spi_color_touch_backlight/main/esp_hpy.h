#ifndef ESP_HPY_H
#define ESP_HPY_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/ledc.h" // For LEDC (PWM) definitions
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================================
// Board Hardware Configuration
// ====================================================================

// -- Display Panel Pins & Parameters (4-Wire SPI) --
#define LCD_HOST                SPI2_HOST
#define LCD_PIXEL_CLOCK_HZ      (40 * 1000 * 1000) // SPI 时钟, 40MHz
#define LCD_H_RES               480
#define LCD_V_RES               320

// SPI 引脚定义
#define PIN_NUM_SCLK            42
#define PIN_NUM_MOSI            41  // SDA
#define PIN_NUM_MISO            15  // SDO
#define PIN_NUM_CS              46
#define PIN_NUM_DC              45  // RS (Data/Command)
#define PIN_NUM_RST             16
#define PIN_NUM_BK_LIGHT        17  // Backlight

// -- Backlight PWM Configuration --
#define LCD_BK_LIGHT_LEDC_TIMER         LEDC_TIMER_0
#define LCD_BK_LIGHT_LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LCD_BK_LIGHT_LEDC_CHANNEL       LEDC_CHANNEL_0
#define LCD_BK_LIGHT_LEDC_DUTY_RES      LEDC_TIMER_10_BIT // PWM 分辨率 (10-bit -> 1024 级)
#define LCD_BK_LIGHT_LEDC_FREQ_HZ       (5000)            // PWM 频率 5kHz

// -- Touch Panel Pins (保持不变) --
#define TOUCH_I2C_NUM           (I2C_NUM_0)
#define TOUCH_I2C_SCL           (GPIO_NUM_2)
#define TOUCH_I2C_SDA           (GPIO_NUM_1)
#define TOUCH_INT               (GPIO_NUM_3)
#define TOUCH_RST               (-1) // -1 表示不使用

// -- Performance/Driver Parameters --
#define PSRAM_DATA_ALIGNMENT    64
#define LCD_DRAW_BUF_HEIGHT     (50) // 每次绘制的高度，影响RAM使用


// ====================================================================
// Public API
// ====================================================================

// -- Public Handles --
extern esp_lcd_panel_handle_t g_lcd_panel_handle;
extern esp_lcd_touch_handle_t g_tp_handle;

// -- Public Function Declarations --

/**
 * @brief 初始化显示屏 (SPI接口和背光)
 * @return esp_err_t
 */
esp_err_t bsp_display_init(void);

/**
 * @brief 初始化触摸屏 (I2C接口)
 * @return esp_err_t
 */
esp_err_t bsp_touch_init(void);

/**
 * @brief 用指定颜色填充整个屏幕
 * @param color 16-bit RGB565 颜色值
 */
void bsp_fill_screen(uint16_t color);

/**
 * @brief 设置屏幕背光亮度
 * @param brightness_percent 亮度百分比 (0-100)
 */
void bsp_display_set_backlight(uint32_t brightness_percent);

#ifdef __cplusplus
}
#endif

#endif // ESP_HPY_H

