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
#define LCD_PIXEL_CLOCK_HZ      (10 * 1000 * 1000)
#define LCD_H_RES               480
#define LCD_V_RES               320
#define LCD_BK_LIGHT_ON_LEVEL   1
#define PIN_NUM_DATA0           15
#define PIN_NUM_DATA1           16
#define PIN_NUM_DATA2           17
#define PIN_NUM_DATA3           18
#define PIN_NUM_DATA4           21
#define PIN_NUM_DATA5           40
#define PIN_NUM_DATA6           39
#define PIN_NUM_DATA7           38
#define PIN_NUM_PCLK            42
#define PIN_NUM_CS              46
#define PIN_NUM_DC              45
#define PIN_NUM_RST             -1
#define PIN_NUM_BK_LIGHT        47
#define PIN_NUM_RD              41

// -- Touch Panel Pins --
#define TOUCH_I2C_NUM           (I2C_NUM_0)
#define TOUCH_I2C_SCL           (GPIO_NUM_2)
#define TOUCH_I2C_SDA           (GPIO_NUM_1)
#define TOUCH_INT               (GPIO_NUM_3)
#define TOUCH_RST               (-1)

// -- Performance/Driver Parameters --
#define PSRAM_DATA_ALIGNMENT    64
#define LCD_DRAW_BUF_HEIGHT     (50)


// ====================================================================
// Public API
// ====================================================================

// -- Public Handles --
extern esp_lcd_panel_handle_t g_lcd_panel_handle;
extern esp_lcd_touch_handle_t g_tp_handle;

// -- Public Function Declarations --
esp_err_t bsp_display_init(void);
esp_err_t bsp_touch_init(void);
void bsp_fill_screen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // ESP_HPY_H

