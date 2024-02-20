#ifndef ENGINE_H
#define ENGINE_H

#include "./math/vec3.h"

#include <stdbool.h>

#define MAX_MESHES 10

#define CAMERA_SPEED_X 10
#define CAMERA_SPEED_Y 10
#define CAMERA_SPEED_Z 10

#define CAMERA_ROTATION_SPEED 100 * PI / 180;

typedef struct camera_t {
    vec3_t position;
    vec3_t direction;
    vec3_t up;

    vec3_t traslation_speed;
    vec3_t rotation_speed;
} camera_t;

typedef struct {
    float far;
    float near;
    float fovy;
    float aspect_ratio;
    camera_t *camera;

    matrix_t view_transform;
    matrix_t projection_transform;
    matrix_t viewport_transform;

    unsigned int mesh_count;
    mesh_t *meshes;

} engine_t;

void create_engine(engine_t *engine);
void destroy_engine(engine_t *engine);
void destroy_mesh(engine_t *engine, const int pos);

// Camera movement
void move_camera(engine_t *engine, float delta_time);
void rotate_camera(engine_t *engine, float delta_time);

#endif // !ENGINE_H
