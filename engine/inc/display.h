/*  Software Rendered Demo Engine In C
    Copyright (C) 2024 https://github.com/aurb

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

#ifndef SCREEN_SDL
#define SCREEN_SDL

#include "engine_types.h"
#include "display.h"
#include "render_buffer.h"

#define FULLSCREEN_SWITCH_MODE (1)
#define FULLSCREEN_CURRENT_MODE (2)

typedef struct {
    int frames;
    double time;
} RUN_STATS;

int display_init(int window_width, int window_height, int window_flags, const char *window_name);
void display_show(const int delay);
RENDER_BUFFER *display_buffer();
void display_cleanup();

double display_last_frame_interval();
RUN_STATS display_run_stats();
void periodic_fps_printf(double period);

#endif