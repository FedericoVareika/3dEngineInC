#include <stdio.h>

#include "engine.h"
#include "state.h"

#include "./math/vec3.h"

#include "./visuals/shapes.h"

#include "./rendering/buffer_drawing.h"

engine_t *engine = NULL;
state_t *state = NULL;

bool init(void) {
    engine_t *engine = malloc(sizeof(engine_t));
    if (!engine) {
        fprintf(stderr, "Error allocating engine in heap.\n");
        return false;
    }
    create_engine(engine);

    state = malloc(sizeof(state_t));
    if (!state) {
        fprintf(stderr, "Error allocating state in heap.\n");
        return false;
    }
    state->engine = engine;
    if (!create_window(state)) {
        return false;
    }

    return true;
}

char *get_gui_text(state_t *state) {
    // FPS
    char *fps_text;
    asprintf(&fps_text, "FPS: %llu", state->time.fps);

    // CAMERA DIR
    char *c_dir_text;
    camera_t *camera = state->engine->camera;
    asprintf(&c_dir_text,
             "Camera direction: (%f, %f, %f)",
             camera->direction.x,
             camera->direction.y,
             camera->direction.z);

    // MAKE GUI TEXT
    char *gui_text;
    asprintf(&gui_text, "%s\n%s", fps_text, c_dir_text);
    // char *gui_text = malloc(strlen(fps_text) + strlen(c_dir_text) + 1);
    // strcpy(gui_text, fps_text);
    // strcat(gui_text, c_dir_text);

    free(fps_text);
    free(c_dir_text);

    return gui_text;
}

int main(void) {
    if (!init()) {
        return 1;
    }

    make_unit_cube(state->engine);
    for (int i = 0; i < state->engine->meshes[0].triangle_count * 3; i++) {
        state->engine->meshes[0].vertices[i].y -= 0;
        state->engine->meshes[0].vertices[i].x -= 1;
    }
    make_unit_cube(state->engine);
    for (int i = 0; i < state->engine->meshes[1].triangle_count * 3; i++) {
        state->engine->meshes[1].vertices[i].x += 1;
        state->engine->meshes[1].vertices[i].y += 1;
    }

    make_prism(state->engine, 3, -5, -5, 3, 10, 1);
    printf("mesh count: %i\n", state->engine->mesh_count);

    while (state->running) {
        process_input(state);
        update(state);

        char *gui_text = get_gui_text(state);
        update_gui(state, gui_text);
        free(gui_text);

        draw_meshes(state);

        render(state);
    }

    return 0;
}
