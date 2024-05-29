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

#ifndef MAPS_H
#define MAPS_H

#include "engine_types.h"

RGB_MAP *rgb_map_alloc(INT width, INT height, INT wrap_margin);
void rgb_map_clear(RGB_MAP *map);
void rgb_map_fill(RGB_MAP *map, VEC_3 *color);
void rgb_map_copy(RGB_MAP *dst, RGB_MAP *src);
void rgb_map_free(RGB_MAP* map);

BUMP_MAP *bump_map_alloc(INT width, INT height);
void bump_map_free(BUMP_MAP* map);

Z_MAP *z_map_alloc(INT width, INT height);
void z_map_clear(Z_MAP *map);
void z_map_copy(Z_MAP *dst, Z_MAP *src);
void z_map_free(Z_MAP* map);

void rgb_map_fixed_mask(RGB_MAP *dst, RGB_MAP **src, RGB_MAP *mask);
void rgb_map_blend_dither(RGB_MAP *dst, RGB_MAP *rgb0_ch, RGB_MAP *rgb1_ch, RGB_MAP *alpha_ch);
void rgb_map_blend_mul(RGB_MAP *dst, RGB_MAP *rgb0_ch, RGB_MAP *rgb1_ch, RGB_MAP *alpha_ch);
void rgb_map_sat_add(RGB_MAP *a, RGB_MAP *b);

RGB_MAP *read_map_from_image(const char * const map_filename, INT u_wrap_margin);
BUMP_MAP *convert_map_to_bump_map(RGB_MAP* in_map, FLOAT margin);

void plasma_precalc(PLASMA_DATA *p, INT s1T, INT s2xA, INT s2xT, INT s2yA, INT s2yT);
void plasma_rasterize(PLASMA_DATA *p, RGB_MAP *map, INT xo, INT yo);
void plasma_free(PLASMA_DATA *p);

#endif