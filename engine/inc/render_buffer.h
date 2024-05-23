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

#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "engine_types.h"

RENDER_BUFFER *render_buffer(INT width, INT height, INT z_buf_on);
void render_buffer_free(RENDER_BUFFER *buf);

void render_buffer_zero(RENDER_BUFFER *buf);
void render_buffer_fill(RENDER_BUFFER *buf, VEC_3 *color);

void render_buffer_copy(RENDER_BUFFER *dst, RENDER_BUFFER *src);
void render_buffer_fixed_mask(RENDER_BUFFER *dst, RENDER_BUFFER **src, RENDER_BUFFER *mask);
void render_buffer_dither_opacity(RENDER_BUFFER *dst, RENDER_BUFFER *src_ch0,
                                  RENDER_BUFFER *src_ch1, RENDER_BUFFER *opacity);
void render_buffer_full_opacity(RENDER_BUFFER *dst, RENDER_BUFFER *src_ch0,
                                  RENDER_BUFFER *src_ch1, RENDER_BUFFER *opacity);
void render_buffer_add_saturation(RENDER_BUFFER *dst, RENDER_BUFFER *src);

#endif
