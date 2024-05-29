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

#define SURFACE_1_KEY '1'
#define SURFACE_2_KEY '2'
#define SURFACE_3_KEY '3'
#define SURFACE_4_KEY '4'
#define SURFACE_5_KEY '5'
#define SURFACE_6_KEY '6'
#define SURFACE_7_KEY '7'
#define SURFACE_8_KEY '8'

#define ROTATION_TOGGLE_KEY 'q'
#define WIREFRAME_TOGGLE_KEY 'w'
#define ANIMATION_TOGGLE_KEY 'e'

#define SOLID_UNSHADED_KEY 'r'
#define SOLID_KEY 't'
#define INTERP_KEY 'y'
#define TEXTURE_MAP_KEY 'u'
#define REFLECTION_MAP_KEY 'i'

#define ASSETS_DIR "assets/"
#define WOOD_MAP "wood_1024.jpg"
#define RED_LIGHT_MAP "lightmap_red_512.jpg"

const FLOAT wave_amp = 0.1;
const FLOAT wave_num = 3;
const FLOAT R1 = 0.4;
const FLOAT R2 = 0.1;
FLOAT surface_animation_t = 0.0; // Current animation time
VEC_4 wave_f;
VEC_4 wave_t;

/*
    u - input u coordinate in range [0.0 - 1.0]
    v - input v coordinate in range [0.0 - 1.0]
    vertex - output VERTEX instance to store resulting point
*/
VEC_4* wave_surface(FLOAT u, FLOAT v) {
    const FLOAT p = 2*PI*6/1.4142; //6 sine periods from corner to corner
    const FLOAT s1u = (u-0.2)*p;
    const FLOAT s1v = (v-0.2)*p;
    const FLOAT dist1 = sqrt(s1u*s1u+s1v*s1v);
    const FLOAT s2u = (u-0.2)*p;
    const FLOAT s2v = (v-0.8)*p;
    const FLOAT dist2 = sqrt(s2u*s2u+s2v*s2v);
    const FLOAT wave_omega = 4.0; //wave angular speed
    return store_v4(&(VEC_4){
        u-0.5, v-0.5, 0.25*(sin(dist1)+sin(dist2-wave_omega*surface_animation_t)), 1.0}
    );
}

VEC_4* wave_function(FLOAT u) {
    wave_f[0] = R1*cos(u);
    wave_f[1] = wave_amp*cos(u*wave_num);
    wave_f[2] = R1*sin(u);
    wave_f[3] = 1.0;
    return (VEC_4*)wave_f;
}
VEC_4* wave_tangent(FLOAT u) {
    wave_t[0] = R1*-sin(u);
    wave_t[1] = wave_amp*wave_num*-sin(u*wave_num);
    wave_t[2] = R1*cos(u);
    wave_t[3] = 1.0;
    return norm_v((VEC_4*)wave_t);
}

VEC_4* fancy_toroid(FLOAT u, FLOAT v) {
    u *= 6.2832;
    v *= 6.2832;

    VEC_4 *W = wave_function(u);
    VEC_4 *R = norm_v(W); //unit radius vector
    VEC_4 *T = wave_tangent(u);
    VEC_4 *S = cross_vv(R, T); //othogonal vector to RT plane
    //slice cross-section is located in the plane of the vectors R and S

    FLOAT B = 0.025*cos(u*12)*cos(v*4)+R2;
    return add_vv(add_vv(mul_vd(R, B*cos(v)), mul_vd(S, B*-sin(v))), W);
}

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 3D Parametric Surface Example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int i = 0;
    FLOAT rotation_t = 0.0; // Current rotation time
    FLOAT omega_x, omega_y, omega_z; //object angular velocities (constant)
    bool rotation_on = false, surface_animation_on = false, wireframe_on = false;
    FLOAT a_x, a_y, a_z; //object initial angles
    FLOAT p_x, p_y, p_z; //object position
    OBJ_3D_TYPE render_3d_type = SOLID_UNSHADED;
    VEC_3 color_blue = {1.0, 0.5, 0.25};
    VEC_3 color_white = {0.8, 0.8, 0.8};
    VEC_3 wire_color = {1.0, 1.0, 1.0};
    const int OBJECTS_COUNT = 8;
    OBJ_3D *objects[OBJECTS_COUNT];
    RGB_MAP *wood_map = NULL, *red_light_map = NULL;
    SCENE_3D *scene = NULL;
    OBJ_3D_CONTAINER *container = NULL;

    printf("3D Parametric Surface example\n");
    printf("Object type: %c, %c, %c, %c, %c, %c, %c, %c\n", 
            SURFACE_1_KEY, SURFACE_2_KEY, SURFACE_3_KEY, SURFACE_4_KEY,
            SURFACE_5_KEY, SURFACE_6_KEY, SURFACE_7_KEY, SURFACE_8_KEY);
    printf("Rotation on/off: %c, Wireframe on/off: %c, Animation on/off: %c\n",
            ROTATION_TOGGLE_KEY, WIREFRAME_TOGGLE_KEY, ANIMATION_TOGGLE_KEY);
    printf("No shading: %c, Flat shading: %c, Gouraud shading: %c, Texture mapping: %c, Reflection mapping: %c\n",
            SOLID_UNSHADED_KEY, SOLID_KEY, INTERP_KEY, TEXTURE_MAP_KEY, REFLECTION_MAP_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    wood_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR WOOD_MAP), 50);
    red_light_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR RED_LIGHT_MAP), 0);
    /* Building scene data structures */
    scene = scene_3d(display_buffer(), 1, 0); // Allocate the scene for 1 renderable object
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, -4.0, 0.0},
        .roll = 0.0,    .fov = 90.0,    .near_z = 0.5,    .far_z = 8.0 });
    //Configure scene-wide lighting
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = {0.2, 0.2, 0.2},
        .directional = COMPOUND_3(color_white),
        .direction = COMPOUND_4(*norm_v(&(VEC_4){0.0, 0.0, 1.0, 1.0})) });
    container = obj_3d_container(NULL, 0);


    //Create sine wave surfaces
    const int SURF_VU = 121; //Number of vertices along U axis
    const int SURF_VV = 121; //Number of vertices along V axis
    const FLOAT SURF_S = 4.5; //Scaling factor
    objects[0] = obj_3d_uv_mesh(QUAD, NO_WRAP, SURF_VU, SURF_VV, 3, 3);
    objects[1] = obj_3d_uv_mesh(RIGHT_TRIANGLE, NO_WRAP, SURF_VU, SURF_VV, 3, 3);
    objects[2] = obj_3d_uv_mesh(EQUILATERAL_TRIANGLE, NO_WRAP, SURF_VU, SURF_VV, 3, 3);
    objects[3] = obj_3d_uv_mesh(WIDE_TRIANGLE, NO_WRAP, SURF_VU, SURF_VV, 3, 3);
    obj_3d_uv_surface_parametric(objects[0], wave_surface, SURF_S, SURF_S, 0.5);
    obj_3d_uv_surface_parametric(objects[1], wave_surface, SURF_S, SURF_S, 0.5);
    obj_3d_uv_surface_parametric(objects[2], wave_surface, SURF_S, SURF_S, 0.5);
    obj_3d_uv_surface_parametric(objects[3], wave_surface, SURF_S, SURF_S, 0.5);

    //Create sine-deformed toroid objects
    const int TOR_VU = 300;
    const int TOR_VV = 100;
    const FLOAT TOR_S = 4.5;
    objects[4] = obj_3d_uv_mesh(QUAD, WRAP_UV, TOR_VU, TOR_VV, 5, 1);
    objects[5] = obj_3d_uv_mesh(RIGHT_TRIANGLE, WRAP_UV, TOR_VU, TOR_VV, 5, 1);
    objects[6] = obj_3d_uv_mesh(EQUILATERAL_TRIANGLE, WRAP_UV, TOR_VU, TOR_VV, 5, 1);
    objects[7] = obj_3d_uv_mesh(WIDE_TRIANGLE, WRAP_UV, TOR_VU, TOR_VV, 5, 1);
    obj_3d_uv_surface_parametric(objects[4], fancy_toroid, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_parametric(objects[5], fancy_toroid, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_parametric(objects[6], fancy_toroid, TOR_S, TOR_S, TOR_S);
    obj_3d_uv_surface_parametric(objects[7], fancy_toroid, TOR_S, TOR_S, TOR_S);

    /** Initial demo settings */
    container->obj = objects[5];
    scene_3d_add_root_container(scene, container);

    a_x = 60.0; a_y = 0.0; a_z = 0.0; //Displayed object orientation
    rotation_on = false;
    wireframe_on = false;
    surface_animation_on = false;
    render_3d_type = SOLID_DIFF_SPEC;
    rotation_t = surface_animation_t = 0.0;
    //Initial position and angular velocities for deformed toroid objects.
    p_x = 0.0; p_y = 0.0; p_z = 0.0;
    omega_x=8.0; omega_y=16.0; omega_z=24.0;
    //initialize all objects properties
    for (i = 0; i < OBJECTS_COUNT; i++) {
        obj_3d_set_properties((OBJ_3D*)objects[i], &(OBJ_3D){
            .type = render_3d_type,
            .surface_color = COMPOUND_3(color_blue),
            .wireframe_color = COMPOUND_3(wire_color),
            .base_map = wood_map,
            .reflection_map = red_light_map,
            .specular_power = 5.0,
            .wireframe_on = wireframe_on}
        );
    }

    while (!quit_flag) {
        render_buffer_zero(display_buffer());

        //rotate and perspective transform the cube
        obj_3d_container_set_transform(
            container,
            a_x + omega_x*rotation_t,
            a_y + omega_y*rotation_t,
            a_z + omega_z*rotation_t,
            p_x, p_y, p_z,
            1.0, 1.0, 1.0);

        // Execute a lighting calculation and rendering for the whole scene
        scene_3d_transform_and_light(scene);
        scene_3d_render(scene);
        display_show(0);
        periodic_fps_printf(1.0);

        rotation_t += (rotation_on ? 1. : 0.)*display_last_frame_interval();
        if (surface_animation_on && (
            container->obj == objects[0] ||
            container->obj == objects[1] ||
            container->obj == objects[2] ||
            container->obj == objects[3])) {
            surface_animation_t += display_last_frame_interval();
            obj_3d_uv_surface_parametric(container->obj, wave_surface, SURF_S, SURF_S, 0.5);
        }

        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_PRESSED) {
                switch((int)event->code) {
                    // Switch the rendering type depending on the user selection
                    case SURFACE_1_KEY:
                        container->obj = objects[0];
                        p_x = 0.0; p_y = 0.5; p_z = 0.0;
                        omega_x=0.0; omega_y=0.0; omega_z=15.0;
                        obj_3d_uv_surface_parametric(container->obj, wave_surface, SURF_S, SURF_S, 0.5);
                        break;
                    case SURFACE_2_KEY:
                        container->obj = objects[1];
                        p_x = 0.0; p_y = 0.5; p_z = 0.0;
                        omega_x=0.0; omega_y=0.0; omega_z=15.0;
                        obj_3d_uv_surface_parametric(container->obj, wave_surface, SURF_S, SURF_S, 0.5);
                        break;
                    case SURFACE_3_KEY:
                        container->obj = objects[2];
                        p_x = 0.0; p_y = 0.5; p_z = 0.0;
                        omega_x=0.0; omega_y=0.0; omega_z=15.0;
                        obj_3d_uv_surface_parametric(container->obj, wave_surface, SURF_S, SURF_S, 0.5);
                        break;
                    case SURFACE_4_KEY:
                        container->obj = objects[3];
                        p_x = 0.0; p_y = 0.5; p_z = 0.0;
                        omega_x=0.0; omega_y=0.0; omega_z=15.0;
                        obj_3d_uv_surface_parametric(container->obj, wave_surface, SURF_S, SURF_S, 0.5);
                        break;
                    case SURFACE_5_KEY:
                        container->obj = objects[4];
                        p_x = 0.0; p_y = 0.0; p_z = 0.0;
                        omega_x=8.0; omega_y=16.0; omega_z=24.0;
                        break;
                    case SURFACE_6_KEY:
                        container->obj = objects[5];
                        p_x = 0.0; p_y = 0.0; p_z = 0.0;
                        omega_x=8.0; omega_y=16.0; omega_z=24.0;
                        break;
                    case SURFACE_7_KEY:
                        container->obj = objects[6];
                        p_x = 0.0; p_y = 0.0; p_z = 0.0;
                        omega_x=8.0; omega_y=16.0; omega_z=24.0;
                        break;
                    case SURFACE_8_KEY:
                        container->obj = objects[7];
                        p_x = 0.0; p_y = 0.0; p_z = 0.0;
                        omega_x=8.0; omega_y=16.0; omega_z=24.0;
                        break;
                    case ROTATION_TOGGLE_KEY:
                        rotation_on = !rotation_on;
                        break;
                    case WIREFRAME_TOGGLE_KEY:
                        wireframe_on = !wireframe_on;
                        break;
                    case ANIMATION_TOGGLE_KEY:
                        surface_animation_on = !surface_animation_on;
                        break;
                    case SOLID_UNSHADED_KEY:
                        render_3d_type = SOLID_UNSHADED;
                        break;
                    case SOLID_KEY:
                        render_3d_type = SOLID_DIFF_SPEC;
                        break;
                    case INTERP_KEY:
                        render_3d_type = INTERP_DIFF_SPEC;
                        break;
                    case TEXTURE_MAP_KEY:
                        render_3d_type = TX_MAP_BASE;
                        break;
                    case REFLECTION_MAP_KEY:
                        render_3d_type = REFLECTION;
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
    rgb_map_free(wood_map);    rgb_map_free(red_light_map);

    engine_cleanup();
    return 0;
}