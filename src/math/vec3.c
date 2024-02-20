#include "vec3.h"
#include <math.h>

vec3_t vec3_sub(const vec3_t *a, const vec3_t *b) {
    // return (vec3_t){a->x - b->x, a->y - b->y, a->z - b->z, a->color};
    return (vec3_t){a->x - b->x, a->y - b->y, a->z - b->z};
}

vec3_t vec3_add(const vec3_t *a, const vec3_t *b) {
    return (vec3_t){a->x + b->x, a->y + b->y, a->z + b->z};
}

vec3_t vec3_mul(const vec3_t *a, const float factor) {
    return (vec3_t){a->x * factor, a->y * factor, a->z * factor};
}

vec3_t vec3_norm(const vec3_t *v) {
    float magnitude = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    return (vec3_t){
        .x = v->x / magnitude,
        .y = v->y / magnitude,
        .z = v->z / magnitude,
    };
}

vec3_t vec3_cross(const vec3_t *a, const vec3_t *b) {
    return (vec3_t){
        .x = a->y * b->z - a->z * b->y,
        .y = a->z * b->x - a->x * b->z,
        .z = a->x * b->y - a->y * b->x,
    };
}

float vec3_dot(const vec3_t *a, const vec3_t *b) {
    return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

float vec3_cross_2d(const vec3_t *a, const vec3_t *b) {
    return a->x * b->y - a->y * b->x;
}

void matrix_transformation(vec4_t *vec, const matrix_t *m) {
    float x = vec->x;
    float y = vec->y;
    float z = vec->z;
    float w = vec->w;

    // transformed vec
    vec->x = m->m0 * x + m->m4 * y + m->m8 * z + m->m12 * w;
    vec->y = m->m1 * x + m->m5 * y + m->m9 * z + m->m13 * w;
    vec->z = m->m2 * x + m->m6 * y + m->m10 * z + m->m14 * w;
    vec->w = m->m3 * x + m->m7 * y + m->m11 * z + m->m15 * w;
}

vec3_t vec4_to_vec3(const vec4_t *v4) {
    return (vec3_t){
        // divide by w because of homogeneous coords
        .x = v4->x / v4->w,
        .y = v4->y / v4->w,
        .z = v4->z / v4->w,
    };
}

vec4_t vec3_to_vec4(const vec3_t *v3) {
    return (vec4_t){
        .x = v3->x,
        .y = v3->y,
        .z = v3->z,
        .w = 1,
    };
}

float lerp(float start, float end, float t) {
    return start * (1.0f - t) + end * t;
}
