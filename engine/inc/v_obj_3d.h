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

#ifndef VECTOR_OBJ_3D_H
#define VECTOR_OBJ_3D_H

#include "engine_types.h"

OBJ_3D *obj_3d(INT vcnt, INT fcnt);
OBJ_3D *obj_3d_copy(OBJ_3D *src);
void obj_3d_free(OBJ_3D *obj);
void obj_3d_set_properties(OBJ_3D *obj, OBJ_3D *props);
void obj_3d_init_geometry(OBJ_3D *obj);

void obj_3d_determine_edges(OBJ_3D *obj);
void obj_3d_calc_face_normals(OBJ_3D *obj);
void obj_3d_calc_vertex_normals(OBJ_3D *obj);

void obj_3d_draw_wireframe(OBJ_3D *obj);
void obj_3d_draw_solid_unshaded(OBJ_3D *obj);
void obj_3d_draw_solid_shaded(OBJ_3D *obj);
void obj_3d_draw_interp_unshaded(OBJ_3D *obj);
void obj_3d_draw_interp_shaded(OBJ_3D *obj);
void obj_3d_draw_textured_base(OBJ_3D *obj);
void obj_3d_draw_textured_base_mul(OBJ_3D *obj);
void obj_3d_draw_textured_base_add(OBJ_3D *obj);
void obj_3d_draw_textured_base_mul_add(OBJ_3D *obj);
void obj_3d_draw_fake_reflection(OBJ_3D *obj);
void obj_3d_draw_bump_fake_reflection(OBJ_3D *obj);
void obj_3d_draw_solid_diff_textured(OBJ_3D *obj);
void obj_3d_draw_solid_spec_textured(OBJ_3D *obj);
void obj_3d_draw_solid_diff_spec_textured(OBJ_3D *obj);
void obj_3d_draw_interp_diff_textured(OBJ_3D *obj);
void obj_3d_draw_interp_spec_textured(OBJ_3D *obj);
void obj_3d_draw_interp_diff_spec_textured(OBJ_3D *obj);

#endif