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

#include <math.h>
#include <stdlib.h>

#include "engine_types.h"
#include "v_renderer.h"
#include "v_geometry.h"
#include "v_obj_3d_container.h"

SCENE_3D* scene_3d(RENDER_BUFFER* render_buf, INT max_objects, INT max_lights) {
    SCENE_3D *scene = calloc(1, sizeof(SCENE_3D));
    scene->render_buf = render_buf;
    scene->root = calloc(max_objects + max_lights, sizeof(OBJ_3D_CONTAINER*));
    scene->root_cnt = 0;
    scene->renderable = calloc(max_objects, sizeof(OBJ_3D_CONTAINER*));
    scene->renderable_cnt = 0;
    scene->light = calloc(max_lights, sizeof(OBJ_3D_CONTAINER*));
    scene->light_cnt = 0;
    copy_v4(&scene->camera.look_at, &(VEC_4){0.0, 0.0, 0.0, 0.0});
    copy_v4(&scene->camera.pos, &(VEC_4){0.0, 0.0, 0.0, 0.0});
    scene->camera.roll = 0.0;
    scene->camera.fov = 90.0;
    scene->camera.near_z = 0.5;
    scene->camera.far_z = 50.0;
    scene->light_settings.enabled = false;
    scene->light_settings.attenuation = 0.0;
    copy_v3(&scene->light_settings.ambient, &(VEC_3){0.0, 0.0, 0.0});
    copy_v3(&scene->light_settings.directional, &(VEC_3){0.0, 0.0, 0.0});
    copy_v4(&scene->light_settings.direction, &(VEC_4){0.0, 0.0, 0.0, 1.0});
    copy_v4(&scene->light_settings.direction_in_camera_space, &(VEC_4){0.0, 0.0, 0.0, 1.0});
    scene->camera.roll = 0.0;
    scene->rotate_all_objects_vertex_normals = false;
    return scene;
}

void scene_3d_free(SCENE_3D* scene) {
    for (INT i = 0; i < scene->root_cnt; i++) {
        obj_3d_container_free(scene->root[i]);
    }
    free(scene->root);
    free(scene->renderable);
    free(scene->light);
    free(scene);
}

void scene_3d_camera_set_settings(SCENE_3D *scene, CAMERA_SETTINGS *settings) {
    copy_v4(&scene->camera.look_at, &settings->look_at);
    copy_v4(&scene->camera.pos, &settings->pos);
    scene->camera.roll = settings->roll;
    if (settings->fov != 0)
        scene->camera.fov = settings->fov;
    if (settings->near_z != 0)
        scene->camera.near_z = settings->near_z;
    if (settings->far_z != 0)
        scene->camera.far_z = settings->far_z;
}

void scene_3d_lighting_set_settings(SCENE_3D *scene, GLOBAL_LIGHT_SETTINGS *settings) {
    scene->light_settings.enabled = settings->enabled;
    scene->light_settings.attenuation = settings->attenuation;
    copy_v3(&scene->light_settings.ambient, &settings->ambient);
    copy_v3(&scene->light_settings.directional, &settings->directional);
    copy_v4(&scene->light_settings.direction, &settings->direction);
}

void scene_3d_add_root_container(SCENE_3D *scene, OBJ_3D_CONTAINER *root) {
    root->scene = scene;
    scene->root[scene->root_cnt] = root;
    scene->root_cnt++;
    //TODO: do I REALLY need distinction/separate arrays for light and renderable objects?
    if (root->obj->type == POINT_LIGHTS) {
        scene->light[scene->light_cnt] = root;
        scene->light_cnt++;
    }
    else {
        scene->renderable[scene->renderable_cnt] = root;
        scene->renderable_cnt++;
    }
}

void scene_3d_add_child_container(OBJ_3D_CONTAINER *parent, OBJ_3D_CONTAINER *child) {
    SCENE_3D* scene = parent->scene;
    child->scene = scene;
    parent->child[parent->child_cnt] = child;
    parent->child_cnt++;
    child->parent = parent;

    if (child->obj->type == POINT_LIGHTS) {
        scene->light[scene->light_cnt] = child;
        scene->light_cnt++;
    }
    else {
        scene->renderable[scene->renderable_cnt] = child;
        scene->renderable_cnt++;
    }
}

void scene_3d_transform_and_light(SCENE_3D* scene) {
    MAT_4_4 camera_matrix;
    MAT_4_4 projection_matrix;
    VEC_4 *scene_zero_camera = NULL;
    INT i = 0;

    copy_m(&projection_matrix, projection_m(scene->camera.fov, scene->render_buf->width, scene->render_buf->height, scene->camera.near_z, scene->camera.far_z));
    copy_m(&camera_matrix, camera_m(&scene->camera.look_at, &scene->camera.pos, scene->camera.roll));

    // Transform directional light vector for this scene
    scene_zero_camera = mul_mv(&camera_matrix, &(VEC_4){0.0, 0.0, 0.0, 1.0});
    if (scene->light_settings.enabled) {
        copy_v4(&scene->light_settings.direction_in_camera_space,
            sub_vv(mul_mv(&camera_matrix, &scene->light_settings.direction), scene_zero_camera));
    }

    // Trigger matrix calculation for all root containers added to the scene.
    // Calculation for child containers is triggered internally in recursive manner.
    for (i = 0; i < scene->root_cnt; i++)
        obj_3d_container_calc_matrices(scene->root[i]);

    // Transform geometry for all point lights in the scene
    if (scene->light_settings.enabled)
        for (i = 0; i < scene->light_cnt; i++)
            obj_3d_container_transform_geometry(
                scene->light[i],
                &camera_matrix, &projection_matrix,
                scene->render_buf->width, scene->render_buf->height
            );

    // Transform geometry and render all renderable objects in the scene
    for (i = 0; i < scene->renderable_cnt; i++) {
        obj_3d_container_transform_geometry(
            scene->renderable[i],
            &camera_matrix, &projection_matrix,
            scene->render_buf->width, scene->render_buf->height
        );
        if (scene->light_settings.enabled)
            obj_3d_container_apply_light(scene->renderable[i]);
    }
}

void scene_3d_render(SCENE_3D* scene) {
    vr_set_render_buffer(scene->render_buf);
    for (INT i = 0; i < scene->renderable_cnt; i++) {
        obj_3d_container_render(scene->renderable[i]);
    }
}
