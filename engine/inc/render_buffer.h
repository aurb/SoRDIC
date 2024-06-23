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

#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "engine_types.h"

RENDER_BUFFER *RENDER_BUFFER_alloc(INT width, INT height, INT z_buf_on);
void RENDER_BUFFER_free(RENDER_BUFFER *buf);
void RENDER_BUFFER_zero(RENDER_BUFFER *buf);
void RENDER_BUFFER_fill(RENDER_BUFFER *buf, COLOR *color);
void RENDER_BUFFER_copy(RENDER_BUFFER *dst, RENDER_BUFFER *src);
void RENDER_BUFFER_ARGB_MAP_copy(RENDER_BUFFER *dst, ARGB_MAP *src);

#endif
