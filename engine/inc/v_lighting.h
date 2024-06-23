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

#ifndef VECTOR_LIGHTS_H
#define VECTOR_LIGHTS_H

#include "engine_types.h"

void lighting_face_calculation(SCENE_3D *scene, OBJ_3D *obj);
void lighting_vertex_calculation(SCENE_3D *scene, OBJ_3D *obj);

void lighting_face_coloring_diffuse(OBJ_3D *obj);
void lighting_vertex_coloring_diffuse(OBJ_3D *obj);
void lighting_face_coloring_specular(OBJ_3D *obj);
void lighting_vertex_coloring_specular(OBJ_3D *obj);
void lighting_face_coloring_merge(OBJ_3D *obj);
void lighting_vertex_coloring_merge(OBJ_3D *obj);

#endif