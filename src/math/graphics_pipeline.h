#ifndef GRAPHICS_PIPELINE_H
#define GRAPHICS_PIPELINE_H

#include "../engine.h"
#include "../math/vec3.h"

// Transform matrix that "moves" points from world view to camera view
matrix_t generate_view_transform(camera_t *camera);

// frustum (view volume) -> canonical view volume
matrix_t
generate_projection_transform(float fovy, float aspect, float near, float far);

// Transform matrix that converts canonical view volume to viewport
// (screen space)
matrix_t generate_viewport_transform(float width, float height);

matrix_t
generate_rotation_matrix(float x_rotation, float y_rotation, float z_rotation);

matrix_t rotation_matrix_from_axis(const vec3_t *axis, float angle);

#endif // !GRAPHICS_PIPELINE_H
