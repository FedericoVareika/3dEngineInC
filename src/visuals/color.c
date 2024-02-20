#include "color.h"

uint32_t color_to_abgr(const color_t *color) {
    return 0xFF000000 | (color->b << 16) | (color->g << 8) | color->r;
}

color_t mult_color(const color_t *color, float factor) {
    color_t new_color = {
        .r = color->r * factor, .g = color->g * factor, .b = color->b * factor};
    return new_color;
}

color_t add_colors(const color_t *a, const color_t *b) {
    return (color_t){a->r + b->r, a->g + b->g, a->b + b->b};
}
