/*  Software Rendering Demo Engine In C
    Copyright (C) 2024 Andrzej Urbaniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "engine.h"

SDL_Window *display_window;
SDL_Renderer *display_renderer;
SDL_Texture *display_texture;
RENDER_BUFFER *display_buf;

int total_frames;
double prev_frame_interval; //powiązane z display_last_frame_interval. rozważyć jak można zmodyfikować example_cube, _lights, _hierarchy, żeby nie było potrzebne
int prev_frame_ticks;

double interval_time;
int interval_frames;

int display_init(int window_width, int window_height, int window_flags, const char *window_name) {
    SDL_DisplayMode disp_mode;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    if (window_flags & FULLSCREEN_CURRENT_MODE) {
        display_window = SDL_CreateWindow(window_name,
            0, 0, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);

        if (0 != SDL_GetWindowDisplayMode(display_window, &disp_mode)) {
            printf("Error SDL_GetWindowDisplayMode: %s\n", SDL_GetError());
            return 0;
        }
        window_width = disp_mode.w;
        window_height = disp_mode.h;
    }
    else {
        display_window = SDL_CreateWindow(window_name,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, 0);
    }

    display_renderer = SDL_CreateRenderer(display_window, -1, 0);
    display_texture = SDL_CreateTexture(display_renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    /** Display render buffer has Z buffer enabled by default. */
    display_buf = RENDER_BUFFER_alloc(window_width, window_height, Z_BUFFER_ON);

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        printf("Error initializing SDL_image\n");
        return 0;
    }

    total_frames = 0;
    interval_time = engine_run_stats().time;
    interval_frames = engine_run_stats().frames;

    prev_frame_interval = 0.0;
    prev_frame_ticks = SDL_GetTicks();
    return 1;
}

void display_show(const int delay) {
    SDL_UpdateTexture(display_texture, NULL, (ARGB_PIXEL*)display_buf->map->data, display_buf->width * sizeof(ARGB_PIXEL));
    SDL_RenderCopy(display_renderer, display_texture, NULL, NULL);
    SDL_RenderPresent(display_renderer);
    SDL_Delay(delay);
    total_frames++;
    prev_frame_interval = (SDL_GetTicks() - prev_frame_ticks)/1000.0;
    prev_frame_ticks = SDL_GetTicks();
}

void display_cleanup() {
    IMG_Quit();

    SDL_DestroyTexture(display_texture);
    SDL_DestroyRenderer(display_renderer); 
    SDL_Quit();

    RENDER_BUFFER_free(display_buf);
}

RENDER_BUFFER *display_buffer() {
    return display_buf;
}

double display_last_frame_interval() {
    return prev_frame_interval;
}

void periodic_fps_printf(double period) {
    if ((engine_run_stats().time - interval_time) > period) {
        int cur_frames = engine_run_stats().frames - interval_frames;
        double cur_time = engine_run_stats().time - interval_time;
        printf("FPS: %d    \r", (int)(cur_frames / cur_time));
        fflush(stdout);
        interval_time = engine_run_stats().time;
        interval_frames = engine_run_stats().frames;
    }
}