#include "buffer_drawing.h"
#include "../math/graphics_pipeline.h"
#include "../rendering/rasterizer.h"

// New mesh with the same dimensions as *mesh
mesh_t *new_empty_mesh(const mesh_t *mesh) {
    mesh_t *new_mesh = malloc(sizeof(mesh_t));
    new_mesh->vertex_count = mesh->vertex_count;
    new_mesh->triangle_count = mesh->triangle_count;
    new_mesh->vertices = malloc(sizeof(vec3_t) * new_mesh->vertex_count);

    return new_mesh;
}

// Transform the mesh through the 3d graphics pipeline
// Returns if the mesh ir outside the screen (for now)
bool transform_mesh(mesh_t *transformed_mesh,
                    const mesh_t *original_mesh,
                    const matrix_t *view_transform,
                    const matrix_t *projection_transform,
                    const matrix_t *viewport_transform) {
    bool outside_projection = false;

    for (int j = 0; j < original_mesh->triangle_count * 3; j++) {
        vec4_t v4 = vec3_to_vec4(&original_mesh->vertices[j]);
        matrix_transformation(&v4, view_transform);
        matrix_transformation(&v4, projection_transform);

        // If the transformed vector is outside the canonical view volume
        // dont render
        // In the future remove this for culling && clipping
        vec3_t ndc_coords = vec4_to_vec3(&v4);
        if (fmin(ndc_coords.x, fmin(ndc_coords.y, ndc_coords.z)) < -1 ||
            fmax(ndc_coords.x, fmax(ndc_coords.y, ndc_coords.z)) > 1) {
            outside_projection = true;
        } else {
            matrix_transformation(&v4, viewport_transform);
            transformed_mesh->vertices[j] = vec4_to_vec3(&v4);
        }
    }

    return outside_projection;
}

void draw_meshes(state_t *state) {
    engine_t *engine = state->engine;
    engine->view_transform = generate_view_transform(engine->camera);
    mesh_t *meshes = engine->meshes;
    for (int i = 0; i < state->engine->mesh_count; i++) {
        // Make a mesh that stores the transformed vertices to draw to the
        // frame_buffer
        mesh_t *transformed_mesh = new_empty_mesh(&meshes[i]);

        // Transform all vertices in mesh to viewport
        bool outside_projection = transform_mesh(transformed_mesh,
                                                 &meshes[i],
                                                 &engine->view_transform,
                                                 &engine->projection_transform,
                                                 &engine->viewport_transform);

        if (!outside_projection) {
            draw_mesh(state, transformed_mesh);
        }

        free(transformed_mesh->vertices);
        free(transformed_mesh);
    }
}
