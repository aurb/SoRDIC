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

#ifndef VECTOR_CONTAINER_H
#define VECTOR_CONTAINER_H

#include "engine_types.h"

OBJ_3D_CONTAINER *obj_3d_container(OBJ_3D *obj, INT max_children);
void obj_3d_container_free(OBJ_3D_CONTAINER *cont);
void obj_3d_container_set_transform(OBJ_3D_CONTAINER *cont, FLOAT ax, FLOAT ay, FLOAT az, FLOAT px, FLOAT py, FLOAT pz);
void obj_3d_container_calc_matrices(OBJ_3D_CONTAINER *cont);
void obj_3d_container_transform_geometry(OBJ_3D_CONTAINER *cont, MAT_4_4 *camera_space, MAT_4_4 *projection_space, INT scr_w, INT scr_h);
void obj_3d_container_apply_light(OBJ_3D_CONTAINER *cont);
void obj_3d_container_render(OBJ_3D_CONTAINER *cont);

#endif