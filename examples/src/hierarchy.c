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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "engine.h"
#include "v_geometry.h"
#include "v_obj_3d.h"
#include "v_obj_3d_container.h"
#include "v_obj_3d_generators.h"
#include "v_rasterizer.h"
#include "v_lighting.h"
#include "v_scene.h"
#include "maps.h"

typedef struct {
    FLOAT ax;
    FLOAT ay;
    FLOAT az;
} OBJECT_ANGLES;

#define HIERARCHY_LEVELS (5)
#define X_ANG_INC_KEY 'q'
#define X_ANG_DEC_KEY 'a'
#define Y_ANG_INC_KEY 'w'
#define Y_ANG_DEC_KEY 's'
#define Z_ANG_INC_KEY 'e'
#define Z_ANG_DEC_KEY 'd'

const int BRANCHES = 3;
const char *window_title = "SoRDIC 3D Object Hierarchy example";
const int V1_COUNT = (18*2);
const int V2_COUNT = (6*2);
const VEC_3 color_1 = {0.99, 0.747, 0.478};
const VEC_3 color_2 = {0.932, 0.797, 0.548};
const VEC_3 color_3 = {0.877, 0.847, 0.603};
const VEC_3 color_4 = {0.816, 0.898, 0.551};
const VEC_3 color_5 = {0.776, 0.958, 0.351};
const VEC_3 color_white = {0.8, 0.8, 0.8};
OBJ_3D* toroids[HIERARCHY_LEVELS];

// Function for building the containers tree in the scene
void hierarchy_level(OBJ_3D_CONTAINER* parent, int level) {
    if (level > HIERARCHY_LEVELS-1)
        return;
    OBJ_3D_CONTAINER* cont = NULL;
    FLOAT ang = 360.0/BRANCHES;
    FLOAT d = 6.0 * pow(0.5, level-1);
    for (int i = 0; i < BRANCHES; i++) {
        // Every container in the scene contains the copy
        // of one of the preallocated toroid objects
        cont = obj_3d_container(obj_3d_copy(toroids[level]), BRANCHES);
        // Attach this level (child) container to the upper level (parent) container
        scene_3d_add_child_container(parent, cont);
        // Add individual transformation to this container
        obj_3d_container_set_transform(cont,
            0.0, i*ang, 0.0,
            d*cos(i*ang*PI/180.0), 2.0, d*sin(i*ang*PI/180.0),
            1.0, 1.0, 1.0
        );
        // Invoke recursively next level
        hierarchy_level(cont, level+1);
    }
}

int main(int argc, char *argv[])
{
    int i = 0;
    EVENT *event = NULL;
    bool quit_flag = false;
    FLOAT omega = 10.0; // A rotation speed of the objects in degrees/s
    SCENE_3D* scene = NULL;
    OBJ_3D_CONTAINER* container = NULL;
    int container_cnt = 0;
    FLOAT cax = 0.0, cay = 0.0, caz = 0.0;
    FLOAT pax = 0.0, pay = 0.0, paz = 0.0;
    OBJECT_ANGLES *object_angles = NULL;

    printf("3D Object Hierarchy example\n");
    printf("X rotation angle increment: %c, decrement: %c\n", X_ANG_INC_KEY, X_ANG_DEC_KEY);
    printf("Y rotation angle increment: %c, decrement: %c\n", Y_ANG_INC_KEY, Y_ANG_DEC_KEY);
    printf("Z rotation angle increment: %c, decrement: %c\n", Z_ANG_INC_KEY, Z_ANG_DEC_KEY);
    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    //determine total number of toroids in this scene
    for (i = 0; i < HIERARCHY_LEVELS; i++) {
        container_cnt += pow(BRANCHES, i);
    }
    //array for initial orientation of all objects in this scene
    object_angles = calloc(container_cnt, sizeof(OBJECT_ANGLES));

    // Allocate the scene for container_cnt renderble objects
    scene = scene_3d(display_buffer(), container_cnt, 0);
    // The camera in this scene is still
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},
        .pos = {0.0, 0.0, 15.0, 0.0},
        .roll = 0.0, .fov = 90.0,
        .near_z = 1.0, .far_z = 30.0 }
    );
    //Configure scene-wide lighting
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = {0.2, 0.2, 0.2},
        .directional = COMPOUND_3(color_white),
        .direction = COMPOUND_4(*norm_v(&(VEC_4){0.0, 0.0, -1.0, 1.0}))}
    );

    // Build different size toroids that will compose the scene
    toroids[0] = obj_3d_toroid(QUAD, 2.25, 0.75, V1_COUNT, V2_COUNT, 1, 1);
    toroids[1] = obj_3d_toroid(QUAD, 1.5, 0.5, V1_COUNT, V2_COUNT, 1, 1);
    toroids[2] = obj_3d_toroid(QUAD, 1.0, 0.333, V1_COUNT, V2_COUNT, 1, 1);
    toroids[3] = obj_3d_toroid(QUAD, 0.666, 0.222, V1_COUNT/2, V2_COUNT/2, 1, 1);
    toroids[4] = obj_3d_toroid(QUAD, 0.444, 0.148, V1_COUNT/2, V2_COUNT/2, 1, 1);
    obj_3d_set_properties(toroids[0], &(OBJ_3D){
        .type = SOLID_DIFF_SPEC,
        .surface_color = COMPOUND_3(color_1),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroids[1], &(OBJ_3D){
        .type = SOLID_DIFF_SPEC,
        .surface_color = COMPOUND_3(color_2),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroids[2], &(OBJ_3D){
        .type = SOLID_DIFF_SPEC,
        .surface_color = COMPOUND_3(color_3),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroids[3], &(OBJ_3D){
        .type = SOLID_DIFF_SPEC,
        .surface_color = COMPOUND_3(color_4),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroids[4], &(OBJ_3D){
        .type = SOLID_DIFF_SPEC,
        .surface_color = COMPOUND_3(color_5),
        .specular_power = 5.0 });

    // Build the root object of the scene tree
    container = obj_3d_container(obj_3d_copy(toroids[0]), BRANCHES);
    scene_3d_add_root_container(scene, container);
    obj_3d_container_set_transform(container, 45.0, 0.0, 0.0, 0.0, -2.0, -3.0, 1.0, 1.0, 1.0);
    // Invoke the tree construction process for the remaining tree levels
    hierarchy_level(container, 1);

    //Store individual orientations for all the objects in the scene
    //Later it will be used for animation purposes
    for (i = 0; i < scene->renderable_cnt; i++) {
        object_angles[i].ax = scene->renderable[i]->ax;
        object_angles[i].ay = scene->renderable[i]->ay;
        object_angles[i].az = scene->renderable[i]->az;
    }

    while (!quit_flag) {
        render_buffer_zero(display_buffer());

        // Update all renderable object transformations
        for (i=0; i<scene->renderable_cnt; i++) {
            obj_3d_container_set_transform(
                scene->renderable[i],
                object_angles[i].ax + cax + pax,
                object_angles[i].ay + cay + pay,
                object_angles[i].az + caz + paz,
                scene->renderable[i]->px,
                scene->renderable[i]->py,
                scene->renderable[i]->pz,
                1.0, 1.0, 1.0);
        }

        // Execute a lighting calculation and rendering for the whole scene
        scene_3d_transform_and_light(scene);
        scene_3d_render(scene);
        display_show(0);
        periodic_fps_printf(1.0);
        cax = cay = caz = 0;
        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_HOLD) {
                switch((int)event->code) {
                    case X_ANG_INC_KEY:
                        cax += event->hold_time*omega; break;
                    case X_ANG_DEC_KEY:
                        cax -= event->hold_time*omega; break;
                    case Y_ANG_INC_KEY:
                        cay += event->hold_time*omega; break;
                    case Y_ANG_DEC_KEY:
                        cay -= event->hold_time*omega; break;
                    case Z_ANG_INC_KEY:
                        caz += event->hold_time*omega; break;
                    case Z_ANG_DEC_KEY:
                        caz -= event->hold_time*omega; break;
                    default:
                        break;
                }
            }
            else if (event->type == KEY_RELEASED) {
                switch((int)event->code) {
                    case X_ANG_INC_KEY:
                        cax = 0.0;   pax += event->hold_time*omega; break;
                    case X_ANG_DEC_KEY:
                        cax = 0.0;   pax -= event->hold_time*omega; break;
                    case Y_ANG_INC_KEY:
                        cay = 0.0;   pay += event->hold_time*omega; break;
                    case Y_ANG_DEC_KEY:
                        cay = 0.0;   pay -= event->hold_time*omega; break;
                    case Z_ANG_INC_KEY:
                        caz = 0.0;   paz += event->hold_time*omega; break;
                    case Z_ANG_DEC_KEY:
                        caz = 0.0;   paz -= event->hold_time*omega; break;
                    default:
                        break;
                }
            }
            event++; //skip to next event
        }
        #ifdef RUN_ONE_FRAME
            quit_flag = true;
        #endif
    }
    printf("\n");

    scene_3d_free(scene);
    for (i = 0; i < HIERARCHY_LEVELS; i++) {
        obj_3d_free(toroids[i]);
    }
    free(object_angles);
    engine_cleanup();
    return 0;
}