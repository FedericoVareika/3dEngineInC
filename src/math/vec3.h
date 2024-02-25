#ifndef VEC3_H
#define VEC3_H

#define MESH_SIZE
#define PI 3.14159265359f

typedef struct vec2_t {
    float x;
    float y;
} vec2_t;

typedef struct vec3_t {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct vec4_t {
    float x;
    float y;
    float z;
    float w;
} vec4_t;

typedef vec4_t quaternion_t;

typedef struct matrix_t {
    float m0, m4, m8, m12;  // Matrix first row (4 components)
    float m1, m5, m9, m13;  // Matrix second row (4 components)
    float m2, m6, m10, m14; // Matrix third row (4 components)
    float m3, m7, m11, m15; // Matrix fourth row (4 components)
} matrix_t;

vec3_t vec3_sub(const vec3_t *a, const vec3_t *b);
vec3_t vec3_add(const vec3_t *a, const vec3_t *b);
vec3_t vec3_mul(const vec3_t *a, const float factor);
vec3_t vec3_norm(const vec3_t *v);
float vec3_dot(const vec3_t *a, const vec3_t *b);
vec3_t vec3_cross(const vec3_t *a, const vec3_t *b);
float vec3_cross_2d(const vec3_t *a, const vec3_t *b);

void matrix_transformation(vec4_t *vec, const matrix_t *m);
vec3_t vec4_to_vec3(const vec4_t *v4);
vec4_t vec3_to_vec4(const vec3_t *v3);

float lerp(float start, float end, float t);

float vec4_dot(const vec4_t *A, const vec4_t *B);
float distance_to_plane(const vec4_t *plane, const vec3_t *point);
vec3_t intersection_plane_segment(const vec4_t *plane,
                                  const vec3_t *A,
                                  const vec3_t *B);

// ---------------------------------------------- //
// ---------------------------------------------- //
// ---------------------------------------------- //

#endif // !VEC3_H
