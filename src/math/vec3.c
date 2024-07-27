#include "vec3.h"
#include <math.h>
#include <stdio.h>

vec3_t vec3_sub(const vec3_t *a, const vec3_t *b) {
    // return (vec3_t){a->x - b->x, a->y - b->y, a->z - b->z, a->color};
    return (vec3_t){a->x - b->x, a->y - b->y, a->z - b->z};
}

vec2_t vec2_sub(const vec2_t *a, const vec2_t *b) {
    // return (vec3_t){a->x - b->x, a->y - b->y, a->z - b->z, a->color};
    return (vec2_t){a->x - b->x, a->y - b->y};
}

vec3_t vec3_add(const vec3_t *a, const vec3_t *b) {
    return (vec3_t){a->x + b->x, a->y + b->y, a->z + b->z};
}

vec2_t vec2_add(const vec2_t *a, const vec2_t *b) {
    return (vec2_t){a->x + b->x, a->y + b->y};
}

vec3_t vec3_mul(const vec3_t *a, const float factor) {
    return (vec3_t){a->x * factor, a->y * factor, a->z * factor};
}

vec2_t vec2_mul(const vec2_t *a, const float factor) {
    return (vec2_t){a->x * factor, a->y * factor};
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

float vec4_dot(const vec4_t *A, const vec4_t *B) {
    return A->x * B->x + A->y * B->y + A->z * B->z + A->w * B->w;
}

float lerp(float start, float end, float t) {
    return start * (1.0f - t) + end * t;
}

float distance_to_plane(const vec4_t *plane, const vec3_t *point) {
    // Plane = (N_x, N_y, N_z, d), N being the normal to the plane and d the
    // distance to (0, 0, 0)
    // This is the parametric equation of a plane or:
    // A.x + B.y + C.z = d --- where (A, B, C) = N
    vec4_t point_4 = vec3_to_vec4(point);
    return vec4_dot(plane, &point_4);
}

vec3_t intersection_plane_segment(const vec4_t *plane,
                                  const vec3_t *A,
                                  const vec3_t *A_uv,
                                  const vec3_t *B,
                                  const vec3_t *B_uv,
                                  vec3_t *new_uv) {
    // Plane = (N_x, N_y, N_z, d), N being the normal to the plane and d the
    // distance to (0, 0, 0)
    // This is the parametric equation of a plane or:
    // A.x + B.y + C.z = d --- where (A, B, C) = N
    // (N.P) = d

    // P_intersection = A + t(A - B)  --- Parametric equation of the line AB
    // t = (d - (N.A)) / (N.(B-A))   --- Found with both equations

    vec3_t N = {plane->x, plane->y, plane->z};
    float d = plane->w;

    vec3_t AB = vec3_sub(B, A);

    float t = (d - vec3_dot(&N, A)) / (vec3_dot(&N, &AB));
    vec3_t tAB = vec3_mul(&AB, t);

    if (A_uv != NULL && B_uv != NULL) {
        vec3_t AB_uv = vec3_sub(B_uv, A_uv);
        vec3_t tAB_uv = vec3_mul(&AB_uv, t);
        *new_uv = vec3_add(A_uv, &tAB_uv);
    }

    return vec3_add(A, &tAB);
}
