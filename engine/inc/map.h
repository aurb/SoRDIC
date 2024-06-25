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

#ifndef MAPS_H
#define MAPS_H

#include "engine_types.h"

ARGB_MAP *ARGB_MAP_alloc(INT width, INT height, INT wrap_margin);
void ARGB_MAP_clear(ARGB_MAP *map);
void ARGB_MAP_fill(ARGB_MAP *map, COLOR *color);
void ARGB_MAP_copy(ARGB_MAP *dst, ARGB_MAP *src);
void ARGB_MAP_free(ARGB_MAP* map);

BUMP_MAP *BUMP_MAP_alloc(INT width, INT height);
void BUMP_MAP_free(BUMP_MAP* map);

Z_MAP *Z_MAP_alloc(INT width, INT height);
void Z_MAP_clear(Z_MAP *map);
void Z_MAP_copy(Z_MAP *dst, Z_MAP *src);
void Z_MAP_free(Z_MAP* map);

ARGB_MAP *ARGB_MAP_read_image(const char * const map_filename, INT u_wrap_margin);
BUMP_MAP *BUMP_MAP_from_ARGB_MAP(ARGB_MAP* in_map, FLOAT margin);

#endif