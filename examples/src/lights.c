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
#include "v_renderer.h"
#include "v_lighting.h"
#include "v_scene.h"
#include "assets_2d.h"

#define V1_COUNT (18*10)
#define V2_COUNT (6*10)
#define TOROID_CNT 3
#define LIGHT_CNT 3

#define SHADING_SWITCH_KEY 'q'
#define LIGHTS_SWITCH_KEY 'w'

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 3D Lighting example";
    EVENT *event = NULL;
    bool quit_flag = false;
    bool light_move_active = false; // Light movement animation state flag
    FLOAT light_move_t = 0.0; // Light movement animation time
    OBJ_3D_TYPE obj_3d_type = SOLID_DIFF_SPEC;
    OBJ_3D_CONTAINER* toroid_cont[TOROID_CNT];
    OBJ_3D_CONTAINER* light_markers[LIGHT_CNT];
    OBJ_3D_CONTAINER* lights[LIGHT_CNT];
    VEC_3 color_orange = {1.0, 0.5, 0.25};
    VEC_3 color_aquamarine = {0.5, 1.0, 0.25};
    VEC_3 color_light_blue = {0.25, 0.5, 1.0};
    VEC_3 color_red = {1.0, 0.0, 0.0};
    VEC_3 color_green = {0.0, 1.0, 0.0};
    VEC_3 color_blue = {0.0, 0.0, 1.0};

    printf("3D Lighting example\n");
    printf("Shading switch Gouraud/flat: %c\n", SHADING_SWITCH_KEY);
    printf("Lights switch move/stop: %c\n", LIGHTS_SWITCH_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    /* Building scene data structures */
    SCENE_3D* scene = scene_3d(display_buffer(), TOROID_CNT+LIGHT_CNT, LIGHT_CNT);
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, 25.0, 0.0},
        .roll = 45.0,    .fov = 90.0,    .near_z = 10.0,    .far_z = 30.0 });
    //Configure scene-wide lighting parameters
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = {0.1, 0.1, 0.1},
        .directional = {0.4, 0.4, 0.4},
        .direction = COMPOUND_4(*norm_v(&(VEC_4){1.0, 1.0, 0.0, 1.0})),
        .attenuation = 0.2}
    );

    // Create lights (each with 1 child slot for its marker)
    lights[0] = obj_3d_container(obj_3d_light(&color_blue), 1);
    lights[1] = obj_3d_container(obj_3d_light(&color_red), 1);
    lights[2] = obj_3d_container(obj_3d_light(&color_green), 1);

    //create light markers and add them along their corresponding lights to the scene
    for (int i=0; i<LIGHT_CNT; i++) {
        light_markers[i] = obj_3d_container(obj_3d_regular_polyhedron(DODECAHEDRON, 0.15), 0);
        scene_3d_add_root_container(scene, lights[i]);
        scene_3d_add_child_container(lights[i], light_markers[i]);
    }

    //create toroids and add them to the scene
    for (int i=0; i<TOROID_CNT; i++) {
        toroid_cont[i] = obj_3d_container(obj_3d_toroid(QUAD, 7.0, 3.0, V1_COUNT, V2_COUNT, 1, 1), 0);
        scene_3d_add_root_container(scene, toroid_cont[i]);
    }

    /** Initial demo settings */
    light_move_active = false; // Light movement animation state flag
    light_move_t = 0.0; // Light movement animation time
    obj_3d_type = SOLID_DIFF_SPEC;

    //Place toroids in the space
    obj_3d_container_set_transform(toroid_cont[0], 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    obj_3d_container_set_transform(toroid_cont[1], 0.0, 0.0, 0.0, 0.0, 7.0, 0.0);
    obj_3d_container_set_transform(toroid_cont[2], 0.0, 0.0, 0.0, 0.0, -7.0, 0.0);

    //Initialize all renderable objects properties
    obj_3d_set_properties(light_markers[0]->obj, &(OBJ_3D){
        .type = SOLID_UNSHADED,
        .surface_color = COMPOUND_3(color_blue) });
    obj_3d_set_properties(light_markers[1]->obj, &(OBJ_3D){
        .type = SOLID_UNSHADED,
        .surface_color = COMPOUND_3(color_red) });
    obj_3d_set_properties(light_markers[2]->obj, &(OBJ_3D){
        .type = SOLID_UNSHADED,
        .surface_color = COMPOUND_3(color_green) });
    obj_3d_set_properties(toroid_cont[0]->obj, &(OBJ_3D){
        .type = obj_3d_type,
        .surface_color = COMPOUND_3(color_orange),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroid_cont[1]->obj, &(OBJ_3D){
        .type = obj_3d_type,
        .surface_color = COMPOUND_3(color_aquamarine),
        .specular_power = 5.0 });
    obj_3d_set_properties(toroid_cont[2]->obj, &(OBJ_3D){
        .type = obj_3d_type,
        .surface_color = COMPOUND_3(color_light_blue),
        .specular_power = 5.0 });

    while (!quit_flag) {
        render_buffer_fill(display_buffer(), &(VEC_3){0.5, 0.5, 0.5});

        // Move the lights
        light_move_t += (light_move_active ? 1. : 0.)*display_last_frame_interval();
        obj_3d_container_set_transform(lights[0], 0.0, 0.0, 0.0,
            8.0*sin((light_move_t/15.0 + 0.1)*2*PI),
            8.0*sin((light_move_t/17.0 + 0.333)*2*PI),
            12.0);
        obj_3d_container_set_transform(lights[1], 0.0, 0.0, 0.0,
            8.0*sin((light_move_t/16.0 + 0.25)*2*PI),
            8.0*sin((light_move_t/13.0 + 0.0)*2*PI),
            12.0);
        obj_3d_container_set_transform(lights[2], 0.0, 0.0, 0.0,
            8.0*sin((light_move_t/17.5 + 0.6)*2*PI),
            8.0*sin((light_move_t/20.0 + 0.4)*2*PI),
            12.0);

        // Execute a lighting calculation and rendering for the whole scene
        scene_3d_transform_and_light(scene);
        scene_3d_render(scene);
        display_show(0);
        periodic_fps_printf(1.0);

        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_PRESSED) {
                switch((int)event->code) {
                    // Switch the rendering type depending on the user selection
                    case SHADING_SWITCH_KEY:
                        if (obj_3d_type == INTERP_DIFF_SPEC)
                            obj_3d_type = SOLID_DIFF_SPEC;
                        else
                            obj_3d_type = INTERP_DIFF_SPEC;
                        for (int i=0; i<TOROID_CNT; i++)
                            toroid_cont[i]->obj->type = obj_3d_type;
                        break;
                    case LIGHTS_SWITCH_KEY:
                        light_move_active = !light_move_active;
                        break;
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
    engine_cleanup();
    return 0;
}