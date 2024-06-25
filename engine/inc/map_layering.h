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

#ifndef MAP_LAYERING_H
#define MAP_LAYERING_H

#include "engine_types.h"

void ARGB_MAP_multiplex(ARGB_MAP *dst, ARGB_MAP **src, ARGB_MAP *mask);

void ARGB_MAP_fade_dither_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, FLOAT p);
void ARGB_MAP_fade_mul_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, FLOAT p);
void ARGB_MAP_blend_dither_global(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, FLOAT p);
void ARGB_MAP_blend_mul_global(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, FLOAT p);

void ARGB_MAP_fade_dither_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, ARGB_MAP *p);
void ARGB_MAP_fade_mul_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, ARGB_MAP *p);
void ARGB_MAP_blend_dither_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, ARGB_MAP *p);
void ARGB_MAP_blend_mul_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, ARGB_MAP *p);

void ARGB_MAP_fade_dither_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, FLOAT f, ARGB_MAP *p);
void ARGB_MAP_fade_mul_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in, FLOAT f, ARGB_MAP *p);
void ARGB_MAP_blend_dither_f_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, FLOAT f, ARGB_MAP *p);
void ARGB_MAP_blend_mul_f_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1, FLOAT f, ARGB_MAP *p);

void ARGB_MAP_sat_add(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1);

#endif