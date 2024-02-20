#include "rasterizer.h"
#include <math.h>

static bool is_top_left(vec3_t *start, vec3_t *end) {
    vec3_t edge = {end->x - start->x, end->y - start->y};
    return (edge.y == 0 && edge.x > 0) || edge.y > 0;
}
// --------------------------------------------------------------------------//

static vec3_t barycentric_coords(
    float x, float y, const vec3_t *A, const vec3_t *B, const vec3_t *C) {

    // Calculate all the parallelogram areas for two points and a
    float bcp_area = (x - B->x) * (C->y - B->y) - (y - B->y) * (C->x - B->x);
    float cap_area = (x - C->x) * (A->y - C->y) - (y - C->y) * (A->x - C->x);
    float abp_area = (x - A->x) * (B->y - A->y) - (y - A->y) * (B->x - A->x);

    float area = abp_area + bcp_area + cap_area;
    return (vec3_t){
        .x = bcp_area / area,
        .y = cap_area / area,
        .z = abp_area / area,
    };
}

static void
fill_triangle(uint32_t *frame_buffer, float *z_buffer, vec3_t *vertices) {
    vec3_t A = vertices[0];
    vec3_t B = vertices[1];
    vec3_t C = vertices[2];

    int x_min = floor(fmin(A.x, fmin(B.x, C.x)));
    int x_max = ceil(fmax(A.x, fmax(B.x, C.x)));

    int y_min = floor(fmin(A.y, fmin(B.y, C.y)));
    int y_max = ceil(fmax(A.y, fmax(B.y, C.y)));

    float biasA = is_top_left(&B, &C) ? 0 : -0.0001f;
    float biasB = is_top_left(&B, &C) ? 0 : -0.0001f;
    float biasC = is_top_left(&B, &C) ? 0 : -0.0001f;

    // Area of parallelogram
    float area = (C.x - A.x) * (B.y - A.y) - (C.y - A.y) * (B.x - A.x);

    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            vec3_t b_coords = barycentric_coords(x, y, &A, &B, &C);

            bool inside_triangle = b_coords.x * area + biasA >= 0 &&
                                   b_coords.y * area + biasB >= 0 &&
                                   b_coords.z * area + biasC >= 0;

            float z = b_coords.x * A.z + b_coords.y * B.z + b_coords.z * C.z;
            bool has_pixel_priority = pixel_priority(z_buffer, x, y, z);
            if (inside_triangle && has_pixel_priority) {

                uint32_t r = (uint32_t)(0xFF * b_coords.x) << 16;
                uint32_t g = (uint32_t)(0xFF * b_coords.y) << 8;
                uint32_t b = (uint32_t)(0xFF * b_coords.z) << 0;
                uint32_t color = 0xFF000000 + r + g + b;

                frame_buffer[SCREEN_WIDTH * y + x] = color;
                z_buffer[SCREEN_WIDTH * y + x] = z;
            }
        }
    }
}

// TODO: Change to bresenham's if too slow
void draw_line(bool *wireframe_buffer, vec3_t *A, vec3_t *B) {
    float dx = B->x - A->x;
    float dy = B->y - A->y;

    float step = fmax(fabs(dx), fabs(dy));

    float x_step = dx / step;
    float y_step = dy / step;

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

void triangle_wireframe(bool *wireframe_buffer, vec3_t *vertices) {
    vec3_t A = vertices[0];
    vec3_t B = vertices[1];
    vec3_t C = vertices[2];

    draw_line(wireframe_buffer, &A, &B);
    draw_line(wireframe_buffer, &B, &C);
    draw_line(wireframe_buffer, &C, &A);
}

void draw_mesh(state_t *state, const mesh_t *mesh) {
    for (int i = 0; i < mesh->triangle_count; i++) {
        fill_triangle(state->buffers.frame_buffer,
                      state->buffers.z_buffer,
                      &mesh->vertices[i * 3]);
        triangle_wireframe(state->buffers.wireframe_buffer,
                           &mesh->vertices[i * 3]);
    }
}
