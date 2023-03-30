#pragma once

#include <stdint.h>

struct Font {
    int8_t y_adv;
    int8_t x_adv_max;
    int8_t x_off_min;
    int8_t x_off_max;
    int8_t y_off_min;
    int8_t y_off_max;
    struct {
        int32_t off;
        int8_t w;
        int8_t h;
        int8_t x_off;
        int8_t y_off;
        int8_t x_adv;
    } info[128];
    const uint8_t *data;

    int8_t height() const
    {
        return y_adv;
    }

    int8_t width(char c) const
    {
        return (c >= 0 && c < 128) ? info[c].x_adv : 0;
    }

    int8_t max_width() const
    {
        return x_adv_max;
    }
};
