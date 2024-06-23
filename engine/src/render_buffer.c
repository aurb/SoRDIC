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

#include "engine.h"

RENDER_BUFFER *RENDER_BUFFER_alloc(INT width, INT height, INT z_buf_on) {
    RENDER_BUFFER *buf = calloc(1, sizeof(RENDER_BUFFER));
    buf->map = ARGB_MAP_alloc(width, height, 0);
    if (z_buf_on == Z_BUFFER_ON) {
        buf->z = Z_MAP_alloc(width, height);
    }
    else {
        buf->z = NULL;
    }
    buf->width = width;
    buf->height = height;
    return buf;
}

void RENDER_BUFFER_free(RENDER_BUFFER *buf) {
    if (buf->map != NULL) {
        ARGB_MAP_free(buf->map);
        buf->map = NULL;
    }
    if (buf->z != NULL) {
        Z_MAP_free(buf->z);
        buf->z = NULL;
    }
    free(buf);
}

void RENDER_BUFFER_zero(RENDER_BUFFER *buf) {
    ARGB_MAP_clear(buf->map);
    Z_MAP_clear(buf->z);
}

void RENDER_BUFFER_fill(RENDER_BUFFER *buf, COLOR *color) {
    ARGB_MAP_fill(buf->map, color);
    Z_MAP_clear(buf->z);
}

void RENDER_BUFFER_copy(RENDER_BUFFER *dst, RENDER_BUFFER *src) {
    ARGB_MAP_copy(dst->map, src->map);
    Z_MAP_copy(dst->z, src->z);
}

void RENDER_BUFFER_ARGB_MAP_copy(RENDER_BUFFER *dst, ARGB_MAP *src) {
    ARGB_MAP_copy(dst->map, src);
    Z_MAP_clear(dst->z);
}
