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

#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine_types.h"

#include "annotations.h"
#include "color.h"
#include "display.h"
#include "gradient.h"
#include "map.h"
#include "map_generators.h"
#include "map_filters.h"
#include "render_buffer.h"
#include "transitions.h"
#include "utils.h"

#include "v_geometry.h"
#include "v_lighting.h"
#include "v_obj_3d.h"
#include "v_obj_3d_container.h"
#include "v_obj_3d_generators.h"
#include "v_rasterizer.h"
#include "v_scene.h"


INT engine_init(INT window_width, INT window_height, INT window_flags, const char *window_name);
RUN_STATS engine_run_stats();
INT engine_load_music(const char *music_filename);
INT engine_play_music();
INT engine_playing();
INT engine_set_music_position(FLOAT t);
EVENT* engine_poll_events();
INT engine_cleanup();

#endif