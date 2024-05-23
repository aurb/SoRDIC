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

#ifndef ASSETS_2D_H
#define ASSETS_2D_H

#include "engine_types.h"

MAP *read_map_from_image(const char * const map_filename, INT u_wrap_margin);
BUMP_MAP *convert_map_to_bump_map(MAP* in_map, FLOAT margin);
void map_free(MAP* map);
void bump_map_free(BUMP_MAP* map);
#endif