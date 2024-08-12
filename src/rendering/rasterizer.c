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

static float edge_cross_fast(const vec3_t ABC[3]) {
    return (ABC[2].x - ABC[0].x) * (ABC[1].y - ABC[0].y) -
           (ABC[2].y - ABC[0].y) * (ABC[1].x - ABC[0].x);
}

static float32x4_t duplicate_q_f32(const float32x4_t in) {
    float32x4_t result;
    result[0] = in[0];
    result[1] = in[1];
    result[2] = in[2];
    result[3] = in[3];
    return result;
}

#ifdef __ARM_NEON
static void fill_triangle_fast(uint32_t *frame_buffer, float *z_buffer,
                               vec3_t ABC[3], vec3_t ABC_uv[3],
                               const vec3_t *face_normal,
                               const vec3_t *directional_light,
                               const tex_t *tex) {
    bool has_tex = ABC_uv && tex;

    float area = edge_cross_fast(ABC);
    if (area < 0) {
        area = -area;

        vec3_t B_new = ABC[2];
        ABC[2] = ABC[1];
        ABC[1] = B_new;

        if (has_tex) {
            B_new = ABC_uv[2];
            ABC_uv[2] = ABC_uv[1];
            ABC_uv[1] = B_new;
        }
    }

    vec3_t A = ABC[0];
    vec3_t B = ABC[1];
    vec3_t C = ABC[2];

    float x_min = floorf(fminf(A.x, fminf(B.x, C.x)));
    float x_max = ceilf(fmaxf(A.x, fmaxf(B.x, C.x)));

    float y_min = floorf(fminf(A.y, fminf(B.y, C.y)));
    float y_max = ceilf(fmaxf(A.y, fmaxf(B.y, C.y)));

    float biasA = is_top_left(&B, &C) ? 0 : -0.0001f;
    float biasB = is_top_left(&C, &A) ? 0 : -0.0001f;
    float biasC = is_top_left(&A, &B) ? 0 : -0.0001f;

    vec3_t P = {x_min, y_min, 0};

    float wA_row;
    float wB_row;
    float wC_row;

    wA_row = edge_cross(&B, &C, &P) + biasA;
    wB_row = edge_cross(&C, &A, &P) + biasB;
    wC_row = edge_cross(&A, &B, &P) + biasC;

    float delta_wA_col = C.y - B.y;
    float delta_wB_col = A.y - C.y;
    float delta_wC_col = B.y - A.y;

    float delta_wA_row = B.x - C.x;
    float delta_wB_row = C.x - A.x;
    float delta_wC_row = A.x - B.x;

    // biasA, biasA, biasA, biasA,
    // biasB, biasB, biasB, biasB,
    // biasC, biasC, biasC, biasC
    uint32x4_t biasA4 = vdupq_n_f32(biasA);
    uint32x4_t biasB4 = vdupq_n_f32(biasB);
    uint32x4_t biasC4 = vdupq_n_f32(biasC);

    float lum = vec3_dot(face_normal, directional_light);
    lum = lum / 2 + 0.5;
    char grey = 0xFF * lum;

    uint32_t color = (int)grey << 24;
    color += (int)grey << 16;
    color += (int)grey << 8;
    color += (int)grey << 0;
    uint32x4_t color_vec = vdupq_n_u32(color);

    float32x4_t delta_wA_col_vec = vdupq_n_f32(delta_wA_col * 4);
    float32x4_t delta_wB_col_vec = vdupq_n_f32(delta_wB_col * 4);
    float32x4_t delta_wC_col_vec = vdupq_n_f32(delta_wC_col * 4);

    float32x4_t delta_wA_row_vec = vdupq_n_f32(delta_wA_row);
    float32x4_t delta_wB_row_vec = vdupq_n_f32(delta_wB_row);
    float32x4_t delta_wC_row_vec = vdupq_n_f32(delta_wC_row);

    float32x4_t wA_row_vec;
    float32x4_t wB_row_vec;
    float32x4_t wC_row_vec;
    {
        float32_t w_row[4] = {wA_row, wA_row + delta_wA_col,
                              wA_row + delta_wA_col * 2,
                              wA_row + delta_wA_col * 3};
        wA_row_vec = vld1q_f32(w_row);
        w_row[0] = wB_row;
        w_row[1] = wB_row + delta_wB_col;
        w_row[2] = wB_row + delta_wB_col * 2;
        w_row[3] = wB_row + delta_wB_col * 3;
        wB_row_vec = vld1q_f32(w_row);
        w_row[0] = wC_row;
        w_row[1] = wC_row + delta_wC_col;
        w_row[2] = wC_row + delta_wC_col * 2;
        w_row[3] = wC_row + delta_wC_col * 3;
        wC_row_vec = vld1q_f32(w_row);
    }

    float32x4_t zeros = vdupq_n_f32(0.0);
    float32x4_t ffs = vdupq_n_u32(0xFF);
    float inv_area = 1 / area;

    for (int y = y_min; y <= y_max; y++) {
        float32x4_t wA_vec = duplicate_q_f32(wA_row_vec);
        float32x4_t wB_vec = duplicate_q_f32(wB_row_vec);
        float32x4_t wC_vec = duplicate_q_f32(wC_row_vec);

        bool has_been_inside = false;
        for (int x = x_min; x <= x_max; x += 4) {
            float32x4_t b_coords_A = vsubq_f32(wA_vec, biasA4);
            float32x4_t b_coords_B = vsubq_f32(wB_vec, biasB4);
            float32x4_t b_coords_C = vsubq_f32(wC_vec, biasC4);

            b_coords_A = vmulq_n_f32(b_coords_A, inv_area);
            b_coords_B = vmulq_n_f32(b_coords_B, inv_area);
            b_coords_C = vmulq_n_f32(b_coords_C, inv_area);

            // ba * az + bb * bz + bc * cz
            float32x4_t z_vec = vmulq_n_f32(b_coords_A, A.z);
            z_vec = vfmaq_n_f32(z_vec, b_coords_B, B.z);
            z_vec = vfmaq_n_f32(z_vec, b_coords_C, C.z);

            uint32x4_t inside_triangle_vec =
                vandq_u32(vandq_u32(vcgtq_f32(b_coords_A, zeros),
                                    vcgtq_f32(b_coords_B, zeros)),
                          vcgtq_f32(b_coords_C, zeros));

            if (inside_triangle_vec[0] || inside_triangle_vec[1] ||
                inside_triangle_vec[2] || inside_triangle_vec[3]) {
                has_been_inside = true;
                float32x4_t z_buff_vec =
                    vld1q_f32(&z_buffer[SCREEN_WIDTH * y + x]);
                uint32x4_t pixel_priority_vec = vcltq_f32(z_vec, z_buff_vec);
                uint32x4_t fb_vec =
                    vld1q_u32(&frame_buffer[SCREEN_WIDTH * y + x]);

                if (has_tex) {
                    float32x4_t u_vec = vmulq_n_f32(b_coords_A, ABC_uv[0].x);
                    u_vec = vfmaq_n_f32(u_vec, b_coords_B, ABC_uv[1].x);
                    u_vec = vfmaq_n_f32(u_vec, b_coords_C, ABC_uv[2].x);
                    float32x4_t v_vec = vmulq_n_f32(b_coords_A, ABC_uv[0].y);
                    v_vec = vfmaq_n_f32(v_vec, b_coords_B, ABC_uv[1].y);
                    v_vec = vfmaq_n_f32(v_vec, b_coords_C, ABC_uv[2].y);
                    float32x4_t w_vec = vmulq_n_f32(b_coords_A, ABC_uv[0].z);
                    w_vec = vfmaq_n_f32(w_vec, b_coords_B, ABC_uv[1].z);
                    w_vec = vfmaq_n_f32(w_vec, b_coords_C, ABC_uv[2].z);

                    float32x4_t width4 = vdupq_n_f32(tex->w);
                    u_vec = vdivq_f32(u_vec, w_vec);
                    u_vec = vmulq_n_f32(u_vec, tex->w);
                    int32x4_t u_coord_vec = vcvtq_s32_f32(vrndmq_f32(u_vec));
                    u_coord_vec = vsubq_s32(
                        u_coord_vec, vmulq_n_s32(vcvtq_s32_f32(vrndmq_f32(
                                                     vdivq_f32(u_vec, width4))),
                                                 tex->w));

                    float32x4_t height4 = vdupq_n_f32(tex->h);
                    v_vec = vdivq_f32(v_vec, w_vec);
                    v_vec = vmulq_n_f32(v_vec, tex->h);
                    int32x4_t v_coord_vec = vcvtq_s32_f32(vrndmq_f32(v_vec));
                    v_coord_vec =
                        vsubq_s32(v_coord_vec,
                                  vmulq_n_s32(vcvtq_s32_f32(vrndmq_f32(
                                                  vdivq_f32(v_vec, height4))),
                                              tex->h));

                    int32x4_t tex_coord_vec = vmulq_n_f32(
                        vmlaq_n_s32(u_coord_vec, v_coord_vec, tex->w), 4);

#ifdef LITTLE_ENDIAN

                    int idx0 = tex_coord_vec[0];
                    int idx1 = tex_coord_vec[1];
                    int idx2 = tex_coord_vec[2];
                    int idx3 = tex_coord_vec[3];

                    uint8x8_t rgba_1_2 = vld1_u8(&tex->data[idx0]);
                    rgba_1_2[4] = tex->data[idx1 + 0];
                    rgba_1_2[5] = tex->data[idx1 + 1];
                    rgba_1_2[6] = tex->data[idx1 + 2];
                    rgba_1_2[7] = tex->data[idx1 + 3];

                    float16x8_t rgba_1_2f = vcvtq_f16_u16(vmovl_u8(rgba_1_2));
                    rgba_1_2f = vmulq_n_f16(rgba_1_2f, lum);
                    rgba_1_2 = vmovn_u16(vcvtq_u16_f16(rgba_1_2f));

                    uint8x8_t rgba_3_4 = vld1_u8(&tex->data[idx2]);
                    rgba_3_4[4] = tex->data[idx3 + 0];
                    rgba_3_4[5] = tex->data[idx3 + 1];
                    rgba_3_4[6] = tex->data[idx3 + 2];
                    rgba_3_4[7] = tex->data[idx3 + 3];

                    float16x8_t rgba_3_4f = vcvtq_f16_u16(vmovl_u8(rgba_3_4));
                    rgba_3_4f = vmulq_n_f16(rgba_3_4f, lum);
                    rgba_3_4 = vmovn_u16(vcvtq_u16_f16(rgba_3_4f));

                    uint8x16_t rgba_vec_8 = vcombine_u8(rgba_1_2, rgba_3_4);

                    rgba_vec_8 = vrev32q_u8(rgba_vec_8);

                    /* unsigned char rgba_arr_full[16]; */
                    /* rgba_arr_full[15] = tex->data[idx3 + 0]; */
                    /* rgba_arr_full[14] = tex->data[idx3 + 1]; */
                    /* rgba_arr_full[13] = tex->data[idx3 + 2]; */
                    /* rgba_arr_full[12] = tex->data[idx3 + 3]; */
                    /* rgba_arr_full[11] = tex->data[idx2 + 0]; */
                    /* rgba_arr_full[10] = tex->data[idx2 + 1]; */
                    /* rgba_arr_full[9] = tex->data[idx2 + 2]; */
                    /* rgba_arr_full[8] = tex->data[idx2 + 3]; */
                    /* rgba_arr_full[7] = tex->data[idx1 + 0]; */
                    /* rgba_arr_full[6] = tex->data[idx1 + 1]; */
                    /* rgba_arr_full[5] = tex->data[idx1 + 2]; */
                    /* rgba_arr_full[4] = tex->data[idx1 + 3]; */
                    /* rgba_arr_full[3] = tex->data[idx0 + 0]; */
                    /* rgba_arr_full[2] = tex->data[idx0 + 1]; */
                    /* rgba_arr_full[1] = tex->data[idx0 + 2]; */
                    /* rgba_arr_full[0] = tex->data[idx0 + 3]; */

                    /* rgba_vec_8 = vld1q_u8(rgba_arr_full); */
                    /* rgba_vec_32_test = vreinterpretq_u32_u8(rgba_vec_8); */
                    /* printf("%x, %x, %x, %x\n", rgba_vec_32_test[0], */
                    /*        rgba_vec_32_test[1], rgba_vec_32_test[2], */
                    /*        rgba_vec_32_test[3]); */
#endif // little endian

                    uint32x4_t rgba_vec_32;
                    rgba_vec_32 = vreinterpretq_u32_u8(rgba_vec_8);
                    /* rgba_vec_32 = vrev64q_u32(rgba_vec_32); */
                    color_vec = rgba_vec_32;
                }

                uint32x4_t is_transparent =
                    vcgtq_u32(vandq_u32(color_vec, ffs), zeros);

                uint32x4_t mask =
                    vandq_u32(inside_triangle_vec, pixel_priority_vec);

                mask = vandq_u32(mask, is_transparent);

                uint32x4_t new_fb_vec = vbslq_u32(mask, color_vec, fb_vec);
                z_buff_vec = vbslq_u32(mask, z_vec, z_buff_vec);

                vst1q_u32(&frame_buffer[SCREEN_WIDTH * y + x], new_fb_vec);
                vst1q_f32(&z_buffer[SCREEN_WIDTH * y + x], z_buff_vec);
            } else if (has_been_inside) {
                x = x_max + 1;
            }

            wA_vec = vaddq_f32(wA_vec, delta_wA_col_vec);
            wB_vec = vaddq_f32(wB_vec, delta_wB_col_vec);
            wC_vec = vaddq_f32(wC_vec, delta_wC_col_vec);
        }
        wA_row_vec = vaddq_f32(wA_row_vec, delta_wA_row_vec);
        wB_row_vec = vaddq_f32(wB_row_vec, delta_wB_row_vec);
        wC_row_vec = vaddq_f32(wC_row_vec, delta_wC_row_vec);
    }
}
#else

static void fill_triangle(uint32_t *frame_buffer, float *z_buffer,
                          const vec3_t *A, const vec3_t *A_uv, const vec3_t *B,
                          const vec3_t *B_uv, const vec3_t *C,
                          const vec3_t *C_uv, const vec3_t *face_normal,
                          const vec3_t *directional_light, const tex_t *tex) {
    bool has_tex = A_uv != NULL && B_uv != NULL && C_uv != NULL;
    has_tex = has_tex && tex;

    bool clockwise = false;
    // Area of parallelogram
    float area = edge_cross(A, B, C);
    if (area < 0) clockwise = true;

    if (clockwise) {
        const vec3_t *B_new = C;
        const vec3_t *C_new = B;
        B = B_new;
        C = C_new;

        area = edge_cross(A, B, C);

        if (has_tex) {
            B_new = C_uv;
            C_new = B_uv;
            B_uv = B_new;
            C_uv = C_new;
        }
    }

    float x_min = floorf(fminf(A->x, fminf(B->x, C->x)));
    float x_max = ceilf(fmaxf(A->x, fmaxf(B->x, C->x)));

    float y_min = floorf(fminf(A->y, fminf(B->y, C->y)));
    float y_max = ceilf(fmaxf(A->y, fmaxf(B->y, C->y)));

    float biasA = is_top_left(B, C) ? 0 : -0.0001f;
    float biasB = is_top_left(C, A) ? 0 : -0.0001f;
    float biasC = is_top_left(A, B) ? 0 : -0.0001f;

    vec3_t P = {x_min, y_min, 0};

    float wA_row;
    float wB_row;
    float wC_row;

    wA_row = edge_cross(B, C, &P) + biasA;
    wB_row = edge_cross(C, A, &P) + biasB;
    wC_row = edge_cross(A, B, &P) + biasC;

    float delta_wA_col = C->y - B->y;
    float delta_wB_col = A->y - C->y;
    float delta_wC_col = B->y - A->y;

    float delta_wA_row = B->x - C->x;
    float delta_wB_row = C->x - A->x;
    float delta_wC_row = A->x - B->x;

    // TODO: Apply good simd here (plan it)
    for (int y = y_min; y <= y_max; y++) {
        float wA = wA_row;
        float wB = wB_row;
        float wC = wC_row;

        bool has_been_inside = false;

        for (int x = x_min; x <= x_max; x++) {
            bool inside_triangle = wA >= 0 && wB >= 0 && wC >= 0;

            vec3_t b_coords;
            float z;
            bool has_pixel_priority = false;
            if (inside_triangle) {
                b_coords = (vec3_t){(wA - biasA) / area, (wB - biasB) / area,
                                    (wC - biasC) / area};
                // use this z coord when/if i change to fixed point
                /* z = b_coords.x / A->z + b_coords.y / B->z + b_coords.z /
                 * C->z; */
                /* z = 1 / z; */

                z = b_coords.x * A->z + b_coords.y * B->z + b_coords.z * C->z;

                has_pixel_priority = pixel_priority(z_buffer, x, y, z);
            }

            if (inside_triangle && has_pixel_priority) {
                has_been_inside = true;
                float lum = vec3_dot(face_normal, directional_light);
                lum = lum / 2 + 0.5;

                uint32_t color;
                uint32_t a, r, g, b;

                if (has_tex) {
                    float u = b_coords.x * A_uv->x + b_coords.y * B_uv->x +
                              b_coords.z * C_uv->x;
                    float v = b_coords.x * A_uv->y + b_coords.y * B_uv->y +
                              b_coords.z * C_uv->y;
                    float w = b_coords.x * A_uv->z + b_coords.y * B_uv->z +
                              b_coords.z * C_uv->z;

                    float w_inv = 1 / w;
                    int u_coord = tex->w * u * w_inv;
                    u_coord = u_coord - (tex->w * (u_coord / tex->w));
                    /* int u_coord = (int)(tex->w * u * w_inv) % tex->w; */
                    /* u_coord = u_coord - (tex->w * (u_coord / tex->w)); */

                    /* u_coord = u_coord >= 0 ? u_coord : u_coord + tex->w; */

                    v = tex->h * v * w_inv;
                    int v_coord = (int)floorf(v);
                    v_coord = v_coord - (tex->h * (int)floorf(v / tex->h));
                    /* int v_coord = (int)(tex->h * v * w_inv) % tex->h; */
                    /* v_coord = v_coord >= 0 ? v_coord : v_coord + tex->h; */

                    int tex_cord_pos = (v_coord * tex->w + u_coord) * 4;
                    r = tex->data[tex_cord_pos + 0];
                    g = tex->data[tex_cord_pos + 1];
                    b = tex->data[tex_cord_pos + 2];
                    a = tex->data[tex_cord_pos + 3];
                } else {
                    color = (uint32_t)0xFFFFFFFF;
                    /* color = 0xCCCCCCFF; */
                    a = 0xFF & (color >> 24);
                    r = 0xFF & (color >> 16);
                    g = 0xFF & (color >> 8);
                    b = 0xFF & (color >> 0);
                }

                r = (int)(r * lum) << 24;
                g = (int)(g * lum) << 16;
                b = (int)(b * lum) << 8;
                a = (int)(a * lum) << 0;

                color = r + g + b + a;

                if (a != 0) {
                    frame_buffer[SCREEN_WIDTH * y + x] = color;
                    z_buffer[SCREEN_WIDTH * y + x] = z;
                }
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
#endif // arm neon

// TODO: Change to bresenham's if too slow
void draw_line(bool *wireframe_buffer, const vec3_t *A, const vec3_t *B) {
    float dx = B->x - A->x;
    float dy = B->y - A->y;

    float step = fmaxf(fabsf(dx), fabsf(dy));

    if (step == 0) { return; }

    float x_step = dx / step;
    float y_step = dy / step;

    if ((x_step == 0 && y_step == 0) || isnan(x_step) || isnan(y_step)) {
        return;
    }

    float x = A->x;
    float y = A->y;

    bool after_x = false;
    bool after_y = false;

    while (!after_x && !after_y) {
        int y_int = (y > (floorf(y) + 0.5f)) ? ceilf(y) : floorf(y);
        int x_int = (x > (floorf(x) + 0.5f)) ? ceilf(x) : floorf(x);
        wireframe_buffer[WINDOW_WIDTH * y_int + x_int] = true;
        x += x_step;
        y += y_step;

        // checks if the delta is negative, and if so, it should check if the
        // value has surpassed its limit from the other side
        after_x = dx < 0 ? x < B->x : x > B->x;
        after_y = dy < 0 ? y < B->y : y > B->y;
    }
}

void triangle_wireframe(bool *wireframe_buffer, const vec3_t *A,
                        const vec3_t *B, const vec3_t *C) {
    draw_line(wireframe_buffer, A, B);
    draw_line(wireframe_buffer, B, C);
    draw_line(wireframe_buffer, C, A);
}

void draw_triangle(state_t *state, const vec3_t A, const vec3_t *A_uv,
                   const vec3_t B, const vec3_t *B_uv, const vec3_t C,
                   const vec3_t *C_uv, const vec3_t face_normal,
                   const tex_t *tex) {
    switch (state->flags.render_flag) {
    case FRAME_BUFFER:
    case Z_BUFFER:
#ifdef __ARM_NEON
        fill_triangle_fast(
            state->buffers.frame_buffer, state->buffers.z_buffer,
            (vec3_t[3]){A, B, C},
            A_uv && B_uv && C_uv ? (vec3_t[3]){*A_uv, *B_uv, *C_uv} : NULL,
            &face_normal, &state->engine->directional_light, tex);
#else
        fill_triangle(state->buffers.frame_buffer, state->buffers.z_buffer, &A,
                      A_uv, &B, B_uv, &C, C_uv, &face_normal,
                      &state->engine->directional_light, tex);
#endif
        break;

    case WIREFRAME:
        triangle_wireframe(state->buffers.wireframe_buffer, &A, &B, &C);
        break;
    }
}
