/* ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "app_camera.h"
#include "app_wifi.h"
#include "app_httpd.h"
#include <u8g2_esp32_hal.h>


void oled(void *pvParameters)
{
    u8g2_t u8g2;
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, (CONFIG_ADR << 1)); // set the display address shifted left (0x3C == 0X78) => ( 0011 1100 = 0111 1000)
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    while (1) {
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_timR14_tf);
        u8g2_DrawStr(&u8g2, 2, 48, "TESTE!!");
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(1000 / portTICK_RATE_MS);

        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_timR14_tf);
        u8g2_DrawStr(&u8g2, 2, 17, "Hello World!");
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


void app_main()
{

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = CONFIG_SDA;
    u8g2_esp32_hal.scl = CONFIG_SCL;
    u8g2_esp32_hal.reset = CONFIG_RST;
    u8g2_esp32_hal_init(u8g2_esp32_hal);
    
    app_wifi_main();
    app_camera_main();
    app_httpd_main();
    
    xTaskCreate(&oled, "oled", 4096, NULL, 1, NULL);


}
