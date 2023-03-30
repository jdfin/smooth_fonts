#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include "font.h"
#include "ws24.h"

const Pixel Pixel::black(0, 0, 0);
const Pixel Pixel::white(255, 255, 255);
const Pixel Pixel::red(255, 0, 0);
const Pixel Pixel::green(0, 255, 0);
const Pixel Pixel::blue(0, 0, 255);


Ws24::Ws24(SPIClass& spi,
           int gpio_spi_cs, int gpio_dc, int gpio_reset, int gpio_bl,
           uint8_t *work, int work_bytes) :
    _spi(spi),
    _spi_settings(spi_clock, spi_bitorder, spi_mode),
    _gpio_spi_cs(gpio_spi_cs),
    _gpio_dc(gpio_dc),
    _gpio_reset(gpio_reset),
    _gpio_bl(gpio_bl),
    _height(phy_height),
    _width(phy_width),
    _work(work),
    _work_bytes(work_bytes)
{
    digitalWrite(_gpio_spi_cs, 1);
    digitalWrite(_gpio_reset, 0);
    digitalWrite(_gpio_dc, 0);
    pinMode(_gpio_spi_cs, OUTPUT);
    pinMode(_gpio_reset, OUTPUT);
    pinMode(_gpio_dc, OUTPUT);
    pinMode(_gpio_bl, OUTPUT);
    brightness(0); // off
}


Ws24::~Ws24()
{
}


// reset and initialize
bool Ws24::begin(int rotate, int br)
{
    hw_reset();

    // chip is in sleep mode out of reset

    write(sleep_out);

    delay(150); // 120 msec minimum after sleep out command

    // power control b
    // power_control = 0b00
    // dc_ena = 1               ESD protection enabled
    uint8_t b0[] = {
        0x00, // 00000000 ok
        0xc1, // 101xx010 invalid
        0x30  // 111x0000 invalid
    };
    write(power_ctl_b, b0, sizeof(b0));

    // power on sequence control
    // (documentation is inconsistent/wrong)
    // cp1 soft start = 0b10    keep 1 frame
    // cp23 soft start = 0b00   keep 3 frame
    // en_vcl = 0b00            1st frame enable
    // en_ddvdh = 0b11          4th frame enable
    // en_vgh = 0b01            2nd frame enable
    // en_vgl = 0b10            3rd frame enable
    // ddvdh_enh = 0b10
    uint8_t b1[] = {
        0x64, // x1xxx1xx ok
        0x03, // x1xxx1xx invalid
        0x12, // x1xxx1xx invalid
        0x81  // xx000000 invalid
    };
    write(power_on_sequence_ctl, b1, sizeof(b1));

    // driver timing control a
    // now = 1                  default + 1 unit
    // eq = 0                   default - 1 unit
    // pc = 0b01                default - 1 unit
    uint8_t b2[] = {
        0x85, // 1000010x ok
        0x00, // 000x0001 invalid
        0x79  // 011110xx ok
    };
    write(driver_timing_ctl_a, b2, sizeof(b2));

    // power control a
    // reg_vd = 0b100           Vcore = 1.6V
    // vbc = 0b010              DDVDH = 5.8V
    uint8_t b3[] = { 0x39, 0x2c, 0x00, 0x34, 0x02 };
    write(power_ctl_a, b3, sizeof(b3));

    // pump ratio control
    // ratio = 0b10             DDVDH=2xVCI
    write(pump_ratio_ctl, 0x20);

    // driver timing control b
    // vg_sw_t4 = 0b00          0 unit
    // vg_sw_t3 = 0b00          0 unit
    // vg_sw_t2 = 0b00          0 unit
    // vg_sw_t1 = 0b00          0 unit
    uint8_t b4[] = {
        0x00, // xxxxxxxx ok
        0x00  // xxxxxx00 ok
    };
    write(driver_timing_ctl_b, b4, sizeof(b4));

    // power control 1
    // vrh = 0b011101           GVDD = 4.30V
    write(power_ctl_1, 0x1d);

    // power control 2
    // bt = 0b010               AVDD = VCIx2, VGH = VCIx6, VGL = -VCIx4
    write(power_ctl_2, 0x12);

    // vcom control 1
    // vmh = 0b0110011          VCOMH = 3.975V
    // vml = 0b0111111          VCOML = -0.925V
    uint8_t b5[] = { 0x33, 0x3f };
    write(vcom_ctl_1, b5, sizeof(b5));

    // vcom control 2
    // nvm=1
    // vmf=0b0010010            VCOMH = VMH - 46, VCOML = VML - 46
    write(vcom_ctl_2, 0x92);

    write(pixel_format_set, 0x55);

    uint8_t madctl_param = 0x08;
    _height = phy_height;
    _width = phy_width;
    if (rotate == 90) {
        madctl_param |= 0x60;
        _height = phy_width;
        _width = phy_height;
    } else if (rotate == 180) {
        madctl_param |= 0xc0;
    } else if (rotate == -90 || rotate == 270) {
        madctl_param |= 0xa0;
        _height = phy_width;
        _width = phy_height;
    }
    write(memory_access_ctl, madctl_param);

    // frame rate control
    // diva = 0b00              fosc
    // rtna = 0b10010           18 clocks, 106 Hz (?)
    uint8_t b6[] = { 0x00, 0x12 };
    write(frame_rate_ctl, b6, sizeof(b6));

    // display function control
    // XXX should have 4 parameters
    // ptg = 0b10               interval scan
    // pt = 0b10                AGND
    // rev = 1                  normally white
    // gs = 0
    // ss = 1                   S720->S1
    // sm = 0
    // isc = 0b0010             scan cycle 5 frames, fFLM = 85ms
    uint8_t b7[] = { 0x0a, 0xa2 };
    write(display_function_ctl, b7, sizeof(b7));

    // set tear scanline
    // XXX should be 2 parameters
    // 0x02 as first parameter is invalid
    write(set_tear_scanline, 0x02);

    // enable 3 gamma
    // 3g_enb = 0               3 gamma control disabled
    write(enable_3g, 0x00);

    // gamma set
    // gc = 0b00000001          gamma curve 1
    write(gamma_set, 0x01);

    uint8_t b8[] = { 0x0f, 0x22, 0x1c, 0x1b, 0x08, 0x0f, 0x48, 0xb8,
                     0x34, 0x05, 0x0c, 0x09, 0x0f, 0x07, 0x00 };
    write(pos_gamma_corr, b8, sizeof(b8));

    uint8_t b9[] = { 0x00, 0x23, 0x24, 0x07, 0x10, 0x07, 0x38, 0x47,
                     0x4b, 0x0a, 0x13, 0x06, 0x30, 0x38, 0x0f };
    write(neg_gamma_corr, b9, sizeof(b9));

    if (!init_colors())
        return false;

    write(display_on);

    brightness(br);

    return true;
}


// write a rectangle of pixels to screen
void Ws24::write(uint16_t row, uint16_t col,
                 uint16_t height, uint16_t width,
                 Pixel *data) // data[height * width], overwritten
{
    write(page_adrs_set, row, row + height - 1);
    write(column_adrs_set, col, col + width - 1);
    write(memory_write, data, height * width * 2);
}


// Fill a rectangle with a solid color.
// Use the work buffer, sending that much repeatedly until enough pixels have
// been sent.
void Ws24::write(uint16_t row, uint16_t col,
                 uint16_t height, uint16_t width, Pixel pixel)
{
    uint32_t cnt_pixels = (uint32_t)(height) * (uint32_t)(width);
    uint8_t cmd_byte = memory_write;

    // work buffer, used to hold Pixels (two bytes each)
    const int work_pixels = _work_bytes / sizeof(Pixel);
    Pixel *work_pix = (Pixel *)_work;

    write(page_adrs_set, row, row + height - 1);
    write(column_adrs_set, col, col + width - 1);

    while (cnt_pixels > 0) {

        // spi_pixels is minimum of pixels left and work size in pixels
        uint32_t spi_pixels = cnt_pixels;
        if (spi_pixels > work_pixels)
            spi_pixels = work_pixels;

        // fill work buf with pixel (it gets overwritten each time)
        for (int i = 0; i < spi_pixels; i++)
            work_pix[i] = pixel;

        // Write data. First time is memory_write, after that
        // memory_write_continue.
        write(cmd_byte, _work, spi_pixels * 2);

        cnt_pixels -= spi_pixels;
        cmd_byte = memory_write_continue;
    }
}


// set backlight brightness (0..255)
void Ws24::brightness(int br)
{
    if (br < 0)
        br = 0;
    else if (br > 255)
        br = 255;
    analogWrite(_gpio_bl, br);
}


// print character to screen
void Ws24::print(const Font& font, uint16_t row, uint16_t col,
                 Pixel fg, Pixel bg, char c)
{
    if (c < 0 || c > 127)
        return;

    // pixels we need for this particular glyph
    int num_pixels = font.y_adv * font.info[c].x_adv;

    Pixel *pix_buf = (Pixel *)_work;
    int pix_buf_len = _work_bytes / sizeof(Pixel);

    if (num_pixels > pix_buf_len)
        return;

    // get rgb components of foreground/background; used for smoothing
    uint8_t fg_r, fg_g, fg_b;
    fg.rgb(fg_r, fg_g, fg_b);
    uint8_t bg_r, bg_g, bg_b;
    bg.rgb(bg_r, bg_g, bg_b);

    // start of glyph data
    const uint8_t *gs = font.data + font.info[c].off;

    Pixel *pix;

    // for each pixel in glyph:
    // *gs = 0 means background color
    // *gs = 255 means foreground color
    // 0 <= *gs <= 255: interpolate between bg and fg
    //   r = bg_r + (fg_r - bg_r) * *gs / 255
    //   same for g and b
    const int d_r = (int)fg_r - (int)bg_r;
    const int d_g = (int)fg_g - (int)bg_g;
    const int d_b = (int)fg_b - (int)bg_b;

    const int8_t x_off = font.info[c].x_off;
    const int8_t y_off = font.info[c].y_off;
    const int8_t w = font.info[c].w;
    const int8_t h = font.info[c].h;
    const int8_t x_adv = font.info[c].x_adv;

    // fill box with background
    pix = pix_buf;
    for (int row = 0; row < font.y_adv; row++)
        for (int col = 0; col < x_adv; col++)
            *pix++ = bg;

    // put glyph in box (cropping edges)
    pix = pix_buf;
    for (int g_row = 0; g_row < h; g_row++) {
        for (int g_col = 0; g_col < w; g_col++) {
            int p_row = g_row + y_off;
            int p_col = g_col + x_off;
            if (p_row < 0 || p_row >= font.height())
                continue;
            if (p_col < 0 || p_col >= x_adv)
                continue;
            uint8_t gray = gs[g_row * w + g_col];
            uint8_t r = bg_r + d_r * gray / 255;
            uint8_t g = bg_g + d_g * gray / 255;
            uint8_t b = bg_b + d_b * gray / 255;
            pix[p_row * x_adv + p_col] = Pixel(r, g, b);
        }
    }

    // plop
    write(row, col, font.y_adv, font.info[c].x_adv, (Pixel *)_work);
}


// print string to screen
void Ws24::print(const Font& font, uint16_t row, uint16_t col,
                 Pixel fg, Pixel bg, const char *str)
{
    while (*str != '\0') {
        char c = *str++;
        print(font, row, col, fg, bg, c);
        col += font.width(c);
    }
}


// pulse hardware reset signal to controller
void Ws24::hw_reset()
{
    delay(150);
    digitalWrite(_gpio_reset, 0);
    delayMicroseconds(20);          // 10 usec min
    digitalWrite(_gpio_reset, 1);
    delay(150);                     // 120 msec min
}


// write command with no parameters to controller
void Ws24::write(uint8_t cmd)
{
    _spi.beginTransaction(_spi_settings);
    digitalWrite(_gpio_spi_cs, 0);

    digitalWrite(_gpio_dc, 0);
    _spi.transfer(cmd);

    digitalWrite(_gpio_spi_cs, 1);
    _spi.endTransaction();
}


// write command with one 8-bit parameter to controller
void Ws24::write(uint8_t cmd, uint8_t p1)
{
    _spi.beginTransaction(_spi_settings);
    digitalWrite(_gpio_spi_cs, 0);

    digitalWrite(_gpio_dc, 0);
    _spi.transfer(cmd);

    digitalWrite(_gpio_dc, 1);
    _spi.transfer(&p1, sizeof(p1));

    digitalWrite(_gpio_spi_cs, 1);
    _spi.endTransaction();
}


// write command with two 16-bit parameters to controller
void Ws24::write(uint8_t cmd, uint16_t p1, uint16_t p2)
{
    _spi.beginTransaction(_spi_settings);
    digitalWrite(_gpio_spi_cs, 0);

    digitalWrite(_gpio_dc, 0);
    _spi.transfer(cmd);

    uint8_t buf[4] = {
        uint8_t(p1 >> 8), uint8_t(p1), uint8_t(p2 >> 8), uint8_t(p2)
    };
    digitalWrite(_gpio_dc, 1);
    _spi.transfer(buf, sizeof(buf));

    digitalWrite(_gpio_spi_cs, 1);
    _spi.endTransaction();
}


// write command with arbitrary parameters to controller
void Ws24::write(uint8_t cmd, void *buf, int buf_len)
{
    // buf is overwritten

    _spi.beginTransaction(_spi_settings);
    digitalWrite(_gpio_spi_cs, 0);

    digitalWrite(_gpio_dc, 0);
    _spi.transfer(cmd);

    digitalWrite(_gpio_dc, 1);
    _spi.transfer(buf, buf_len);

    digitalWrite(_gpio_spi_cs, 1);
    _spi.endTransaction();
}


// initialize 16-bit to 18-bit color mapping in controller
bool Ws24::init_colors()
{
    if (_work_bytes < 128)
        return false;

    for (int red = 0; red < 32; red++)
        _work[red] = red << 1;

    for (int grn = 0; grn < 64; grn++)
        _work[32 + grn] = grn;

    for (int blu = 0; blu < 32; blu++)
        _work[96 + blu] = blu << 1;

    write(color_set, _work, 128);

    return true;
}
