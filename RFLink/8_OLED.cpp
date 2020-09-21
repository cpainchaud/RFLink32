// ************************************* //
// * Arduino Project RFLink-esp        * //
// * https://github.com/couin3/RFLink  * //
// * 2018..2020 Stormteam - Marc RIVES * //
// * More details in RFLink.ino file   * //
// ************************************* //

#include <Arduino.h>
#include "RFLink.h"
#include "8_OLED.h"

#ifdef OLED_ENABLED

#include "4_Display.h"
#include "8_OLED.h"
#include <U8x8lib.h> // Comment to avoid dependency graph inclusion

#define U8X8_PIN_NONE 255
// 0.91" I2C 128x32 Display
// U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE, /* clock=*/PIN_OLED_SCL, /* data=*/PIN_OLED_SDA);
// U8X8_SSD1306_128X64_ALT0_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE, /* clock=*/PIN_OLED_SCL, /* data=*/PIN_OLED_SDA);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE, /* clock=*/PIN_OLED_SCL, /* data=*/PIN_OLED_SDA);
// U8X8_SSD1306_128X64_VCOMH0_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE, /* clock=*/PIN_OLED_SCL, /* data=*/PIN_OLED_SDA);

#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8X8LOG u8x8log;

void setup_OLED()
{
    u8x8.begin();
    u8x8.setPowerSave(1);
    u8x8.setContrast(OLED_CONTRAST);
    u8x8.setFlipMode(OLED_FLIP);

    u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
    u8x8log.setRedrawMode(0); // 0: Update screen with newline, 1: Update screen for every char
    u8x8log.setLineHeightOffset(0);
}

void splash_OLED()
{
    char ver[7];
    sprintf_P(ver, PSTR("v%d.%d"), BUILDNR, REVNR);
    u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
    u8x8.draw2x2String(0, 0, "RFLink");
    u8x8.draw2x2String(10, 2, "ESP");
    u8x8.drawString(0, 3, ver);
    u8x8.drawString(0, 4, "20/09/20");
    u8x8.drawString(6, 6, "github.com");
    u8x8.drawString(2, 7, "/couin3/RFLink");
    u8x8.setPowerSave(0);
}

void print_OLED()
{
    /*
    static char delim[2] = ";";
    static char *ptr;
    static boolean ret;

 
    ptr = strtok(pbuffer, delim);
    ret = false;

    while ((ptr != NULL) && (ptr[0] != '\r'))
    {
        u8x8log.print(ptr);
        u8x8log.print(delim);
        if (ret)
            u8x8log.print("\n");
        else
            ret = true;
        ptr = strtok(NULL, delim);
    }
*/
    u8x8log.print('\f');
    replacechar(pbuffer, ';', '\n');
    u8x8log.print(pbuffer);
}

#endif // OLED_ENABLED