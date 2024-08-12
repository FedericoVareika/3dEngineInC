#include "buffer_drawing.h"
#include "../math/graphics_pipeline.h"
#include "rasterizer.h"

void project_and_draw(state_t *state, const vec3_t *A, const vec3_t *A_uv,
                      const vec3_t *B, const vec3_t *B_uv, const vec3_t *C,
                      const vec3_t *C_uv, const vec3_t *face_normal,
                      const tex_t *tex) {
    /* printf("After clipping\n"); */

    vec4_t A_4 = vec3_to_vec4(A);
    vec4_t B_4 = vec3_to_vec4(B);
    vec4_t C_4 = vec3_to_vec4(C);

    // ------------------- Projection Transform -------------------- //
    matrix_transformation(&A_4, &state->engine->projection_transform);
    matrix_transformation(&B_4, &state->engine->projection_transform);
    matrix_transformation(&C_4, &state->engine->projection_transform);

    vec3_t A_uv_proj;
    vec3_t B_uv_proj;
    vec3_t C_uv_proj;

    if (A_uv != NULL && B_uv != NULL && C_uv != NULL) {
        A_uv_proj = vec3_mul(A_uv, 1 / A_4.w);
        B_uv_proj = vec3_mul(B_uv, 1 / B_4.w);
        C_uv_proj = vec3_mul(C_uv, 1 / C_4.w);
    }

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
    draw_triangle(state, A_vp, A_uv == NULL ? NULL : &A_uv_proj, B_vp,
                  B_uv == NULL ? NULL : &B_uv_proj, C_vp,
                  C_uv == NULL ? NULL : &C_uv_proj, *face_normal, tex);
    /* draw_textured_triangle(state, A_vp, B_vp, C_vp, *face_normal); */
}

void clip_and_draw(state_t *state, const vec3_t *A, const vec3_t *A_uv,
                   const vec3_t *B, const vec3_t *B_uv, const vec3_t *C,
                   const vec3_t *C_uv, const int plane_id,
                   const vec3_t *face_normal, const tex_t *tex) {
    // If the plane_id is invalid stop
    if (plane_id >= CLIPPING_PLANES) return;

    // If already clipped against all planes draw the triangle
    if (plane_id < 0) {
        project_and_draw(state, A, A_uv, B, B_uv, C, C_uv, face_normal, tex);
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
    if (negative_counter == 3) return;

    // if two of the vertices are outside the clipping plane then translate both
    // to the plane intersection with the triangle edges
    if (negative_counter == 2) {
        if (da > 0) {
            vec3_t B_uv_;
            vec3_t B_ =
                intersection_plane_segment(&plane, A, A_uv, B, B_uv, &B_uv_);
            vec3_t C_uv_;
            vec3_t C_ =
                intersection_plane_segment(&plane, A, A_uv, C, C_uv, &C_uv_);
            clip_and_draw(state, A, A_uv, &B_, A_uv == NULL ? NULL : &B_uv_,
                          &C_, A_uv == NULL ? NULL : &C_uv_, plane_id - 1,
                          face_normal, tex);
        } else if (db > 0) {
            vec3_t A_uv_;
            vec3_t A_ =
                intersection_plane_segment(&plane, B, B_uv, A, A_uv, &A_uv_);
            vec3_t C_uv_;
            vec3_t C_ =
                intersection_plane_segment(&plane, B, B_uv, C, C_uv, &C_uv_);
            clip_and_draw(state, &A_, A_uv == NULL ? NULL : &A_uv_, B, B_uv,
                          &C_, A_uv == NULL ? NULL : &C_uv_, plane_id - 1,
                          face_normal, tex);
        } else if (dc > 0) {
            vec3_t A_uv_;
            vec3_t A_ =
                intersection_plane_segment(&plane, C, C_uv, A, A_uv, &A_uv_);
            vec3_t B_uv_;
            vec3_t B_ =
                intersection_plane_segment(&plane, C, C_uv, B, B_uv, &B_uv_);
            clip_and_draw(state, &A_, A_uv == NULL ? NULL : &A_uv_, &B_,
                          A_uv == NULL ? NULL : &B_uv_, C, C_uv, plane_id - 1,
                          face_normal, tex);
        }
        return;
    }

    // if only one vertex is outside the clipping plane then create new vertices
    // and two new triangles from them to clip
    if (negative_counter == 1) {
        if (da < 0) {
            vec3_t A_uv_1;
            vec3_t A_1 =
                intersection_plane_segment(&plane, A, A_uv, B, B_uv, &A_uv_1);
            vec3_t A_uv_2;
            vec3_t A_2 =
                intersection_plane_segment(&plane, A, A_uv, C, C_uv, &A_uv_2);
            clip_and_draw(state, B, B_uv, &A_2, A_uv == NULL ? NULL : &A_uv_2,
                          &A_1, A_uv == NULL ? NULL : &A_uv_1, plane_id - 1,
                          face_normal, tex);
            clip_and_draw(state, B, B_uv, C, C_uv, &A_2,
                          A_uv == NULL ? NULL : &A_uv_2, plane_id - 1,
                          face_normal, tex);
        } else if (db < 0) {
            vec3_t B_uv_1;
            vec3_t B_1 =
                intersection_plane_segment(&plane, B, B_uv, C, C_uv, &B_uv_1);
            vec3_t B_uv_2;
            vec3_t B_2 =
                intersection_plane_segment(&plane, B, B_uv, A, A_uv, &B_uv_2);
            clip_and_draw(state, C, C_uv, &B_2, A_uv == NULL ? NULL : &B_uv_2,
                          &B_1, A_uv == NULL ? NULL : &B_uv_1, plane_id - 1,
                          face_normal, tex);
            clip_and_draw(state, C, C_uv, A, A_uv, &B_2,
                          A_uv == NULL ? NULL : &B_uv_2, plane_id - 1,
                          face_normal, tex);
        } else if (dc < 0) {
            vec3_t C_uv_1;
            vec3_t C_1 =
                intersection_plane_segment(&plane, C, C_uv, A, A_uv, &C_uv_1);
            vec3_t C_uv_2;
            vec3_t C_2 =
                intersection_plane_segment(&plane, C, C_uv, B, B_uv, &C_uv_2);
            clip_and_draw(state, A, A_uv, &C_2, A_uv == NULL ? NULL : &C_uv_2,
                          &C_1, A_uv == NULL ? NULL : &C_uv_1, plane_id - 1,
                          face_normal, tex);
            clip_and_draw(state, A, A_uv, B, B_uv, &C_2,
                          A_uv == NULL ? NULL : &C_uv_2, plane_id - 1,
                          face_normal, tex);
        }
        return;
    }

    clip_and_draw(state, A, A_uv, B, B_uv, C, C_uv, plane_id - 1, face_normal,
                  tex);
}

void process_and_draw_triangle(state_t *state, const model_t *model,
                               const int mesh_idx, const int triangle_id) {
    mesh_t *mesh = &model->meshes[mesh_idx];
    // Assign vertices
    unsigned int A_index = mesh->v_indices[triangle_id * 3 + 0];
    unsigned int B_index = mesh->v_indices[triangle_id * 3 + 1];
    unsigned int C_index = mesh->v_indices[triangle_id * 3 + 2];
    const vec3_t A = model->vertices[A_index];
    const vec3_t B = model->vertices[B_index];
    const vec3_t C = model->vertices[C_index];

    /* printf("Before accessing uv's\n"); */
    vec3_t *A_uvp = NULL;
    vec3_t *B_uvp = NULL;
    vec3_t *C_uvp = NULL;
    if (model->tex_coords != NULL) {
        unsigned int A_uv_index = mesh->t_indices[triangle_id * 3 + 0];
        unsigned int B_uv_index = mesh->t_indices[triangle_id * 3 + 1];
        unsigned int C_uv_index = mesh->t_indices[triangle_id * 3 + 2];

        if (A_uv_index != -1 && B_uv_index != -1 && C_uv_index != -1) {
            A_uvp = &model->tex_coords[A_uv_index];
            B_uvp = &model->tex_coords[B_uv_index];
            C_uvp = &model->tex_coords[C_uv_index];
        }
    }

    // Calculate face normal
    vec3_t face_normal;
    vec3_t AB = vec3_sub(&B, &A);
    vec3_t AC = vec3_sub(&C, &A);
    if (model->normals != NULL) {
        unsigned int A_norm_index = mesh->n_indices[triangle_id * 3 + 0];
        unsigned int B_norm_index = mesh->n_indices[triangle_id * 3 + 1];
        unsigned int C_norm_index = mesh->n_indices[triangle_id * 3 + 2];

        if (A_norm_index == -1 || B_norm_index == -1 || C_norm_index == -1) {
            face_normal = vec3_cross(&AC, &AB);
            face_normal = vec3_norm(&face_normal);
        } else {
            vec3_t A_n = model->normals[A_norm_index];
            vec3_t B_n = model->normals[B_norm_index];
            vec3_t C_n = model->normals[C_norm_index];
            face_normal = (vec3_t){
                A_n.x + B_n.x + C_n.x, 
                A_n.y + B_n.y + C_n.y, 
                A_n.z + B_n.z + C_n.z, 
            };
            face_normal = vec3_norm(&face_normal);
        }
    } else {
        face_normal = vec3_cross(&AB, &AC);
        face_normal = vec3_norm(&face_normal);
    }

    /* if (vec3_dot(&A, &face_normal) < 0) return; */
    vec3_t camera_to_norm = vec3_add(&A, &face_normal);

    // Assign vec4
    vec4_t A_4 = vec3_to_vec4(&A);
    vec4_t B_4 = vec3_to_vec4(&B);
    vec4_t C_4 = vec3_to_vec4(&C);
    vec4_t CN_4 = vec3_to_vec4(&camera_to_norm);

    // ------------------------- View Transform --------------------------

    matrix_transformation(&A_4, &state->engine->view_transform);
    matrix_transformation(&B_4, &state->engine->view_transform);
    matrix_transformation(&C_4, &state->engine->view_transform);
    matrix_transformation(&CN_4, &state->engine->view_transform);
    const vec3_t A_view = vec4_to_vec3(&A_4);
    const vec3_t B_view = vec4_to_vec3(&B_4);
    const vec3_t C_view = vec4_to_vec3(&C_4);

    vec3_t camera_to_norm_view = vec4_to_vec3(&CN_4);
    vec3_t face_normal_view = vec3_sub(&camera_to_norm_view, &A_view);

    // ------------------------ Backface Culling -------------------------

    if (vec3_dot(&A_view, &face_normal_view) < 0) return;

    // ----------------------- Triangle clipping -------------------------

    tex_t *diffuse_tex = NULL;
    if (mesh->mtl && mesh->mtl->diffuse_tex_idx != -1)
        diffuse_tex = &model->textures[mesh->mtl->diffuse_tex_idx];
    clip_and_draw(state, &A_view, A_uvp, &B_view, B_uvp, &C_view, C_uvp,
                  CLIPPING_PLANES - 1, &face_normal, diffuse_tex);
}

void draw_meshes(state_t *state) {
    engine_t *engine = state->engine;
    engine->view_transform = generate_view_transform(engine->camera);
    model_t **models = engine->models;
    for (int i = 0; i < engine->model_count; i++) {
        for (int j = 0; j < models[i]->mesh_count; j++) {
            for (int l = 0; l < models[i]->meshes[j].triangle_count; l++) {
                process_and_draw_triangle(state, models[i], j, l);
            }
        }
    }
}
