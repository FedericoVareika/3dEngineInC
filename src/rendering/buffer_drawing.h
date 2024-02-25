#ifndef BUFFER_DRAWING_H
#define BUFFER_DRAWING_H

#include "../state.h"

void cllp_and_draw(state_t *state,
                   const vec3_t *A,
                   const vec3_t *B,
                   const vec3_t *C,
                   const int plane_id);
void draw_meshes(state_t *state);

#endif // !BUFFER_DRAWING_H
