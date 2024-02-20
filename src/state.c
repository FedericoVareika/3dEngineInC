#include "SDL2/SDL.h"
#include <dlfcn.h>

#include "SDL2/SDL_render.h"
#include "engine.h"
#include "state.h"

#include "./math/vec3.h"

#define FONT_SIZE 24

bool create_window(state_t *state) {
    state->running = true;

    // WINDOW
    state->window = SDL_CreateWindow("Raster",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     WINDOW_WIDTH,
                                     WINDOW_HEIGHT,
                                     0);
    if (!state->window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // RENDERER
    state->renderer = SDL_CreateRenderer(state->window,
                                         -1,
                                         SDL_RENDERER_ACCELERATED |
                                             SDL_RENDERER_PRESENTVSYNC |
                                             SDL_RENDERER_TARGETTEXTURE);
    if (!state->renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    // TTF FONT
    if (TTF_Init() < 0) {
        fprintf(stderr, "Error initializing ttf.\n");
        return false;
    }

    state->ttf_font =
        TTF_OpenFont("./assets/fonts/SHPinscher-Regular.ttf", FONT_SIZE);
    if (!state->ttf_font) {
        fprintf(stderr, "Error opening ttf font.\n");
        return false;
    }

    // BUFFERS
    state->buffers.frame_buffer =
        malloc(sizeof(uint32_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!state->buffers.frame_buffer) {
        fprintf(stderr, "Error allocating memory for the framebuffer.\n");
        return false;
    }

    state->buffers.z_buffer =
        malloc(sizeof(uint32_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!state->buffers.z_buffer) {
        fprintf(stderr, "Error allocating memory for the z buffer.\n");
        return false;
    }
    clear_zbuffer(state->buffers.z_buffer);

    state->buffers.wireframe_buffer =
        malloc(sizeof(bool) * SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!state->buffers.wireframe_buffer) {
        fprintf(stderr, "Error allocating memory for the wireframe buffer.\n");
        return false;
    }

    // TEXTURES

    // frame buffer texture
    state->textures.frame_buffer_texture =
        SDL_CreateTexture(state->renderer,
                          SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING,
                          SCREEN_WIDTH,
                          SCREEN_HEIGHT);

    // z buffer texture
    state->textures.z_buffer_texture =
        SDL_CreateTexture(state->renderer,
                          SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING,
                          SCREEN_WIDTH,
                          SCREEN_HEIGHT);

    // wireframe texture
    state->textures.wireframe_texture =
        SDL_CreateTexture(state->renderer,
                          SDL_PIXELFORMAT_RGBA32,
                          SDL_TEXTUREACCESS_STREAMING,
                          SCREEN_WIDTH,
                          SCREEN_HEIGHT);
    // SDL_SetTextureBlendMode(state->textures.gui_texture,
    // SDL_BLENDMODE_BLEND);

    // gui texture
    state->textures.gui_texture = SDL_CreateTexture(state->renderer,
                                                    SDL_PIXELFORMAT_RGBA32,
                                                    SDL_TEXTUREACCESS_TARGET,
                                                    SCREEN_WIDTH,
                                                    SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(state->textures.gui_texture, SDL_BLENDMODE_BLEND);

    // FLAGS
    state->flags.render_flag = FRAME_BUFFER;
    state->flags.render_gui = true;

    return true;
}

void destroy_window(state_t *state) {
    SDL_DestroyTexture(state->textures.gui_texture);
    SDL_DestroyTexture(state->textures.wireframe_texture);
    SDL_DestroyTexture(state->textures.z_buffer_texture);
    SDL_DestroyTexture(state->textures.frame_buffer_texture);

    free(state->buffers.z_buffer);
    free(state->buffers.frame_buffer);

    TTF_CloseFont(state->ttf_font);
    TTF_Quit();

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}

void process_input(state_t *state) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_QUIT:
            state->running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                state->running = false;
                break;

            // Camera traslation movement with WASD
            case SDLK_SPACE:
                state->engine->camera->traslation_speed.y = CAMERA_SPEED_Y;
                break;
            case SDLK_LSHIFT:
                state->engine->camera->traslation_speed.y = -CAMERA_SPEED_Y;
                break;
            case SDLK_w:
                state->engine->camera->traslation_speed.z = -CAMERA_SPEED_Z;
                break;
            case SDLK_a:
                state->engine->camera->traslation_speed.x = -CAMERA_SPEED_X;
                break;
            case SDLK_s:
                state->engine->camera->traslation_speed.z = CAMERA_SPEED_Z;
                break;
            case SDLK_d:
                state->engine->camera->traslation_speed.x = CAMERA_SPEED_X;
                break;

            // Camera rotation movement with arrows
            case SDLK_UP:
                state->engine->camera->rotation_speed.x = CAMERA_ROTATION_SPEED;
                break;
            case SDLK_DOWN:
                state->engine->camera->rotation_speed.x =
                    -CAMERA_ROTATION_SPEED;
                break;
            case SDLK_LEFT:
                state->engine->camera->rotation_speed.y = CAMERA_ROTATION_SPEED;
                break;
            case SDLK_RIGHT:
                state->engine->camera->rotation_speed.y =
                    -CAMERA_ROTATION_SPEED;
                break;

            // Change render flags
            case SDLK_p: // change render frame buffer
                state->flags.render_flag = FRAME_BUFFER;
                break;
            case SDLK_o: // change render z buffer
                state->flags.render_flag = Z_BUFFER;
                break;
            case SDLK_i: // change render wireframe
                state->flags.render_flag = WIREFRAME;
                break;
            case SDLK_u: // change render gui
                state->flags.render_gui = !state->flags.render_gui;
                break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {

            // Camera traslation movement with WASD
            case SDLK_SPACE:
                if (state->engine->camera->traslation_speed.y > 0)
                    state->engine->camera->traslation_speed.y -= CAMERA_SPEED_Y;
                break;
            case SDLK_LSHIFT:
                if (state->engine->camera->traslation_speed.y < 0)
                    state->engine->camera->traslation_speed.y += CAMERA_SPEED_Y;
                break;
            case SDLK_w:
                if (state->engine->camera->traslation_speed.z < 0)
                    state->engine->camera->traslation_speed.z += CAMERA_SPEED_Z;
                break;
            case SDLK_a:
                if (state->engine->camera->traslation_speed.x < 0)
                    state->engine->camera->traslation_speed.x += CAMERA_SPEED_X;
                break;
            case SDLK_s:
                if (state->engine->camera->traslation_speed.z > 0)
                    state->engine->camera->traslation_speed.z -= CAMERA_SPEED_Z;
                break;
            case SDLK_d:
                if (state->engine->camera->traslation_speed.x > 0)
                    state->engine->camera->traslation_speed.x -= CAMERA_SPEED_X;
                break;

            // Camera rotation movement with arrows
            case SDLK_UP:
                if (state->engine->camera->rotation_speed.x > 0)
                    state->engine->camera->rotation_speed.x -=
                        CAMERA_ROTATION_SPEED;
                break;
            case SDLK_DOWN:
                if (state->engine->camera->rotation_speed.x < 0)
                    state->engine->camera->rotation_speed.x +=
                        CAMERA_ROTATION_SPEED;
                break;
            case SDLK_LEFT:
                if (state->engine->camera->rotation_speed.y > 0)
                    state->engine->camera->rotation_speed.y -=
                        CAMERA_ROTATION_SPEED;
                break;
            case SDLK_RIGHT:
                if (state->engine->camera->rotation_speed.y < 0)
                    state->engine->camera->rotation_speed.y +=
                        CAMERA_ROTATION_SPEED;
                break;
            }
            break;
        }
    }
}

static void update_time(state_t *state) {
    Uint64 now = SDL_GetPerformanceCounter();
    state->time.frames++;
    // printf("%i\n", (int)now);

    state->time.delta_ms = (now - state->time.last_frame) * 1000 /
                           ((double)SDL_GetPerformanceFrequency());
    state->time.delta = state->time.delta_ms * 0.001f;

    state->time.last_frame = now;

    if ((state->time.last_frame - state->time.last_second) /
            (double)SDL_GetPerformanceFrequency() >
        1) {
        state->time.fps = state->time.frames;
        state->time.frames = 0;
        state->time.last_second = state->time.last_frame;
    }
}

void update(state_t *state) {
    update_time(state);
    move_camera(state->engine, state->time.delta);
    rotate_camera(state->engine, state->time.delta);
}

void clear_framebuffer(uint32_t *frame_buffer, uint32_t color) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            frame_buffer[(SCREEN_WIDTH * y) + x] = color;
        }
    }
}

void clear_wireframe_buffer(bool *wireframe_buffer, bool val) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            wireframe_buffer[(SCREEN_WIDTH * y) + x] = val;
        }
    }
}

void clear_zbuffer(float *zbuffer) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            zbuffer[(SCREEN_WIDTH * y) + x] = 1;
        }
    }
}

void update_gui(state_t *state, const char *gui_text) {
    // SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
    SDL_Color grey = {0xFF, 0xE0, 0xE0, 0xE0};
    SDL_Surface *text_surface =
        TTF_RenderText_Solid_Wrapped(state->ttf_font, gui_text, grey, 1000);
    SDL_Texture *text_texture =
        SDL_CreateTextureFromSurface(state->renderer, text_surface);

    SDL_SetRenderTarget(state->renderer, state->textures.gui_texture);
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 0);
    SDL_RenderClear(state->renderer);

    // printf("w: %i, h: %i\n", text_surface->w, text_surface->h);
    SDL_Rect dest_rect = {
        .x = 10, .y = 10, .w = text_surface->w, .h = text_surface->h};
    SDL_RenderCopy(state->renderer, text_texture, NULL, &dest_rect);
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);

    SDL_SetRenderTarget(state->renderer, NULL);
}

void render_gui(state_t *state) {
    // SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
    // SDL_Surface *text_surface =
    //     TTF_RenderText_Solid(state->ttf_font, "Hello", white);
    // SDL_Texture *text_texture =
    //     SDL_CreateTextureFromSurface(state->renderer, text_surface);
    //
    // SDL_SetRenderTarget(state->renderer, state->gui_texture);
    // SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 0);
    // SDL_RenderClear(state->renderer);
    //
    // // printf("w: %i, h: %i\n", text_surface->w, text_surface->h);
    // SDL_Rect dest_rect = {
    //     .x = 10, .y = 10, .w = text_surface->w, .h = text_surface->h};
    // SDL_RenderCopy(state->renderer, text_texture, NULL, &dest_rect);
    // SDL_DestroyTexture(text_texture);
    // SDL_FreeSurface(text_surface);
    //
    // SDL_SetRenderTarget(state->renderer, NULL);
    // SDL_RenderCopy(state->renderer, state->gui_texture, NULL, NULL);

    SDL_RenderCopy(state->renderer, state->textures.gui_texture, NULL, NULL);
}

void render(state_t *state) {
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(state->renderer);

    switch (state->flags.render_flag) {
    case FRAME_BUFFER:
        render_framebuffer(state);
        break;
    case Z_BUFFER:
        render_z_buffer(state);
        break;
    case WIREFRAME:
        render_wireframe_buffer(state);
        break;
    }

    if (state->flags.render_gui)
        render_gui(state);

    SDL_RenderPresent(state->renderer);

    clear_framebuffer(state->buffers.frame_buffer, 0xFF000000);
    clear_zbuffer(state->buffers.z_buffer);
    clear_wireframe_buffer(state->buffers.wireframe_buffer, false);
}

void render_framebuffer(state_t *state) {
    SDL_UpdateTexture(state->textures.frame_buffer_texture,
                      NULL,
                      state->buffers.frame_buffer,
                      (int)(SCREEN_WIDTH * sizeof(uint32_t)));
    SDL_RenderCopyEx(state->renderer,
                     state->textures.frame_buffer_texture,
                     NULL,
                     NULL,
                     0,
                     NULL,
                     // 0);
                     SDL_FLIP_VERTICAL);
}

void convert_z_buffer_to_greyscale(uint32_t *greyscale_buffer,
                                   float *z_buffer) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float clamped_value = z_buffer[y * SCREEN_WIDTH + x];
            if (clamped_value < -1)
                clamped_value = 0;
            if (clamped_value > 1) {
                clamped_value = 0xFF;
            }

            uint32_t color = 0xFF;
            color = (color << 8) + (uint32_t)(0xFF * clamped_value);
            color = (color << 8) + (uint32_t)(0xFF * clamped_value);
            color = (color << 8) + (uint32_t)(0xFF * clamped_value);

            greyscale_buffer[(SCREEN_WIDTH * y) + x] = color;
        }
    }
}

void render_z_buffer(state_t *state) {
    uint32_t greyscale[SCREEN_WIDTH * SCREEN_HEIGHT];
    convert_z_buffer_to_greyscale(greyscale, state->buffers.z_buffer);

    SDL_UpdateTexture(state->textures.z_buffer_texture,
                      NULL,
                      greyscale,
                      (int)(SCREEN_WIDTH * sizeof(uint32_t)));
    SDL_RenderCopyEx(state->renderer,
                     state->textures.z_buffer_texture,
                     NULL,
                     NULL,
                     0,
                     NULL,
                     // 0);
                     SDL_FLIP_VERTICAL);
}

void convert_wireframe_buffer_to_color(uint32_t *color_buffer,
                                       const bool *wireframe_buffer) {
    uint32_t color = 0xFFDDDDDD;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (wireframe_buffer[SCREEN_WIDTH * y + x]) {
                color_buffer[SCREEN_WIDTH * y + x] = color;
            } else {
                color_buffer[SCREEN_WIDTH * y + x] = 0xFF000000;
            }
        }
    }
}

void render_wireframe_buffer(state_t *state) {
    uint32_t color_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    convert_wireframe_buffer_to_color(color_buffer,
                                      state->buffers.wireframe_buffer);

    SDL_SetRenderTarget(state->renderer, state->textures.wireframe_texture);
    SDL_SetRenderDrawColor(state->renderer, 0xFF, 0x0, 0x0, 0x0);
    SDL_RenderClear(state->renderer);
    SDL_SetRenderTarget(state->renderer, 0);

    SDL_UpdateTexture(state->textures.wireframe_texture,
                      NULL,
                      color_buffer,
                      (int)(SCREEN_WIDTH * sizeof(uint32_t)));
    SDL_RenderCopyEx(state->renderer,
                     state->textures.wireframe_texture,
                     NULL,
                     NULL,
                     0,
                     NULL,
                     // 0);
                     SDL_FLIP_VERTICAL);
}

bool pixel_priority(const float *z_buffer,
                    const int x,
                    const int y,
                    const float z) {
    return z_buffer[(SCREEN_WIDTH * y) + x] > z;
}

void draw_frame_buffer_pixel(uint32_t *frame_buffer,
                             const int x,
                             const int y,
                             const uint32_t color) {
    frame_buffer[(SCREEN_WIDTH * y) + x] = color;
}

void draw_z_buffer_pixel(float *z_buffer,
                         const int x,
                         const int y,
                         const float z) {
    z_buffer[(SCREEN_WIDTH * y) + x] = z;
}

void draw_wireframe_buffer_pixel(bool *wireframe_buffer,
                                 const int x,
                                 const int y,
                                 const bool b) {
    wireframe_buffer[(SCREEN_WIDTH * y) + x] = b;
}
