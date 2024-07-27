#ifndef RASTERIZER_H
#define RASTERIZER_H

#include "../math/vec3.h"
#include "../state.h"

void draw_triangle(state_t *state,
                   const vec3_t A,
                   const vec3_t *A_uv,
                   const vec3_t B,
                   const vec3_t *B_uv,
                   const vec3_t C,
                   const vec3_t *C_uv,
                   const vec3_t face_normal,
                   const tex_t *tex);
void draw_mesh(state_t *state, const mesh_t *mesh);

#endif
