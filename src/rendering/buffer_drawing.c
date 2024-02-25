#include "buffer_drawing.h"
#include "../math/graphics_pipeline.h"
#include "rasterizer.h"

void project_and_draw(state_t *state,
                      const vec3_t *A,
                      const vec3_t *B,
                      const vec3_t *C, 
                      const vec3_t *face_normal) {
    vec4_t A_4 = vec3_to_vec4(A);
    vec4_t B_4 = vec3_to_vec4(B);
    vec4_t C_4 = vec3_to_vec4(C);

    // ------------------- Projection Transform -------------------- //
    matrix_transformation(&A_4, &state->engine->projection_transform);
    matrix_transformation(&B_4, &state->engine->projection_transform);
    matrix_transformation(&C_4, &state->engine->projection_transform);

    vec3_t A_proj = vec4_to_vec3(&A_4);
    vec3_t B_proj = vec4_to_vec3(&B_4);
    vec3_t C_proj = vec4_to_vec3(&C_4);

    float max_x = fmax(A_proj.x, fmax(B_proj.x, C_proj.x));
    float max_y = fmax(A_proj.y, fmax(B_proj.y, C_proj.y));
    float max_z = fmax(A_proj.z, fmax(B_proj.z, C_proj.z));

    float min_x = fmin(A_proj.x, fmin(B_proj.x, C_proj.x));
    float min_y = fmin(A_proj.y, fmin(B_proj.y, C_proj.y));
    float min_z = fmin(A_proj.z, fmin(B_proj.z, C_proj.z));

    if (fmax(max_x, fmax(max_y, max_z)) > 2 ||
        fmin(min_x, fmin(min_y, min_z)) < -2) {
        return;
    }

    // ------------------- Viewport transform -------------------- //
    matrix_transformation(&A_4, &state->engine->viewport_transform);
    matrix_transformation(&B_4, &state->engine->viewport_transform);
    matrix_transformation(&C_4, &state->engine->viewport_transform);

    const vec3_t A_vp = vec4_to_vec3(&A_4);
    const vec3_t B_vp = vec4_to_vec3(&B_4);
    const vec3_t C_vp = vec4_to_vec3(&C_4);

    // ---------------------- Draw Triangle ----------------------- //
    draw_triangle(state, A_vp, B_vp, C_vp, *face_normal);
}

void clip_and_draw(state_t *state,
                   const vec3_t *A,
                   const vec3_t *B,
                   const vec3_t *C,
                   const int plane_id, 
                   const vec3_t *face_normal) {
    // If the plane_id is invalid stop
    if (plane_id >= CLIPPING_PLANES) {
        return;
    }

    // If already clipped against all planes draw the triangle
    if (plane_id < 0) {
        project_and_draw(state, A, B, C, face_normal);
        return;
    }

    vec4_t plane = state->engine->clipping_planes[plane_id];

    float da = distance_to_plane(&plane, A);
    float db = distance_to_plane(&plane, B);
    float dc = distance_to_plane(&plane, C);

    unsigned short negative_counter = 0;
    da < 0 ? negative_counter++ : 0;
    db < 0 ? negative_counter++ : 0;
    dc < 0 ? negative_counter++ : 0;

    // if the triangle is outside the entire clipping plane dont render it
    if (negative_counter == 3) {
        return;
    }

    // if two of the vertices are outside the clipping plane then translate both
    // to the plane intersection with the triangle edges
    if (negative_counter == 2) {
        if (da > 0) {
            vec3_t B_ = intersection_plane_segment(&plane, A, B);
            vec3_t C_ = intersection_plane_segment(&plane, A, C);
            clip_and_draw(state, A, &B_, &C_, plane_id - 1, face_normal);
        } else if (db > 0) {
            vec3_t A_ = intersection_plane_segment(&plane, B, A);
            vec3_t C_ = intersection_plane_segment(&plane, B, C);
            clip_and_draw(state, &A_, B, &C_, plane_id - 1, face_normal);
        } else if (dc > 0) {
            vec3_t A_ = intersection_plane_segment(&plane, C, A);
            vec3_t B_ = intersection_plane_segment(&plane, C, B);
            clip_and_draw(state, &A_, &B_, C, plane_id - 1, face_normal);
        }
        return;
    }

    // if only one vertex is outside the clipping plane then create new vertices
    // and two new triangles from them to clip
    if (negative_counter == 1) {
        if (da < 0) {
            vec3_t A_1 = intersection_plane_segment(&plane, A, B);
            vec3_t A_2 = intersection_plane_segment(&plane, A, C);
            clip_and_draw(state, B, &A_2, &A_1, plane_id - 1, face_normal);
            clip_and_draw(state, B, C, &A_2, plane_id - 1, face_normal);
        } else if (db < 0) {
            vec3_t B_1 = intersection_plane_segment(&plane, B, C);
            vec3_t B_2 = intersection_plane_segment(&plane, B, A);
            clip_and_draw(state, C, &B_2, &B_1, plane_id - 1, face_normal);
            clip_and_draw(state, C, A, &B_2, plane_id - 1, face_normal);
        } else if (dc < 0) {
            vec3_t C_1 = intersection_plane_segment(&plane, C, A);
            vec3_t C_2 = intersection_plane_segment(&plane, C, B);
            clip_and_draw(state, A, &C_2, &C_1, plane_id - 1, face_normal);
            clip_and_draw(state, A, B, &C_2, plane_id - 1, face_normal);
        }
        return;
    }

    clip_and_draw(state, A, B, C, plane_id - 1, face_normal);
}

void process_and_draw_triangle(state_t *state,
                               const mesh_t *mesh,
                               const int triangle_id) {
    // Assign vertices
    unsigned short A_index = mesh->indices[triangle_id * 3 + 0];
    unsigned short B_index = mesh->indices[triangle_id * 3 + 1];
    unsigned short C_index = mesh->indices[triangle_id * 3 + 2];
    const vec3_t A = mesh->vertices[A_index];
    const vec3_t B = mesh->vertices[B_index];
    const vec3_t C = mesh->vertices[C_index];

    // Calculate face normal
    vec3_t AB = vec3_sub(&B, &A);
    vec3_t AC = vec3_sub(&C, &A);
    vec3_t face_normal = vec3_cross(&AC, &AB);
    face_normal = vec3_norm(&face_normal);

    // Assign vec4
    vec4_t A_4 = vec3_to_vec4(&A);
    vec4_t B_4 = vec3_to_vec4(&B);
    vec4_t C_4 = vec3_to_vec4(&C);
    vec4_t N_4 = vec3_to_vec4(&face_normal);

    // ------------------------- View Transform --------------------------
    // //
    matrix_transformation(&A_4, &state->engine->view_transform);
    matrix_transformation(&B_4, &state->engine->view_transform);
    matrix_transformation(&C_4, &state->engine->view_transform);
    matrix_transformation(&N_4, &state->engine->view_transform);
    const vec3_t A_view = vec4_to_vec3(&A_4);
    const vec3_t B_view = vec4_to_vec3(&B_4);
    const vec3_t C_view = vec4_to_vec3(&C_4);

    AB = vec3_sub(&B_view, &A_view);
    AC = vec3_sub(&C_view, &A_view);
    vec3_t face_normal_view = vec3_cross(&AC, &AB);
    face_normal_view = vec3_norm(&face_normal_view);

    if (vec3_dot(&A_view, &face_normal_view) > 0)
        return;

    // ------------------------ Backface Culling -------------------------
    // //

    // ----------------------- Triangle clipping -------------------------
    // //
    clip_and_draw(state, &A_view, &B_view, &C_view, CLIPPING_PLANES - 1, &face_normal);
}

void draw_meshes(state_t *state) {
    engine_t *engine = state->engine;
    engine->view_transform = generate_view_transform(engine->camera);
    mesh_t **meshes = engine->meshes;
    for (int i = 0; i < state->engine->mesh_count; i++) {
        for (int j = 0; j < meshes[i]->triangle_count; j++) {
            process_and_draw_triangle(state, meshes[i], j);
        }
    }
}
