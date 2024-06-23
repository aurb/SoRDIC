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

#ifndef VECTOR_RENDERER
#define VECTOR_RENDERER

#include "engine_types.h"

void vr_init();
void vr_set_render_buffer(const RENDER_BUFFER* rb);
void vr_cleanup();

void line_flat(INT x0, INT y0, INT x1, INT y1, COLOR *color);
void line_flat_z(PROJECTION_COORD** v, COLOR *color);

void polygon_solid(INT vcnt, PROJECTION_COORD** vp, COLOR *color);
void polygon_solid_z(INT vcnt, PROJECTION_COORD** vp, COLOR *color);
void polygon_solid_spec_z(INT vcnt, PROJECTION_COORD** vp, COLOR *color, COLOR *spec);
void polygon_interp_z(INT vcnt, PROJECTION_COORD** vp, COLOR **vcolor);
void polygon_texture_bump_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const BUMP_MAP * const mbase, MAP_COORD *mrc, const ARGB_MAP * const mref);
void polygon_texture_base_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_texture_base_mul_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const ARGB_MAP * const mbase, MAP_COORD *mrc, const ARGB_MAP * const mmul);
void polygon_texture_base_add_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const ARGB_MAP * const mbase, MAP_COORD *mrc, const ARGB_MAP * const madd);
void polygon_texture_base_mul_add_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const ARGB_MAP * const mbase, MAP_COORD *mrc, const ARGB_MAP * const mmul, const ARGB_MAP * const madd);
void polygon_solid_diff_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR *diff, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_solid_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR *spec, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_solid_diff_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR *diff, COLOR *spec, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_interp_diff_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR **vdiff, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_interp_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR **vspec, MAP_COORD *mbc, const ARGB_MAP * const mbase);
void polygon_interp_diff_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, COLOR **vdiff, COLOR **vspec, MAP_COORD *mbc, const ARGB_MAP * const mbase);

#endif