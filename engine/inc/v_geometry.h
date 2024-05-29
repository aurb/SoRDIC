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

#ifndef VECTOR_GEOMETRY_H
#define VECTOR_GEOMETRY_H

#include "engine_types.h"

void vr_geometry_init();
void vr_geometry_cleanup();

VEC_4 *store_v4(VEC_4 *src);
MAT_4_4 *store_m(MAT_4_4 *src);
VEC_3 *copy_v3(VEC_3 *dst, VEC_3 *src);
VEC_4 *copy_v4(VEC_4 *dst, VEC_4 *src);
MAT_4_4 *copy_m(MAT_4_4 *dst, MAT_4_4 *src);
VEC_4 *add_vv(VEC_4 *a, VEC_4 *b);
VEC_4 *sub_vv(VEC_4 *a, VEC_4 *b);
VEC_4 *mul_vd(VEC_4 *a, FLOAT b);
VEC_4 *div_vd(VEC_4 *a, FLOAT b);
VEC_4 *cross_vv(VEC_4 *a, VEC_4 *b);
FLOAT dot_vv(VEC_4 *a, VEC_4 *b);
FLOAT length_v(VEC_4 *a);
VEC_4 *norm_v(VEC_4 *a);
VEC_4 *mul_mv(MAT_4_4 *a, VEC_4 *b);
MAT_4_4 *mul_mm(MAT_4_4 *a, MAT_4_4 *b);
MAT_4_4 *scale_m(FLOAT scale_x, FLOAT scale_y, FLOAT scale_z);
MAT_4_4 *transform_m(FLOAT ang_x, FLOAT ang_y, FLOAT ang_z, FLOAT dx, FLOAT dy, FLOAT dz, FLOAT sx, FLOAT sy, FLOAT sz);
MAT_4_4 *camera_m(VEC_4 *AT, VEC_4 *EYE, FLOAT roll);
MAT_4_4 *projection_m(FLOAT fov, FLOAT w, FLOAT h, FLOAT n, FLOAT f);

#endif