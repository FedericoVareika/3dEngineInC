#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
} color_t;

uint32_t color_to_abgr(const color_t *color);
color_t mult_color(const color_t *color, float factor);
color_t add_colors(const color_t *a, const color_t *b);

#endif
