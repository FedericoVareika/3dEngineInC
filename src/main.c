#include <stdio.h>

#include "engine.h"
#include "loading/obj_loading.h"
#include "state.h"

#include "./math/vec3.h"

/* #include "./visuals/shapes.h" */

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

    free(fps_text);
    free(c_dir_text);

    return gui_text;
}

int main(void) {
    if (!init()) {
        return 1;
    }

    mesh_t *mesh = malloc(sizeof(mesh_t));
    bool loaded = load_mesh("./assets/objects/mountains.obj", mesh);
    if (!loaded) 
        printf("error loading\n");
    else {
        state->engine->meshes[state->engine->mesh_count] = mesh;
        state->engine->mesh_count++;
    }

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
