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

#ifndef MAP_GENERATORS_H
#define MAP_GENERATORS_H

#include "engine_types.h"

void map_generator_init();
void map_generator_cleanup();

void ARGB_MAP_plasma_pattern(ARGB_MAP *map, GRADIENT *g, FLOAT scale, FLOAT s2xA, FLOAT s2xT, FLOAT xo, FLOAT s2yA, FLOAT s2yT, FLOAT yo);

void ARGB_MAP_vertical_pattern(ARGB_MAP *map, GRADIENT *g);
void ARGB_MAP_horizontal_pattern(ARGB_MAP *map, GRADIENT *g);
void ARGB_MAP_diagonal_pattern(ARGB_MAP *map, GRADIENT *g);
void ARGB_MAP_radial_pattern(ARGB_MAP *map, GRADIENT *g, INT x0, INT y0);
void ARGB_MAP_xor_pattern(ARGB_MAP *map, GRADIENT *g);

#endif