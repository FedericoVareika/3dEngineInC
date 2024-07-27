#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "loading/obj_loading.h"
#include "state.h"

#include "visuals/shapes.h"

#include "rendering/buffer_drawing.h"

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
    char *camera_pos_text;
    asprintf(&camera_pos_text,
             "Camera pos:   (%f, %f, %f) \n",
             state->engine->camera->position.x,
             state->engine->camera->position.y,
             state->engine->camera->position.z);

    // TIME PERCENTAGES
    char *time_debug_text;
    asprintf(&time_debug_text,
             "Process input:  %f \n"
             "Update time:    %f \n"
             "Draw time:      %f \n"
             "Render time:    %f \n",
             state->time_tracking.input_process_time_percentage,
             state->time_tracking.update_time_percentage,
             state->time_tracking.mesh_draw_time_percentage,
             state->time_tracking.render_time_percentage);

    // MAKE GUI TEXT
    char *gui_text;
    asprintf(
        &gui_text, "%s\n%s\n%s", fps_text, camera_pos_text, time_debug_text);

    free(time_debug_text);
    free(camera_pos_text);
    free(fps_text);

    return gui_text;
}

char *mystrcat(char *dest, const char *src) {
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return --dest;
}

int main(int argc, char *argv[]) {
    freopen("log", "w", stdout);
    printf("%d\n", argc);

    char obj_path[50];
    char *obj_path_p = obj_path;
    obj_path[0] = '\0';
    char sprite_path[50];
    char *sprite_path_p = sprite_path;
    sprite_path[0] = '\0';

    obj_path_p = mystrcat(obj_path_p, "assets/objects/");
    if (argc > 1)
        obj_path_p = mystrcat(obj_path_p, argv[1]);
    else
        obj_path_p = mystrcat(obj_path_p, "Tree");
    obj_path_p = mystrcat(obj_path_p, ".obj");

    sprite_path_p = mystrcat(sprite_path_p, "assets/sprites/");
    if (argc > 2) {
        sprite_path_p = mystrcat(sprite_path_p, argv[2]);
        sprite_path_p = mystrcat(sprite_path_p, ".png");
    } else {
        sprite_path_p = NULL;
    }

    if (!init()) {
        return 1;
    }

    mesh_t *mesh = malloc(sizeof(mesh_t));
    if (!mesh) {
        fprintf(stderr, "Error allocating mesh in heap.\n");
    } else {
        bool loaded = load_mesh(
            &obj_path[0], sprite_path_p != NULL ? &sprite_path[0] : NULL, mesh);
        if (!loaded)
            printf("error loading\n");
        else {
            state->engine->meshes[state->engine->mesh_count] = mesh;
            state->engine->mesh_count++;
        }
    }

    printf("mesh count: %i\n", state->engine->mesh_count);

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

    destroy_window(state);
    freopen("/dev/stdout", "w", stdout);

    return 0;
}
