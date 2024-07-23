#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "loading/obj_loading.h"
#include "state.h"

#include "visuals/shapes.h"

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

    state->time_tracking.input_process_time = 0;
    state->time_tracking.input_process_time_percentage = 0;

    state->time_tracking.update_time = 0;
    state->time_tracking.update_time_percentage = 0;

    state->time_tracking.mesh_draw_time = 0;
    state->time_tracking.mesh_draw_time_percentage = 0;

    state->time_tracking.render_time = 0;
    state->time_tracking.render_time_percentage = 0;

    state->time_tracking.total_time = 0;

    return true;
}

char *get_gui_text(state_t *state) {
    // FPS
    char *fps_text;
    asprintf(&fps_text, "FPS: %llu", state->time.fps);

    // CAMERA DIR
    char *time_debug_text;
    asprintf(&time_debug_text,
             "\
             Process input:  %f \n\
             Update time:    %f \n\
             Draw time:      %f \n\
             Render time:    %f \n",
             state->time_tracking.input_process_time_percentage,
             state->time_tracking.update_time_percentage,
             state->time_tracking.mesh_draw_time_percentage,
             state->time_tracking.render_time_percentage);

    // MAKE GUI TEXT
    char *gui_text;
    asprintf(&gui_text, "%s\n%s", fps_text, time_debug_text);

    free(fps_text);
    free(time_debug_text);

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

    mesh = malloc(sizeof(mesh_t));
    state->engine->meshes[state->engine->mesh_count] = mesh;
    make_prism(state->engine, 0, 100, 0, 1, 1, 1);

    /* printf("mesh count: %i\n", state->engine->mesh_count); */

    Uint64 start_time;
    Uint64 time;
    Uint64 input_process_time;
    Uint64 update_time;
    Uint64 mesh_draw_time;
    Uint64 render_time;

    while (state->running) {
        start_time = clock();

        time = start_time;
        process_input(state);
        input_process_time = clock() - time;

        time = clock();
        update(state);
        update_time = clock() - time;

        char *gui_text = get_gui_text(state);
        update_gui(state, gui_text);
        free(gui_text);

        time = clock();
        draw_meshes(state);
        mesh_draw_time = clock() - time;

        time = clock();
        render(state);
        render_time = clock() - time;

        state->time_tracking.input_process_time = input_process_time;
        state->time_tracking.update_time = update_time;
        state->time_tracking.mesh_draw_time = mesh_draw_time;
        state->time_tracking.render_time = render_time;
        state->time_tracking.total_time = clock() - start_time;
    }

    return 0;
}
