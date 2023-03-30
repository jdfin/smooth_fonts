#include <SPI.h>
#include "ws24.h"
#include "consolas_24.h"
#include "consolas_italic_36.h"
#include "curlz_mt_36.h"
#include "comic_sans_ms_48.h"


// gpio pins
static const int gpio_spi_cs = 17;
static const int gpio_lcd_dc = 10;
static const int gpio_lcd_reset = 11;
static const int gpio_lcd_bl = 12;

constexpr int work_bytes =
    comic_sans_ms_48_max_height * comic_sans_ms_48_max_width * sizeof(Pixel);
uint8_t work[work_bytes];

Ws24 lcd(SPI, gpio_spi_cs, gpio_lcd_dc, gpio_lcd_reset, gpio_lcd_bl,
         work, work_bytes);


void setup()
{
    Serial.begin(115200);

    SPI.begin();

    // rotate 90 left, backlight 100%
    lcd.begin(-90, 255);

    // clear
    lcd.write(0, 0, lcd.height(), lcd.width(), Pixel::white);

    uint16_t row = 0;

    lcd.print(comic_sans_ms_48, row, 0, Pixel::black, Pixel::white,
              "Comic Sans MS 48");
    row += comic_sans_ms_48.height();

    lcd.print(curlz_mt_36, row, 0, Pixel::black, Pixel::white,
              "Curlz MT 36");
    row += curlz_mt_36.height();

    lcd.print(consolas_24, row, 0, Pixel::red, Pixel::white,
              "Consolas 24");
    row += consolas_24.height();

    lcd.print(consolas_24, row, 0, Pixel::green, Pixel::black,
              "!\"#$%&'()*+,-./:");
    lcd.print(consolas_24, row, consolas_24.max_width() * 20,
              Pixel::blue, Pixel::white, "01234");
    row += consolas_24.height();

    lcd.print(consolas_24, row, 0, Pixel::green, Pixel::black,
              ";<=>?@[\\]^_`{|}~");
    lcd.print(consolas_24, row, consolas_24.max_width() * 20,
              Pixel::blue, Pixel::white, "56789");
    row += consolas_24.height();

    lcd.print(consolas_24, row, 0, Pixel::blue, Pixel::white,
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    row += consolas_24.height();

    lcd.print(consolas_24, row, 0, Pixel::blue, Pixel::white,
              "abcdefghijklmnopqrstuvwxyz");
    row += consolas_24.height();

    for (int i = 0; i < 10; i++) {
        char c = '0' + i;
        uint16_t col = i * 32 + 16 - consolas_italic_36.width(c) / 2;
        lcd.print(consolas_italic_36, row, col, Pixel::red, Pixel::white, c);
    }
    row += consolas_italic_36.height();
}


void loop()
{
}
