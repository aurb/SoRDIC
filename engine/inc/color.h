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

#ifndef COLOR_H
#define COLOR_H

#include "engine_types.h"

ARGB_PIXEL COLOR_to_ARGB_PIXEL(COLOR *o);
FLOAT ARGB_PIXEL_to_l(ARGB_PIXEL p);
COLOR* COLOR_scale(COLOR *a, FLOAT s);
COLOR* COLOR_blend(COLOR *a, COLOR *b, FLOAT p);
COLOR* COLOR_add(COLOR *a, COLOR *b);
COLOR* COLOR_add_sat(COLOR *a, COLOR *b);
COLOR* COLOR_mul(COLOR *a, COLOR *b);
COLOR* COLOR_hsl_to_rgb(COLOR *o);

#endif