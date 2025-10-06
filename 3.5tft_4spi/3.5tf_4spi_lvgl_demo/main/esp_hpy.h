#ifndef ESP_HPY_H
#define ESP_HPY_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================================
// Board Hardware Configuration
// ====================================================================
#define LCD_HOST                SPI2_HOST
#define LCD_PIXEL_CLOCK_HZ      (40 * 1000 * 1000)
#define LCD_H_RES               480
#define LCD_V_RES               320
#define LCD_BK_LIGHT_ON_LEVEL   1

#define PIN_NUM_SCLK            42
#define PIN_NUM_MOSI            41
#define PIN_NUM_MISO            15
#define PIN_NUM_CS              46
#define PIN_NUM_DC              45
#define PIN_NUM_RST             16
#define PIN_NUM_BK_LIGHT        17

#define TOUCH_I2C_NUM           (I2C_NUM_0)
#define TOUCH_I2C_SCL           (GPIO_NUM_2)
#define TOUCH_I2C_SDA           (GPIO_NUM_1)
#define TOUCH_INT               (GPIO_NUM_3)
#define TOUCH_RST               (-1)

#define PSRAM_DATA_ALIGNMENT    64
#define LCD_DRAW_BUF_HEIGHT     (50)

// ====================================================================
// Public API
// ====================================================================
extern esp_lcd_panel_handle_t g_lcd_panel_handle;
extern esp_lcd_panel_io_handle_t g_io_handle;
extern esp_lcd_touch_handle_t g_tp_handle;

esp_err_t bsp_display_init(void);
esp_err_t bsp_touch_init(void);
void bsp_fill_screen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif // ESP_HPY_H

