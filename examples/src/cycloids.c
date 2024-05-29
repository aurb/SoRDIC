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
#include "v_rasterizer.h"
#include "v_geometry.h"
#include "v_obj_3d.h"
#include "v_obj_3d_container.h"
#include "v_obj_3d_generators.h"
#include "v_lighting.h"
#include "v_scene.h"
#include "maps.h"

#define CYCLOID_1_KEY '1'
#define CYCLOID_2_KEY '2'
#define CYCLOID_3_KEY '3'
#define CYCLOID_4_KEY '4'

#define ROTATION_TOGGLE_KEY 'q'
#define WIREFRAME_TOGGLE_KEY 'w'
#define ANIMATION_TOGGLE_KEY 'e'

#define SOLID_UNSHADED_KEY 'r'
#define SOLID_KEY 't'

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 3D Cycloids Example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int i = 0;
    FLOAT rotation_t = 0.0; // Current rotation time
    FLOAT animation_t = 0.0;
    FLOAT omega_x, omega_y, omega_z; //object angular velocities (constant)
    bool rotation_on = false, animation_on = false, wireframe_on = false;
    FLOAT a_x, a_y, a_z; //object initial angles
    FLOAT p_x, p_y, p_z; //object position
    OBJ_3D_TYPE render_3d_type = SOLID_UNSHADED;
    VEC_3 color_blue = {1.0, 0.5, 0.25};
    VEC_3 color_white = {0.8, 0.8, 0.8};
    VEC_3 wire_color = {1.0, 1.0, 1.0};
    const int OBJECTS_COUNT = 4;
    OBJ_3D *objects[OBJECTS_COUNT];
    SCENE_3D *scene = NULL;
    OBJ_3D_CONTAINER *scene_container = NULL;

    printf("3D Cycloids example\n");
    printf("Object type: %c, %c, %c\n", 
            CYCLOID_1_KEY, CYCLOID_2_KEY, CYCLOID_3_KEY);
    printf("Rotation on/off: %c, Wireframe on/off: %c, Animation on/off: %c\n",
            ROTATION_TOGGLE_KEY, WIREFRAME_TOGGLE_KEY, ANIMATION_TOGGLE_KEY);
    printf("No shading: %c, Flat shading: %c\n",
            SOLID_UNSHADED_KEY, SOLID_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    /* Building scene data structures */
    scene = scene_3d(display_buffer(), 1, 0); // Allocate the scene for 1 renderable object
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, -4.0, 0.0},
        .roll = 0.0,    .fov = 90.0,    .near_z = 1.0,    .far_z = 7.0 });
    //Configure scene-wide lighting
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = {0.2, 0.2, 0.2},
        .directional = COMPOUND_3(color_white),
        .direction = COMPOUND_4(*norm_v(&(VEC_4){0.0, 0.0, 1.0, 1.0})) });
    scene_container = obj_3d_container(NULL, 0);

/*
interesting presets
    const FLOAT C1 = -1.0;
    const FLOAT C2 = -7.0;
    const FLOAT C3 = -28.0;
    const FLOAT R1 = 0.30;
    const FLOAT R2 = 0.15;
    const FLOAT R3 = 0.075;
*/

    FLOAT C1 = 0.0, C2 = 0.0, C3 = 0.0;
    FLOAT R1 = 0.0, R2 = 0.0, R3 = 0.0, H = 0.0;

    C1 = 1.0;    C2 = 20.0;
    R1 = 0.3;    R2 = 0.18;
    CYCLOID cycloid_1 = {
        .phi = 0.01,
        .x.cnt = 2,
        .x.amp[0] = R1, .x.frq[0] = C1, .x.phs[0] = PI_2,
        .x.amp[1] = R2, .x.frq[1] = C2, .x.phs[1] = PI_2,
        .y.cnt = 0,
        .z.cnt = 2,
        .z.amp[0] = R1, .z.frq[0] = C1, .z.phs[0] = 0.0,
        .z.amp[1] = R2, .z.frq[1] = C2, .z.phs[1] = 0.0,
    };

    C1 = 1.0;    C2 = 20.0;    C3 = 5.0;
    R1 = 0.3;    R2 = 0.18;    H = 0.0;
    CYCLOID cycloid_2 = {
        .phi = 0.01,
        .x.cnt = 2,
        .x.amp[0] = R1, .x.frq[0] = C1, .x.phs[0] = PI_2,
        .x.amp[1] = R2, .x.frq[1] = C2, .x.phs[1] = PI_2,
        .y.cnt = 1,
        .y.amp[0] = H, .y.frq[0] = C3, .y.phs[0] = 0.0,
        .z.cnt = 2,
        .z.amp[0] = R1, .z.frq[0] = C1, .z.phs[0] = 0.0,
        .z.amp[1] = R2, .z.frq[1] = C2, .z.phs[1] = 0.0,
    };

    C1 = -1.0;    C2 = -7.0;    C3 = -28.0;
    R1 = 0.30;    R2 = 0.15;    R3 = 0.075;
    CYCLOID cycloid_3 = {
        .phi = 0.01,
        .x.cnt = 3,
        .x.amp[0] = R1, .x.frq[0] = C1, .x.phs[0] = PI_2,
        .x.amp[1] = R2, .x.frq[1] = C2, .x.phs[1] = PI_2,
        .x.amp[2] = R3, .x.frq[2] = C3, .x.phs[2] = PI_2,
        .y.cnt = 0,
        .z.cnt = 3,
        .z.amp[0] = R1, .z.frq[0] = C1, .z.phs[0] = 0.0,
        .z.amp[1] = R2, .z.frq[1] = C2, .z.phs[1] = 0.0,
        .z.amp[2] = R3, .z.frq[2] = C3, .z.phs[2] = 0.0,
    };

    C1 = 1.0;    C2 = 10.0;    C3 = 7.0;
    R1 = 0.3;    R2 = 0.19;    R3 = 0.19;
    CYCLOID cycloid_4 = {
        .phi = 0.01,
        .x.cnt = 2,
        .x.amp[0] = R1, .x.frq[0] = C1, .x.phs[0] = PI_2,
        .x.amp[1] = R2, .x.frq[1] = C2, .x.phs[1] = PI_2,
        .y.cnt = 2,
        .y.amp[0] = R1, .y.frq[0] = C1, .y.phs[0] = 0.0,
        .y.amp[1] = R3, .y.frq[1] = C3, .y.phs[1] = PI_2,
        .z.cnt = 2,
        .z.amp[0] = R1, .z.frq[0] = C1, .z.phs[0] = 0.0,
        .z.amp[1] = R2, .z.frq[1] = C2, .z.phs[1] = 0.0,
    };

    //Create sine-deformed toroid objects
    const int OBJ_VU = 1000;
    const int OBJ_VV = 4;
    const FLOAT TOR_S = 4.5;
    objects[0] = obj_3d_uv_mesh(QUAD, WRAP_UV, OBJ_VU, OBJ_VV, 1, 1);
    objects[1] = obj_3d_uv_mesh(QUAD, WRAP_UV, OBJ_VU, OBJ_VV, 1, 1);
    objects[2] = obj_3d_uv_mesh(QUAD, WRAP_UV, OBJ_VU, OBJ_VV, 1, 1);
    objects[3] = obj_3d_uv_mesh(QUAD, WRAP_UV, OBJ_VU, OBJ_VV, 1, 1);
    obj_3d_uv_surface_cycloid(objects[0], &cycloid_1, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_cycloid(objects[1], &cycloid_2, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_cycloid(objects[2], &cycloid_3, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_cycloid(objects[3], &cycloid_4, TOR_S, TOR_S, TOR_S);

    scene_container->obj = objects[0];
    scene_3d_add_root_container(scene, scene_container);

    //Displayed object position/orientation/movement parameters
    a_x = 90.0; a_y = 0.0; a_z = 0.0; // orientation
    p_x = 0.0; p_y = 0.0; p_z = 0.0;
    omega_x=8.0; omega_y=16.0; omega_z=24.0;

    /** Initial demo settings */
    rotation_on = false;
    wireframe_on = false;
    animation_on = false;
    render_3d_type = SOLID_DIFF_SPEC;
    rotation_t = 0.0;
    animation_t = 0.0;
    //initialize all objects properties
    for (i = 0; i < OBJECTS_COUNT; i++) {
        obj_3d_set_properties((OBJ_3D*)objects[i], &(OBJ_3D){
            .type = render_3d_type,
            .surface_color = COMPOUND_3(color_blue),
            .wireframe_color = COMPOUND_3(wire_color),
            .specular_power = 1.0,
            .wireframe_on = wireframe_on}
        );
    }

    while (!quit_flag) {
        render_buffer_fill(display_buffer(), &(VEC_3){0.1, 0.2, 0.3});

        //rotate and perspective transform the cube
        obj_3d_container_set_transform(
            scene_container,
            a_x + omega_x*rotation_t,
            a_y + omega_y*rotation_t,
            a_z + omega_z*rotation_t,
            p_x, p_y, p_z, 1.0, 1.0, 1.0);

        // Execute a lighting calculation and rendering for the whole scene
        scene_3d_transform_and_light(scene);
        scene_3d_render(scene);
        display_show(0);
        periodic_fps_printf(1.0);

        rotation_t += (rotation_on ? 1. : 0.)*display_last_frame_interval();
        animation_t += (animation_on ? 1. : 0.)*display_last_frame_interval();

        if (animation_on) {
            if (scene_container->obj == objects[1]) {
                cycloid_2.y.amp[0] = 0.5*sin(animation_t*PI/3);
                obj_3d_uv_surface_cycloid(objects[1], &cycloid_2, TOR_S, TOR_S, TOR_S);
            }
            else if (scene_container->obj == objects[2]) {
                cycloid_3.x.amp[1] = 0.15 + 0.05*sin(animation_t*PI_4);
                cycloid_3.z.amp[1] = 0.15 + 0.05*sin(animation_t*PI_4);
                cycloid_3.x.phs[2] = PI_2 - animation_t*PI/3;
                cycloid_3.z.phs[2] = - animation_t*PI/3;
                obj_3d_uv_surface_cycloid(objects[2], &cycloid_3, TOR_S, TOR_S, TOR_S);
            }
            else if (scene_container->obj == objects[3]) {
                FLOAT c_phs = animation_t*PI/2;
                cycloid_4.x.phs[1] = PI_2 + c_phs;
                cycloid_4.z.phs[1] = c_phs;
                obj_3d_uv_surface_cycloid(objects[3], &cycloid_4, TOR_S, TOR_S, TOR_S);
            }
        }

        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_PRESSED) {
                switch((int)event->code) {
                    // Switch the rendering type depending on the user selection
                    case CYCLOID_1_KEY:
                        scene_container->obj = objects[0];
                        break;
                    case CYCLOID_2_KEY:
                        scene_container->obj = objects[1];
                        break;
                    case CYCLOID_3_KEY:
                        scene_container->obj = objects[2];
                        break;
                    case CYCLOID_4_KEY:
                        scene_container->obj = objects[3];
                        break;
                    case ROTATION_TOGGLE_KEY:
                        rotation_on = !rotation_on;
                        break;
                    case WIREFRAME_TOGGLE_KEY:
                        wireframe_on = !wireframe_on;
                        break;
                    case ANIMATION_TOGGLE_KEY:
                        animation_on = !animation_on;
                        break;
                    case SOLID_UNSHADED_KEY:
                        render_3d_type = SOLID_UNSHADED;
                        break;
                    case SOLID_KEY:
                        render_3d_type = SOLID_DIFF_SPEC;
                        break;
                    default:
                        break;
                }
                for (i = 0; i<OBJECTS_COUNT; i++) {
                    objects[i]->type = render_3d_type;
                    objects[i]->wireframe_on = wireframe_on;
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
    for (i = 0; i < OBJECTS_COUNT; i++) {
        obj_3d_free(objects[i]);
    }
    engine_cleanup();
    return 0;
}