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

#ifndef MAP_FILTERS_H
#define MAP_FILTERS_H

#include "engine_types.h"

void map_filters_init();
void map_filters_cleanup();

#define MAX_EDGE_WIDTH (15)

void ARGB_MAP_green_gradient_global_copy(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *fg, GRADIENT *g, const INT p);
void ARGB_MAP_green_gradient_global_blend(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *bg, ARGB_MAP *fg, GRADIENT *g, const INT p);

void ARGB_MAP_green_gradient_per_pixel_copy(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *fg, GRADIENT *g, ARGB_MAP *p);
void ARGB_MAP_green_gradient_per_pixel_blend(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *bg, ARGB_MAP *fg, GRADIENT *g, ARGB_MAP *p);

void ARGB_MAP_blur_nx1_global_copy(ARGB_MAP *out, INT out_x, INT out_y, COLOR *bg, ARGB_MAP *fg, const INT p);
//void ARGB_MAP_blur_nx1_global_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p);
void ARGB_MAP_blur_nx1_global_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p);

void ARGB_MAP_blur_nx1_per_pixel_copy(ARGB_MAP *out, ARGB_MAP *in, ARGB_MAP *p);
void ARGB_MAP_blur_nx1_per_pixel_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, ARGB_MAP *p);

void ARGB_MAP_blur_1xn_global_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p);
void ARGB_MAP_blur_1xn_global_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p);

void ARGB_MAP_pixelize_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p);
void ARGB_MAP_pixelize_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p);
void ARGB_MAP_rand_pixelize_copy(ARGB_MAP *out, ARGB_MAP *in,
                                 const INT l_min, const INT l_max, UINT seed_l,
                                 const INT h_min, const INT h_max, UINT seed_h);
void ARGB_MAP_rand_pixelize_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg,
                                  const INT l_min, const INT l_max, UINT seed_l,
                                  const INT h_min, const INT h_max, UINT seed_h);
#endif