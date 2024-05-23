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

#ifndef VECTOR_SCENE_H
#define VECTOR_SCENE_H

#include "engine_types.h"

SCENE_3D* scene_3d(RENDER_BUFFER* render_buf, INT max_objects, INT max_lights);
void scene_3d_free(SCENE_3D* scene);
void scene_3d_camera_set_settings(SCENE_3D *scene, CAMERA_SETTINGS *settings);
void scene_3d_lighting_set_settings(SCENE_3D *scene, GLOBAL_LIGHT_SETTINGS *settings);
void scene_3d_add_root_container(SCENE_3D *scene, OBJ_3D_CONTAINER *root);
void scene_3d_add_child_container(OBJ_3D_CONTAINER *parent, OBJ_3D_CONTAINER *child);
void scene_3d_transform_and_light(SCENE_3D* scene);
void scene_3d_render(SCENE_3D* scene);

#endif