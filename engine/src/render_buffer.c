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

#include <stdlib.h>
#include <string.h>
#include "engine_types.h"
#include "engine.h"
#include "maps.h"

RENDER_BUFFER *render_buffer(INT width, INT height, INT z_buf_on) {
    RENDER_BUFFER *buf = calloc(1, sizeof(RENDER_BUFFER));
    buf->rgb = rgb_map_alloc(width, height, 0);
    if (z_buf_on == Z_BUFFER_ON) {
        buf->z = z_map_alloc(width, height);
    }
    else {
        buf->z = NULL;
    }
    buf->width = width;
    buf->height = height;
    return buf;
}

void render_buffer_free(RENDER_BUFFER *buf) {
    if (buf->rgb != NULL) {
        rgb_map_free(buf->rgb);
        buf->rgb = NULL;
    }
    if (buf->z != NULL) {
        z_map_free(buf->z);
        buf->z = NULL;
    }
    free(buf);
}

void render_buffer_zero(RENDER_BUFFER *buf) {
    rgb_map_clear(buf->rgb);
    z_map_clear(buf->z);
}

void render_buffer_fill(RENDER_BUFFER *buf, VEC_3 *color) {
    rgb_map_fill(buf->rgb, color);
    z_map_clear(buf->z);
}

void render_buffer_copy(RENDER_BUFFER *dst, RENDER_BUFFER *src) {
    rgb_map_copy(dst->rgb, src->rgb);
    z_map_copy(dst->z, src->z);
}
