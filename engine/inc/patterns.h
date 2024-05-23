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

#ifndef PATTERNS_H
#define PATTERNS_H

#include "engine_types.h"

typedef struct {
    RGB_PIXEL *gradient;
    INT gradient_size;
    INT *sin1, *sin2_x, *sin2_y; //sine tables
    INT sin_size; //size of each sine table
    INT xo, yo;
} SINE_PATTERN;

void sine_pattern_make_tables(SINE_PATTERN *p, INT s1T, INT s2xA, INT s2xT, INT s2yA, INT s2yT);
void sine_pattern_free(SINE_PATTERN *p);
void sine_pattern_render(SINE_PATTERN *p, RENDER_BUFFER *rb, INT xo, INT yo);

#endif