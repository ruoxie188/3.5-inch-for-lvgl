#ifndef ESP_HPY_H
#define ESP_HPY_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h" // For I2C_NUM_0 definition
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================================
// Board Hardware Configuration
// ====================================================================

// -- Display Panel Pins & Parameters --
#define LCD_PIXEL_CLOCK_HZ      (5 * 1000 * 1000) // 16位总线速度可以更快，可根据实际情况调整，例如 (15 * 1000 * 1000)
#define LCD_H_RES               480
#define LCD_V_RES               320
#define LCD_BK_LIGHT_ON_LEVEL   1

// 16-bit Parallel Data Pins
#define PIN_NUM_DATA0           15
#define PIN_NUM_DATA1           16
#define PIN_NUM_DATA2           17
#define PIN_NUM_DATA3           18
#define PIN_NUM_DATA4           21
#define PIN_NUM_DATA5           40
#define PIN_NUM_DATA6           39
#define PIN_NUM_DATA7           38
#define PIN_NUM_DATA8           47
#define PIN_NUM_DATA9           48
#define PIN_NUM_DATA10          7
#define PIN_NUM_DATA11          8
#define PIN_NUM_DATA12          9
#define PIN_NUM_DATA13          10
#define PIN_NUM_DATA14          11
#define PIN_NUM_DATA15          12

// Control Pins
#define PIN_NUM_PCLK            42  // WR pin
#define PIN_NUM_CS              46
#define PIN_NUM_DC              45  // RS pin
#define PIN_NUM_RST             -1  // -1 if not used
#define PIN_NUM_BK_LIGHT        13
#define PIN_NUM_RD              41

// -- Touch Panel Pins --
#define TOUCH_I2C_NUM           (I2C_NUM_0)
#define TOUCH_I2C_SCL           (GPIO_NUM_2)
#define TOUCH_I2C_SDA           (GPIO_NUM_1)
#define TOUCH_INT               (GPIO_NUM_3)
#define TOUCH_RST               (-1) // -1 if not used

// -- Performance/Driver Parameters --
#define PSRAM_DATA_ALIGNMENT    64
#define LCD_DRAW_BUF_HEIGHT     (50)


// ====================================================================
// Public API
// ====================================================================

// -- Public Handles --
extern esp_lcd_panel_handle_t g_lcd_panel_handle;
extern esp_lcd_panel_io_handle_t g_io_handle; // <-- 【新增】为 LVGL port 提供 IO 句柄
extern esp_lcd_touch_handle_t g_tp_handle;

// -- Public Function Declarations --
esp_err_t bsp_display_init(void);
esp_err_t bsp_touch_init(void);
void bsp_fill_screen(uint16_t color); // 保留此函数用于可能的调试

#ifdef __cplusplus
}
#endif

#endif // ESP_HPY_H

