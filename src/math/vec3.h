#ifndef VEC3_H
#define VEC3_H

#define MESH_SIZE
#define PI 3.14159265359

typedef struct vec3_t {
    float x, y, z;
} vec3_t;

typedef struct vec4_t {
    float x;
    float y;
    float z;
    float w;
} vec4_t;

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

// ---------------------------------------------- //
// ---------------------------------------------- //
// ---------------------------------------------- //

// typedef struct triangle_t {
//     vec3_t p[3];
// } triangle_t;

typedef struct mesh_t {
    int vertex_count;
    int triangle_count;

    // make this into an array of pointers to avoid vertex copying
    vec3_t *vertices;
} mesh_t;

#endif // !VEC3_H
