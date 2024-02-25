#include "graphics_pipeline.h"
#include <math.h>

matrix_t generate_view_transform(camera_t *camera) {
    matrix_t M_v = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // matrix from:
    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bb281710(v=vs.85)

    vec3_t at = vec3_add(&camera->position, &camera->direction);

    vec3_t z_axis = vec3_sub(&camera->position, &at);
    z_axis = vec3_norm(&z_axis);

    vec3_t x_axis = vec3_cross(&camera->up, &z_axis);
    x_axis = vec3_norm(&x_axis);

    vec3_t y_axis = vec3_cross(&z_axis, &x_axis);

    M_v.m0 = x_axis.x;
    M_v.m1 = y_axis.x;
    M_v.m2 = z_axis.x;
    M_v.m12 = -vec3_dot(&x_axis, &camera->position);

    M_v.m4 = x_axis.y;
    M_v.m5 = y_axis.y;
    M_v.m6 = z_axis.y;
    M_v.m13 = -vec3_dot(&y_axis, &camera->position);

    M_v.m8 = x_axis.z;
    M_v.m9 = y_axis.z;
    M_v.m10 = z_axis.z;
    M_v.m14 = -vec3_dot(&z_axis, &camera->position);

    M_v.m15 = 1;

    /* ┌                                                           ┐ T
     * |       xaxis.x         yaxis.x         zaxis.x          0  |
     * |       xaxis.y         yaxis.y         zaxis.y          0  |
     * |       xaxis.z         yaxis.z         zaxis.z          0  |
     * |   -(xaxis . pos)  -(yaxis . pos)  -(zaxis . pos)       1  |
     * └                                                           ┘
     * */

    return M_v;
}

// view space (frustum) --> canonical view volume (ndc)
matrix_t
generate_projection_transform(float fovy, float aspect, float near, float far) {
    matrix_t M_proj = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    M_proj.m0 = 1 / (aspect * tan(fovy * 0.5));
    M_proj.m5 = 1 / (tan(fovy * 0.5));
    M_proj.m10 = -(far + near) / (far - near);     // changed
    M_proj.m11 = -1;                               // changed
    M_proj.m14 = -(2 * far * near) / (far - near); // changed

    /* ┌                                                                    ┐
     * | 1/(aspect*tan(fovy/2))        0               0            0       |
     * |           0             1/(tan(fovy/2))       0            0       |
     * |           0                   0         -(f+n)/(f-n) (-2fn)/(f-n)  |
     * |           0                   0              -1            0       |
     * └                                                                    ┘
     * */

    return M_proj;
}

// canonical view volume (ndc) --> viewport
matrix_t generate_viewport_transform(float width, float height) {
    matrix_t M_vp = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    M_vp.m0 = (width-1) / 2;
    M_vp.m12 = (width-1) / 2;
    M_vp.m5 = height / 2;
    M_vp.m13 = (height) / 2;
    M_vp.m10 = 1;
    M_vp.m15 = 1;

    /* ┌                                  ┐
     * | width/2    0       0    width/2  |
     * |    0    height/2   0    height/2 |
     * |    0       0       1       0     |
     * |    0       0       0       1     |
     * └                                  ┘
     * */

    return M_vp;
}

// To see why, go to https://en.wikipedia.org/wiki/Rotation_matrix
matrix_t
generate_rotation_matrix(float x_rotation, float y_rotation, float z_rotation) {
    float a = x_rotation;
    float b = y_rotation;
    float c = z_rotation;

    matrix_t R = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    R.m0 = cos(b) * cos(c);
    R.m1 = cos(b) * sin(c);
    R.m2 = -sin(b);
    R.m3 = 0;

    R.m4 = sin(a) * sin(b) * cos(c) - cos(a) * sin(c);
    R.m5 = sin(a) * sin(b) * sin(c) + cos(a) * cos(c);
    R.m6 = sin(a) * cos(b);
    R.m7 = 0;

    R.m8 = cos(a) * sin(b) * cos(c) + sin(a) * sin(c);
    R.m9 = cos(a) * sin(b) * sin(c) - sin(a) * cos(c);
    R.m10 = cos(a) * cos(b);
    R.m11 = 0;

    R.m12 = 0;
    R.m13 = 0;
    R.m14 = 0;
    R.m15 = 1;

    return R;
}

// To see why, go to https://en.wikipedia.org/wiki/Rotation_matrix
matrix_t rotation_matrix_from_axis(const vec3_t *axis, float angle) {
    vec3_t u = *axis;
    float cos_a = cos(angle);
    // printf("cos(%f) = %f\n", angle, cos_a);
    float sin_a = sin(angle);
    // printf("sin(%f) = %f\n", angle, sin_a);

    matrix_t R_u = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    R_u.m0 = cos_a + u.x * u.x * (1 - cos_a);
    R_u.m1 = u.y * u.x * (1 - cos_a) + u.z * sin_a;
    R_u.m2 = u.z * u.x * (1 - cos_a) - u.y * sin_a;
    R_u.m3 = 0;

    R_u.m4 = u.x * u.y * (1 - cos_a) - u.z * sin_a;
    R_u.m5 = cos_a + u.y * u.y * (1 - cos_a);
    R_u.m6 = u.z * u.y * (1 - cos_a) + u.x * sin_a;
    R_u.m7 = 0;

    R_u.m8 = u.x * u.z * (1 - cos_a) + u.y * sin_a;
    R_u.m9 = u.y * u.z * (1 - cos_a) - u.x * sin_a;
    R_u.m10 = cos_a + u.z * u.z * (1 - cos_a);
    R_u.m11 = 0;

    R_u.m12 = 0;
    R_u.m13 = 0;
    R_u.m14 = 0;
    R_u.m15 = 1;

    return R_u;
}
