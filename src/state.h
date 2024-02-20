#ifndef STATE_H
#define STATE_H

#include "SDL2/SDL_render.h"
#include "engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SCREEN_WIDTH 1280 // 21
#define SCREEN_HEIGHT 720 // 9

typedef struct {
    bool running;

    struct {
        Uint64 last_second;
        Uint64 last_frame;
        Uint64 delta_ms; // changed to ms
        double delta;
        Uint64 frames;
        Uint64 fps; // dont know if i should make it float
    } time;

    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *ttf_font;

    struct {
        uint32_t *frame_buffer;
        float *z_buffer;
        bool *wireframe_buffer;
    } buffers;

    struct {
        SDL_Texture *frame_buffer_texture;
        SDL_Texture *z_buffer_texture;
        SDL_Texture *wireframe_texture;

        SDL_Texture *gui_texture;
    } textures;

    struct {
        enum {
            FRAME_BUFFER,
            Z_BUFFER,
            WIREFRAME,
        } render_flag;

        bool render_gui;
    } flags;

    engine_t *engine;
} state_t;

bool create_window(state_t *state);
void destroy_window(state_t *state);

void process_input(state_t *state);

void update(state_t *state);
void update_gui(state_t *state, const char *gui_text);

void render(state_t *state);
void render_gui(state_t *state);
void render_framebuffer(state_t *state);
void render_z_buffer(state_t *state);
void render_wireframe_buffer(state_t *state);

void draw_frame_buffer_pixel(uint32_t *frame_buffer,
                             const int x,
                             const int y,
                             const uint32_t color);
void draw_wireframe_buffer_pixel(bool *wireframe_buffer,
                                 const int x,
                                 const int y,
                                 const bool b);

// Z-BUFFER
void clear_zbuffer(float *z_buffer);
bool pixel_priority(const float *z_buffer,
                    const int x,
                    const int y,
                    const float z);
void draw_z_buffer_pixel(float *z_buffer,
                         const int x,
                         const int y,
                         const float z);
#endif
