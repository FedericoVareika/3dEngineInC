#include "engine.h"
#include "state.h"

#include "./math/graphics_pipeline.h"
#include "./math/vec3.h"

void create_engine(engine_t *engine) {

    engine->camera = malloc(sizeof(camera_t));
    if (!engine->camera) {
        fprintf(stderr, "Camera malloc failed \n");
        return;
    }
    engine->camera->position = (vec3_t){1, 0, 0};
    engine->camera->direction = (vec3_t){0, 0, -1};
    engine->camera->up = (vec3_t){0, 1, 0};
    engine->camera->traslation_speed = (vec3_t){0, 0, 0};
    engine->camera->rotation_speed = (vec3_t){0, 0, 0};

    engine->far = -1000;
    engine->near = -0.1;
    engine->fovy = 90.f / 180.f * PI;
    engine->aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    // frustum coords
    engine->top = engine->near * tanf(-engine->fovy / 2);
    engine->right = engine->top * engine->aspect_ratio;
    engine->bottom = -engine->top;
    engine->left = -engine->right;

    printf("top: %f, right: %f, bottom: %f, left: %f\n", engine->top,
           engine->right, engine->bottom, engine->left);

    // near
    engine->clipping_planes[0] = (vec4_t){0, 0, -1, engine->near};
    // far
    engine->clipping_planes[1] = (vec4_t){0, 0, 1, -engine->far};
    // top
    engine->clipping_planes[2] = (vec4_t){0, engine->near, engine->bottom, 0};
    // bottom
    engine->clipping_planes[3] = (vec4_t){0, -engine->near, engine->bottom, 0};
    // left
    engine->clipping_planes[4] = (vec4_t){-engine->near, 0, -engine->right, 0};
    // right
    engine->clipping_planes[5] = (vec4_t){engine->near, 0, -engine->right, 0};

    // Engine meshes
    engine->model_count = 0;
    engine->models = malloc(sizeof(mesh_t *) * MAX_MESHES);
    if (!engine->models) {
        fprintf(stderr, "Mesh malloc failed \n");
        return;
    }

    engine->directional_light = (vec3_t){0, -1, 1};
    engine->directional_light = vec3_norm(&engine->directional_light);

    engine->projection_transform = generate_projection_transform(
        engine->fovy, engine->aspect_ratio, -engine->near, -engine->far);
    engine->viewport_transform =
        generate_viewport_transform(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void destroy_engine(engine_t *engine) {
    for (int i = 0; i < engine->model_count; i++) {
        free(engine->models[i]->vertices);
        free(engine->models[i]->tex_coords);
        free(engine->models[i]->normals);

        // TODO free textures

        for (int j = 0; j < engine->models[i]->mesh_count; j++) {
            free(engine->models[i]->meshes[j].v_indices);
            free(engine->models[i]->meshes[j].n_indices);
            free(engine->models[i]->meshes[j].t_indices);
            if (engine->models[i]->meshes[j].mtl)
                free(engine->models[i]->meshes[j].mtl->name);
        }
        free(engine->models[i]->textures);
        free(engine->models[i]);
    }
    free(engine->models);
    free(engine->camera);
}

void move_camera(engine_t *engine, float delta_time) {
    camera_t *camera = engine->camera;
    vec3_t delta_pos = vec3_mul(&camera->traslation_speed, delta_time);

    // calculate camera base_vectors (u, v, w)
    vec3_t w = vec3_mul(&camera->direction, -1);
    w = vec3_norm(&w);
    vec3_t u = vec3_cross(&camera->up, &w);
    u = vec3_norm(&u);
    vec3_t v = vec3_cross(&w, &u);
    v = vec3_norm(&v);

    vec3_t delta_u = vec3_mul(&u, delta_pos.x);
    // changed to up so that it will always be vertical
    vec3_t delta_up = vec3_mul(&camera->up, delta_pos.y);
    vec3_t delta_w = vec3_mul(&w, delta_pos.z);

    delta_pos = vec3_add(&delta_u, &delta_up);
    delta_pos = vec3_add(&delta_pos, &delta_w);

    camera->position = vec3_add(&camera->position, &delta_pos);
    engine->directional_light = camera->direction;
}

void rotate_camera(engine_t *engine, float delta_time) {
    camera_t *camera = engine->camera;

    // calculate camera base_vectors (u, v, w)
    vec3_t w = vec3_mul(&camera->direction, -1);
    w = vec3_norm(&w);
    vec3_t u = vec3_cross(&camera->up, &w);
    u = vec3_norm(&u);
    // vec3_t v = vec3_cross(&w, &u);
    // v = vec3_norm(&v);

    vec4_t temp_v4 = vec3_to_vec4(&camera->direction);

    // stop camera from looking "more down" or "more up"
    bool looking_down = vec3_dot(&camera->direction, &camera->up) <= -0.999;
    bool looking_up = vec3_dot(&camera->direction, &camera->up) >= 0.999;
    if (!(looking_up && camera->rotation_speed.x > 0) &&
        !(looking_down && camera->rotation_speed.x < 0)) {
        const matrix_t R_u = rotation_matrix_from_axis(
            &u, delta_time * camera->rotation_speed.x);
        matrix_transformation(&temp_v4, &R_u);
    }

    const matrix_t R_w =
        rotation_matrix_from_axis(&w, delta_time * camera->rotation_speed.z);
    matrix_transformation(&temp_v4, &R_w);

    const matrix_t R_up = rotation_matrix_from_axis(
        &camera->up, delta_time * camera->rotation_speed.y);
    matrix_transformation(&temp_v4, &R_up);

    camera->direction = vec4_to_vec3(&temp_v4);
}
