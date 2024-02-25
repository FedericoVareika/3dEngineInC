#include "rasterizer.h"
#include <math.h>

static bool is_top_left(const vec3_t *start, const vec3_t *end) {
    vec3_t edge = {end->x - start->x, end->y - start->y};
    return (edge.y == 0 && edge.x > 0) || edge.y > 0;
}

static float edge_cross(const vec3_t *A, const vec3_t *B, const vec3_t *C) {
    return (C->x - A->x) * (B->y - A->y) - (C->y - A->y) * (B->x - A->x);
}

// --------------------------------------------------------------------------//

static void fill_triangle(uint32_t *frame_buffer,
                          float *z_buffer,
                          const vec3_t *A,
                          const vec3_t *B,
                          const vec3_t *C,
                          const vec3_t *face_normal,
                          const vec3_t *directional_light) {
    float x_min = floor(fmin(A->x, fmin(B->x, C->x)));
    float x_max = ceil(fmax(A->x, fmax(B->x, C->x)));

    float y_min = floor(fmin(A->y, fmin(B->y, C->y)));
    float y_max = ceil(fmax(A->y, fmax(B->y, C->y)));

    float biasA = is_top_left(B, C) ? 0 : -0.0001f;
    float biasB = is_top_left(C, A) ? 0 : -0.0001f;
    float biasC = is_top_left(A, B) ? 0 : -0.0001f;

    // Area of parallelogram
    float area = edge_cross(A, B, C);

    vec3_t P = {x_min, y_min, 0};

    float wA_row = edge_cross(B, C, &P) + biasA;
    float wB_row = edge_cross(C, A, &P) + biasB;
    float wC_row = edge_cross(A, B, &P) + biasC;

    float delta_wA_col = C->y - B->y;
    float delta_wB_col = A->y - C->y;
    float delta_wC_col = B->y - A->y;

    float delta_wA_row = B->x - C->x;
    float delta_wB_row = C->x - A->x;
    float delta_wC_row = A->x - B->x;

    for (int y = y_min; y <= y_max; y++) {
        float wA = wA_row;
        float wB = wB_row;
        float wC = wC_row;
        // printf("wa: %f, wb: %f, wc: %f\n", wA, wB, wC);

        bool has_been_inside = false;

        for (int x = x_min; x <= x_max; x++) {
            vec3_t b_coords = {
                (wA - biasA) / area, (wB - biasB) / area, (wC - biasC) / area};

            bool inside_triangle = wA >= 0 && wB >= 0 && wC >= 0;

            float z = b_coords.x * A->z + b_coords.y * B->z + b_coords.z * C->z;
            bool has_pixel_priority = pixel_priority(z_buffer, x, y, z);
            if (inside_triangle && has_pixel_priority) {
                has_been_inside = true;
                float lum = vec3_dot(face_normal, directional_light);
                lum = lum / 2 + 0.5;

                uint32_t a = (uint32_t)(0xFF) << 24;
                uint32_t r = (uint32_t)(0xFF * lum) << 16; // * b_coords.x
                uint32_t g = (uint32_t)(0xFF * lum) << 8;  // * b_coords.y
                uint32_t b = (uint32_t)(0xFF * lum) << 0;  // * b_coords.z
                uint32_t color = a + r + g + b;
                /* color = 0xFFCCCCCC; */

                frame_buffer[SCREEN_WIDTH * y + x] = color;
                z_buffer[SCREEN_WIDTH * y + x] = z;
            } else if (has_been_inside && !inside_triangle) {
                x = x_max + 1;
            }

            wA += delta_wA_col;
            wB += delta_wB_col;
            wC += delta_wC_col;
        }
        wA_row += delta_wA_row;
        wB_row += delta_wB_row;
        wC_row += delta_wC_row;
    }
}

// TODO: Change to bresenham's if too slow
void draw_line(bool *wireframe_buffer, const vec3_t *A, const vec3_t *B) {
    float dx = B->x - A->x;
    float dy = B->y - A->y;

    float step = fmax(fabs(dx), fabs(dy));

    if (step == 0) {
        return;
    }

    float x_step = dx / step;
    float y_step = dy / step;

    if ((x_step == 0 && y_step == 0) || isnan(x_step) || isnan(y_step)) {
        return;
    }

    // printf("x_step: %f, y_step: %f\n", x_step, y_step);

    float x = A->x;
    float y = A->y;

    bool after_x = false;
    bool after_y = false;

    while (!after_x && !after_y) {
        int y_int = (y > (floor(y) + 0.5f)) ? ceil(y) : floor(y);
        int x_int = (x > (floor(x) + 0.5f)) ? ceil(x) : floor(x);
        wireframe_buffer[WINDOW_WIDTH * y_int + x_int] = true;
        x += x_step;
        y += y_step;

        // checks if the delta is negative, and if so, it should check if the
        // value has surpassed its limit from the other side
        after_x = dx < 0 ? x < B->x : x > B->x;
        after_y = dy < 0 ? y < B->y : y > B->y;
    }
}

void triangle_wireframe(bool *wireframe_buffer,
                        const vec3_t *A,
                        const vec3_t *B,
                        const vec3_t *C) {
    draw_line(wireframe_buffer, A, B);
    draw_line(wireframe_buffer, B, C);
    draw_line(wireframe_buffer, C, A);
}

void draw_triangle(state_t *state,
                   const vec3_t A,
                   const vec3_t B,
                   const vec3_t C,
                   const vec3_t face_normal) {
    fill_triangle(state->buffers.frame_buffer,
                  state->buffers.z_buffer,
                  &A,
                  &B,
                  &C,
                  &face_normal,
                  &state->engine->directional_light);
    triangle_wireframe(state->buffers.wireframe_buffer, &A, &B, &C);
}
