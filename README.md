ESP32S3N8R8三种方式驱动3.5寸触摸屏且运行LVGL
【立创esp32s3n8r8开发板使用spi/8080/16位并口在3.5寸触摸屏运行lvgl性能对比】 https://www.bilibili.com/video/BV1jYHczKExs/?share_source=copy_web&vd_source=bc7a46888bfad0526d566d36e633de14)

## 项目介绍
ESP32S3N8R8采用4线SPI/8080协议/16bit并口驱动3.5寸触摸屏，3.5寸屏幕(屏幕驱动ST7796,触摸芯片FT6336)，同时运行LVGL

##基础硬件

3.5寸触摸屏
立创ESP32S3N8R8开发板

##软件部分

软件部分主要依赖espidf 组件
  espressif/esp_lcd_st7796: ^1.3.0
  espressif/esp_lcd_touch_ft5x06: ==1.0.0
  还有lvgl环境
    lvgl/lvgl: ~8.3.0
  espressif/esp_lvgl_port: ^2.6.2
