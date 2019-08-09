#include <u8g2_esp32_hal.h>
#include "tcpip_adapter.h"
#include <string.h>
#include "esp_wifi.h"
#include "app_main.h"


static u8g2_t u8g2;

void drawScrollString(int16_t offset, int dy, const char *s)
{
    static char buf[128];  // should for screen with up to 256 pixel width
    size_t len;
    size_t char_offset = 0;
    u8g2_uint_t dx = 0;
    size_t visible = 0;
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, 0, 49, u8g2_GetDisplayWidth(&u8g2) - 1, u8g2_GetDisplayHeight(&u8g2) - 1);
    u8g2_SetDrawColor(&u8g2, 1);

    len = strlen(s);
    if ( offset < 0 ) {
        char_offset = (-offset) / 8;
        dx = offset + char_offset * 8;
        if ( char_offset >= u8g2_GetDisplayWidth(&u8g2) / 8 )
            return;
        visible = u8g2_GetDisplayWidth(&u8g2) / 8 - char_offset + 1;
        strncpy(buf, s, visible);
        buf[visible] = '\0';
        u8g2_SetFont(&u8g2, u8g2_font_8x13_mf);
        u8g2_DrawStr(&u8g2, char_offset * 8 - dx, dy, buf);
    } else {
        char_offset = offset / 8;
        if ( char_offset >= len )
            return;   // nothing visible
        dx = offset - char_offset * 8;
        visible = len - char_offset;
        if ( visible > u8g2_GetDisplayWidth(&u8g2) / 8 + 1 )
            visible = u8g2_GetDisplayWidth(&u8g2) / 8 + 1;
        strncpy(buf, s + char_offset, visible);
        buf[visible] = '\0';
        u8g2_SetFont(&u8g2, u8g2_font_8x13_mf);
        u8g2_DrawStr(&u8g2, -dx, dy, buf);
    }
}

void oled(void *pvParameters)
{
    wifi_mode_t mode = WIFI_MODE_NULL;
    static char buff[256];
    esp_wifi_get_mode(&mode);
    if ( mode == WIFI_MODE_STA) {
        tcpip_adapter_ip_info_t ip;
        tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip);
        const char *addr = ip4addr_ntoa(&ip.ip);
        snprintf(buff, sizeof(buff), "Camera Ready! Use '%s' to connect", addr);
    } else {
        snprintf(buff, sizeof(buff), "Camera Ready! Use '%s' to connect", CONFIG_SERVER_IP);
    }

    int16_t offset = -(int16_t) u8g2_GetDisplayWidth(&u8g2);
    bool irq = false;

    u8g2_ClearBuffer(&u8g2);
    while (1) {

        int16_t len = strlen(buff);

        if (offset < len * 8 + 1) {
            drawScrollString(offset, 64, buff);           // no clearBuffer required, screen will be partially cleared here
        } else {
            offset = -(int16_t) u8g2_GetDisplayWidth(&u8g2);
        }
        offset += 2;
        if (gpio_get_level(CONFIG_PIR_PIN)) {
            if (!irq) {
                irq = true;

                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 0, 0, 128, 58);
                u8g2_SetDrawColor(&u8g2, 1);

                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_embedded_4x_t);
                u8g2_DrawGlyph(&u8g2, 5, 42, 67);

                u8g2_SetFont(&u8g2, u8g2_font_logisoso16_tr);
                u8g2_DrawStr(&u8g2, 45, 35, "PirVaild");
            }
        } else {
            if (irq) {
                irq = false;
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawBox(&u8g2, 0, 0, 128, 58);
                u8g2_SetDrawColor(&u8g2, 1);
                u8g2_SetFont(&u8g2, u8g2_font_logisoso16_tr);
                u8g2_DrawStr(&u8g2, 20, 30, "PirInvalid");
            }
        }
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(20 / portTICK_RATE_MS);
    }
}

en_fsm_state g_state = WAIT_FOR_WAKEUP;

void app_main()
{

    app_speech_wakeup_init();

    g_state = WAIT_FOR_WAKEUP;

    vTaskDelay(30 / portTICK_PERIOD_MS);

    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = CONFIG_SDA;
    u8g2_esp32_hal.scl = CONFIG_SCL;
    u8g2_esp32_hal.reset = CONFIG_RST;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R2, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, (CONFIG_ADR << 1)); // set the display address shifted left (0x3C == 0X78) => ( 0011 1100 = 0111 1000)
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    gpio_config_t gpio_conf;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.pin_bit_mask = 1LL << CONFIG_PIR_PIN;
    gpio_config(&gpio_conf);

    u8g2_ClearBuffer(&u8g2);

    char buff[256] = "Please say 'Hi LeXin' to the board";
    int16_t len = strlen(buff) ;
    int16_t offset = -(int16_t) u8g2_GetDisplayWidth(&u8g2);
    while (g_state == WAIT_FOR_WAKEUP) {
        drawScrollString(offset, 64, buff);           // no clearBuffer required, screen will be partially cleared here
        u8g2_SendBuffer(&u8g2);              // transfer internal memory to the display
        vTaskDelay(5 / portTICK_PERIOD_MS);
        offset += 2;
        if ( offset > len * 8 + 1 )
            offset = -(int16_t) u8g2_GetDisplayWidth(&u8g2);
    }

    app_wifi_main();
    app_camera_main();
    app_httpd_main();

    xTaskCreate(&oled, "oled", 4096, NULL, 1, NULL);
}




