#ifndef ENGINE_H
#define ENGINE_H

#include "./math/vec3.h"

#include <stdint.h>
#include <stdbool.h>

#define MAX_MESHES 100

#define CAMERA_SPEED_X 10
#define CAMERA_SPEED_Y 10
#define CAMERA_SPEED_Z 10

#define CAMERA_ROTATION_SPEED 100 * PI / 180;

#define CLIPPING_PLANES 6

typedef struct tex_t {
    uint8_t *data;
    unsigned int n;
    unsigned int w;
    unsigned int h;
} tex_t;

typedef struct mesh_t {
    int vertex_count;
    int triangle_count;

    vec3_t *vertices;
    vec3_t *tex_coords;
    vec3_t *normals;

    unsigned int *v_indices;
    unsigned int *t_indices;
    unsigned int *n_indices;

    tex_t tex;
} mesh_t;

typedef struct model_t {
    matrix_t transform;
    int mesh_count;

    mesh_t *meshes;
} model_t;

typedef struct camera_t {
    vec3_t position;
    vec3_t direction;
    vec3_t up;

    vec3_t traslation_speed;
    vec3_t rotation_speed;
} camera_t;

typedef struct engine_t {
    float far;
    float near;
    float fovy;
    float aspect_ratio;

    // Calculated at runtime
    float top;
    float left;
    float bottom;
    float right;

    // frustum clipping planes
    // Normal.Point = distance
    vec4_t clipping_planes[CLIPPING_PLANES];

    camera_t *camera;

    matrix_t view_transform;
    matrix_t projection_transform;
    matrix_t viewport_transform;

    unsigned int mesh_count;
    // model_t *models;
    mesh_t **meshes;

    vec3_t directional_light; 

} engine_t;

void create_engine(engine_t *engine);
void destroy_engine(engine_t *engine);
void destroy_mesh(engine_t *engine, const int pos);

// Camera movement
void move_camera(engine_t *engine, float delta_time);
void rotate_camera(engine_t *engine, float delta_time);

#endif // !ENGINE_H
