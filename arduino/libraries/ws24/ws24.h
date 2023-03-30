#pragma once

#include <SPI.h>
#include <stdint.h>


struct Font;


// An RGB pixel is sent to the display in two bytes: 5 bits red, 6 bits green,
// 5 bits blue, packed as follows. The packing assumes a uint16_t is stored
// little-endian, so SPI sends the lower byte first. The LCD expects to
// receive 5 bits red, 6 bits green, then 5 bits blue, each MSB first.
//
// Packed pixel in memory:
//     | 15 14 13 12 11 10  9  8 |  7  6  5  4  3  2  1  0 |
//     | g4 g3 g2 b7 b6 b5 b4 b3 | r7 r6 r5 r4 r3 g7 g6 g5 |
//
// Pixel as SPIed to display module:
//       little end first,         then big end
//     | r7 r6 r5 r4 r3 g7 g6 g5 | g4 g3 g2 b7 b6 b5 b4 b3 |

class Pixel {

    public:

        Pixel() : _pixel(0) {}

        Pixel(uint8_t r, uint8_t g, uint8_t b) :
            _pixel((((uint16_t)g << 11) & 0xe000) |
                   (((uint16_t)b << 5) & 0x1f00) |
                   (((uint16_t)r << 0) & 0x00f8) |
                   (((uint16_t)g >> 5) & 0x0007))
        {
        }

        void rgb(uint8_t& r, uint8_t& g, uint8_t& b) const
        {
            r = (uint8_t)(_pixel & 0x00f8);
            g = (uint8_t)(((_pixel & 0x0007) << 5) | ((_pixel & 0xe000) >> 11));
            b = (uint8_t)((_pixel & 0x1f00) >> 5);
        }

        uint16_t raw() const
        {
            return _pixel;
        }

        static const Pixel black;
        static const Pixel white;
        static const Pixel red;
        static const Pixel green;
        static const Pixel blue;

    private:

        uint16_t _pixel;
};


class Ws24 {

    public:

        Ws24(SPIClass& spi,
             int gpio_spi_cs, int gpio_dc,
             int gpio_reset, int gpio_bl,
             uint8_t *work, int work_bytes);

        virtual ~Ws24();

        // reset and initialize
        // rotate=0, 90, 180, 270, -90
        // rotate=90 means screen is rotated 90 clockwise
        // br = 0 (off) ... 255 (max)
        // returns true on success, false on error
        bool begin(int rotate=0, int br=128);

        // write a rectangle of pixels to screen
        void write(uint16_t row, uint16_t col,
                   uint16_t height, uint16_t width,
                   Pixel *data); // data[height * width], overwritten

        // fill a rectangle with a solid color
        void write(uint16_t row, uint16_t col,
                   uint16_t height, uint16_t width,
                   Pixel pixel); // one Pixel to fill rectangle

        // set backlight brightness (0..255)
        void brightness(int br);

        // screnn height and width; these change with rotation
        uint16_t height() const { return _height; }
        uint16_t width() const { return _width; }

        // print character to display
        void print(const Font& font, uint16_t row, uint16_t col,
                   Pixel fg, Pixel bg, char c);

        // print string to display
        void print(const Font& font, uint16_t row, uint16_t col,
                   Pixel fg, Pixel bg, const char *str);

    private:

        // ILI9341 command bytes
        static const uint8_t sleep_out = 0x11;
        static const uint8_t gamma_set = 0x26;
        static const uint8_t display_on = 0x29;
        static const uint8_t column_adrs_set = 0x2a;
        static const uint8_t page_adrs_set = 0x2b;
        static const uint8_t memory_write = 0x2c;
        static const uint8_t color_set = 0x2d;
        static const uint8_t memory_access_ctl = 0x36;
        static const uint8_t pixel_format_set = 0x3a;
        static const uint8_t memory_write_continue = 0x3c;
        static const uint8_t set_tear_scanline = 0x44;
        static const uint8_t frame_rate_ctl = 0xb1;
        static const uint8_t display_function_ctl = 0xb6;
        static const uint8_t power_ctl_1 = 0xc0;
        static const uint8_t power_ctl_2 = 0xc1;
        static const uint8_t vcom_ctl_1 = 0xc5;
        static const uint8_t vcom_ctl_2 = 0xc7;
        static const uint8_t power_ctl_a = 0xcb;
        static const uint8_t power_ctl_b = 0xcf;
        static const uint8_t pos_gamma_corr = 0xe0;
        static const uint8_t neg_gamma_corr = 0xe1;
        static const uint8_t driver_timing_ctl_a = 0xe8;
        static const uint8_t driver_timing_ctl_b = 0xea;
        static const uint8_t power_on_sequence_ctl = 0xed;
        static const uint8_t enable_3g = 0xf2;
        static const uint8_t pump_ratio_ctl = 0xf7;

        // physical height/width (portrait, ribbon cable at bottom)
        static const uint16_t phy_height = 320;
        static const uint16_t phy_width = 240;

        // SPI settings
        static const uint32_t spi_clock = 4000000;
        static const BitOrder spi_bitorder = MSBFIRST;
        static const SPIMode spi_mode = SPI_MODE3;

        SPIClass& _spi;

        SPISettings _spi_settings;

        int _gpio_spi_cs;
        int _gpio_dc;
        int _gpio_reset;
        int _gpio_bl;

        // logical height/width, possibly rotated from physical
        uint16_t _height;
        uint16_t _width;

        // Work buffer used in a few places:
        // * initializing colors lut (must be at least 128 bytes)
        // * filling rectangles on screen (any size is okay, but bigger means
        //   fewer transfers)
        // * rendering character (must be big enough for biggest font used)
        // Supplied to constructor because that's where needed size is known
        uint8_t *_work;
        int _work_bytes;

        void hw_reset();

        void write(uint8_t cmd);
        void write(uint8_t cmd, uint8_t p1);
        void write(uint8_t cmd, uint16_t p1, uint16_t p2);
        void write(uint8_t cmd, void *buf, int buf_len);

        bool init_colors();
};
