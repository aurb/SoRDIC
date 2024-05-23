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
#include <string.h>
#include "engine.h"
#include "v_renderer.h"
#include "v_geometry.h"
#include "v_obj_3d.h"
#include "v_obj_3d_container.h"
#include "v_obj_3d_generators.h"
#include "v_lighting.h"
#include "v_scene.h"
#include "assets_2d.h"

#define TOROID_1_KEY '1'
#define TOROID_2_KEY '2'
#define TOROID_3_KEY '3'
#define CUBE_KEY '4'
#define OCTAHEDRON_KEY '5'
#define DODECAHEDRON_KEY '6'
#define ICOSAHEDRON_KEY '7'

#define ROTATION_TOGGLE_KEY 'q'
#define WIREFRAME_TOGGLE_KEY 'e'
#define MOVE_LEFT_KEY 'a'
#define MOVE_RIGHT_KEY 'd'
#define MOVE_UP_KEY 'w'
#define MOVE_DOWN_KEY 's'

#define SOLID_UNSHADED_KEY 'r'
#define SOLID_DIFF_KEY 't'
#define SOLID_SPEC_KEY 'y'
#define SOLID_DIFF_SPEC_KEY 'u'
#define INTERP_UNSHADED_KEY 'i'
#define INTERP_DIFF_KEY 'o'
#define INTERP_SPEC_KEY 'p'
#define INTERP_DIFF_SPEC_KEY '['

#define UNSHADED_TEXTURED_MUL_KEY 'f'
#define UNSHADED_TEXTURED_ADD_KEY 'g'
#define UNSHADED_TEXTURED_MUL_ADD_KEY 'h'
#define FLAT_REFLECTION_KEY 'j'
#define BUMP_REFLECTION_KEY 'k'

#define UNSHADED_TEXTURED_KEY 'z'
#define SOLID_DIFF_TEXTURED_KEY 'x'
#define SOLID_SPEC_TEXTURED_KEY 'c'
#define SOLID_DIFF_SPEC_TEXTURED_KEY 'v'
#define INTERP_DIFF_TEXTURED_KEY 'b'
#define INTERP_SPEC_TEXTURED_KEY 'n'
#define INTERP_DIFF_SPEC_TEXTURED_KEY 'm'

#define ASSETS_DIR "assets/"
#define WOOD_MAP "wood_512.jpg"
#define HEIGHT_MAP "heightmap_512.jpg"
#define MOUNTAINS_MAP "mountains_512.jpg"
#define SPECULAR_MAP "lightmap_red_m_512.jpg"

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 3D Object Rendering Types example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int i = 0, j = 0;
    FLOAT rotation_t = 0.0; // Current time
    FLOAT omega_x, omega_y, omega_z; //object angular velocities (constant)
    bool rotation_on = false, wireframe_on = false;
    FLOAT a_x, a_y, a_z; //object initial angles
    FLOAT v_x, v_y; //object linear velocities when corresponding key pressed
    FLOAT p_x, p_y, p_z; //object position
    FLOAT c_x, c_y; //object position change
    OBJ_3D_TYPE obj_3d_type = SOLID_UNSHADED;
    VEC_3 color_blue = {1.0, 0.5, 0.25};
    VEC_3 color_orange = {0.25, 0.5, 1.0};
    VEC_3 color_white = {1.0, 1.0, 1.0};
    VEC_3 wire_color = {1.0, 1.0, 1.0};
    VEC_3 color_ambient = {0.0, 0.0, 0.0};
    const int V1_COUNT = 180;
    const int V2_COUNT = 60;
    const int OBJECTS_COUNT = 7;
    OBJ_3D *objects[OBJECTS_COUNT];
    MAP *wood_map = NULL, *height_map = NULL, *mountains_map = NULL, *specular_map = NULL;
    BUMP_MAP *bump_map = NULL;
    SCENE_3D *scene = NULL;
    OBJ_3D_CONTAINER *container = NULL;

    printf("3D Object Rendering Types example\n");
    printf("Object type: %c, %c, %c, %c, %c, %c, %c\n", TOROID_1_KEY, TOROID_2_KEY, TOROID_3_KEY, CUBE_KEY, OCTAHEDRON_KEY, DODECAHEDRON_KEY, ICOSAHEDRON_KEY);
    printf("Rotation on/off: %c, Wireframe: %c\n", ROTATION_TOGGLE_KEY, WIREFRAME_TOGGLE_KEY);
    printf("Solid    unshaded: %c, diffuse: %c, specular: %c, diffuse & specular: %c\n",
        SOLID_UNSHADED_KEY,
        SOLID_DIFF_KEY,
        SOLID_SPEC_KEY,
        SOLID_DIFF_SPEC_KEY);

    printf("Interpolated    unshaded: %c, diffuse: %c, specular: %c, diffuse & specular: %c\n",
        INTERP_UNSHADED_KEY,
        INTERP_DIFF_KEY,
        INTERP_SPEC_KEY,
        INTERP_DIFF_SPEC_KEY);

    printf("Solid textured    unshaded: %c, diffuse: %c, specular: %c, diffuse & specular: %c\n",
        UNSHADED_TEXTURED_KEY,
        SOLID_DIFF_TEXTURED_KEY,
        SOLID_SPEC_TEXTURED_KEY,
        SOLID_DIFF_SPEC_TEXTURED_KEY);

    printf("Solid textured + reflection    mul: %c, add: %c, mul & add: %c, flat reflection: %c, bump reflection: %c\n",
        UNSHADED_TEXTURED_MUL_KEY,
        UNSHADED_TEXTURED_ADD_KEY,
        UNSHADED_TEXTURED_MUL_ADD_KEY,
        FLAT_REFLECTION_KEY,
        BUMP_REFLECTION_KEY);

    printf("Interpolated textured    diffuse: %c, specular: %c, diffuse & specular: %c\n",
        INTERP_DIFF_TEXTURED_KEY,
        INTERP_SPEC_TEXTURED_KEY,
        INTERP_DIFF_SPEC_TEXTURED_KEY);

    printf("Move left: %c, right: %c, up: %c, down: %c\n", MOVE_LEFT_KEY, MOVE_RIGHT_KEY, MOVE_UP_KEY, MOVE_DOWN_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    wood_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR WOOD_MAP), 50);
    height_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR HEIGHT_MAP), 50);
    mountains_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR MOUNTAINS_MAP), 0);
    specular_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR SPECULAR_MAP), 0);
    bump_map = convert_map_to_bump_map(height_map, 0.1);
    /* Building scene data structures */
    scene = scene_3d(display_buffer(), 1, 0); // Allocate the scene for 1 renderable object
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},
        .pos = {0.0, 0.0, -4.0, 0.0},
        .roll = 0.0, .fov = 90.0,
        .near_z = 0.5, .far_z = 7.5 }
    );
    //Configure scene-wide lighting
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = COMPOUND_3(color_ambient),
        .directional = COMPOUND_3(color_white),
        .direction = COMPOUND_4(*norm_v(&(VEC_4){0.0, 0.0, 1.0, 1.0}))}
    );
    container = obj_3d_container(NULL, 0);

    //Build all objects used in example
    objects[0] = obj_3d_toroid(QUAD, 1.5, 0.5, V1_COUNT, V2_COUNT, 5, 1);
    objects[1] = obj_3d_toroid(RIGHT_TRIANGLE, 1.5, 0.5, V1_COUNT, V2_COUNT, 5, 1);
    objects[2] = obj_3d_toroid(EQUILATERAL_TRIANGLE, 1.5, 0.5, V1_COUNT, V2_COUNT, 5, 1);
    objects[3] = obj_3d_regular_polyhedron(CUBE, 1.0);
    objects[4] = obj_3d_regular_polyhedron(OCTAHEDRON, 1.0);
    objects[5] = obj_3d_regular_polyhedron(DODECAHEDRON, 1.0);
    objects[6] = obj_3d_regular_polyhedron(ICOSAHEDRON, 1.0);

    /** Initial demo settings */
    container->obj = objects[0]; /** initial object: toroid */
    scene_3d_add_root_container(scene, container);

    //Displayed object position/orientation/movement parameters
    a_x = 20.0; a_y = 0.0; a_z = 0.0; // orientation
    omega_x=0.0; omega_y=10.0; omega_z=20.0; // angular velocities
    p_x = 0.0; p_y = 0.0; p_z = 0.0; // position
    c_x = 0.0; c_y = 0.0; // position change (when 'wasd' keys are used)
    v_x = 0.75; v_y = 0.5; // linear movement velocities (when moved with 'wasd' keys)

    rotation_on = false;
    wireframe_on = false;
    obj_3d_type = SOLID_DIFF_SPEC;
    rotation_t = 0.0;
    //initialize all objects properties
    for (i = 0; i < OBJECTS_COUNT; i++) {
        obj_3d_set_properties((OBJ_3D*)objects[i], &(OBJ_3D){
            .type = obj_3d_type,
            .surface_color = COMPOUND_3(color_blue),
            .wireframe_color = COMPOUND_3(wire_color),
            .base_map = wood_map,
            .bump_map = bump_map,
            .mul_map = mountains_map,
            .add_map = specular_map,
            .reflection_map = specular_map,
            .specular_power = 5.0,
            .wireframe_on = wireframe_on });
    }

    /** Add some checkerboard coloring to object faces/vertices */
    for (i = 0; i < V1_COUNT; i++) {
        for (j = 0; j < V2_COUNT; j++) {
            /** Checkerboard for quad-based objects */
            copy_v3(&objects[0]->faces[i*V2_COUNT+j].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
            copy_v3(&objects[0]->vertices[i*V2_COUNT+j].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);

            /** Stripes for triangle-based objects */
            copy_v3(&objects[1]->faces[(i*V2_COUNT+j)*2].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
            copy_v3(&objects[1]->faces[(i*V2_COUNT+j)*2+1].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
            copy_v3(&objects[1]->vertices[i*V2_COUNT+j].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);

            copy_v3(&objects[2]->faces[(i*V2_COUNT+j)*2].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
            copy_v3(&objects[2]->faces[(i*V2_COUNT+j)*2+1].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
            copy_v3(&objects[2]->vertices[i*V2_COUNT+j].color_surf,
            (j/10^i/15)&1 ? &color_blue : &color_orange);
        }
    }

    while (!quit_flag) {
        render_buffer_zero(display_buffer());

        rotation_t += (rotation_on ? 1. : 0.)*display_last_frame_interval();
        //rotate and perspective transform the cube
        obj_3d_container_set_transform(
            container,
            a_x + omega_x*rotation_t,
            a_y + omega_y*rotation_t,
            a_z + omega_z*rotation_t,
            p_x + c_x,
            p_y + c_y,
            p_z);

        // Execute a lighting calculation and rendering for the whole scene
        scene_3d_transform_and_light(scene);
        scene_3d_render(scene);

        display_show(0);
        periodic_fps_printf(1.0);

        c_x = 0.0;    c_y = 0.0;
        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_PRESSED) {
                switch((int)event->code) {
                    // Switch the rendering type depending on the user selection
                    case TOROID_1_KEY:
                        container->obj = objects[0];
                        break;
                    case TOROID_2_KEY:
                        container->obj = objects[1];
                        break;
                    case TOROID_3_KEY:
                        container->obj = objects[2];
                        break;
                    case CUBE_KEY:
                        container->obj = objects[3];
                        break;
                    case OCTAHEDRON_KEY:
                        container->obj = objects[4];
                        break;
                    case DODECAHEDRON_KEY:
                        container->obj = objects[5];
                        break;
                    case ICOSAHEDRON_KEY:
                        container->obj = objects[6];
                        break;
                    case ROTATION_TOGGLE_KEY:
                        rotation_on = !rotation_on;
                        break;
                    case WIREFRAME_TOGGLE_KEY:
                        wireframe_on = !wireframe_on;
                        break;
                    case SOLID_UNSHADED_KEY:
                        obj_3d_type = SOLID_UNSHADED;
                        break;
                    case SOLID_DIFF_KEY:
                        obj_3d_type = SOLID_DIFF;
                        break;
                    case SOLID_SPEC_KEY:
                        obj_3d_type = SOLID_SPEC;
                        break;
                    case SOLID_DIFF_SPEC_KEY:
                        obj_3d_type = SOLID_DIFF_SPEC;
                        break;
                    case INTERP_UNSHADED_KEY:
                        obj_3d_type = INTERP_UNSHADED;
                        break;
                    case INTERP_DIFF_KEY:
                        obj_3d_type = INTERP_DIFF;
                        break;
                    case INTERP_SPEC_KEY:
                        obj_3d_type = INTERP_SPEC;
                        break;
                    case INTERP_DIFF_SPEC_KEY:
                        obj_3d_type = INTERP_DIFF_SPEC;
                        break;
                    case FLAT_REFLECTION_KEY:
                        obj_3d_type = REFLECTION;
                        break;
                    case BUMP_REFLECTION_KEY:
                        obj_3d_type = TX_MAP_BUMP_REFLECTION;
                        break;
                    case UNSHADED_TEXTURED_KEY:
                        obj_3d_type = TX_MAP_BASE;
                        break;
                    case UNSHADED_TEXTURED_MUL_KEY:
                        obj_3d_type = TX_MAP_BASE_MUL;
                        break;
                    case UNSHADED_TEXTURED_ADD_KEY:
                        obj_3d_type = TX_MAP_BASE_ADD;
                        break;
                    case UNSHADED_TEXTURED_MUL_ADD_KEY:
                        obj_3d_type = TX_MAP_BASE_MUL_ADD;
                        break;
                    case SOLID_DIFF_TEXTURED_KEY:
                        obj_3d_type = SOLID_DIFF_TEXTURED;
                        break;
                    case SOLID_SPEC_TEXTURED_KEY:
                        obj_3d_type = SOLID_SPEC_TEXTURED;
                        break;
                    case SOLID_DIFF_SPEC_TEXTURED_KEY:
                        obj_3d_type = SOLID_DIFF_SPEC_TEXTURED;
                        break;
                    case INTERP_DIFF_TEXTURED_KEY:
                        obj_3d_type = INTERP_DIFF_TEXTURED;
                        break;
                    case INTERP_SPEC_TEXTURED_KEY:
                        obj_3d_type = INTERP_SPEC_TEXTURED;
                        break;
                    case INTERP_DIFF_SPEC_TEXTURED_KEY:
                        obj_3d_type = INTERP_DIFF_SPEC_TEXTURED;
                        break;

                    default:
                        break;
                }
                for (i = 0; i<OBJECTS_COUNT; i++) {
                    objects[i]->type = obj_3d_type;
                    objects[i]->wireframe_on = wireframe_on;
                }
            }
            else if (event->type == KEY_HOLD) {
                switch((int)event->code) {
                    case MOVE_RIGHT_KEY:
                        c_x += event->hold_time*-v_x; break;
                    case MOVE_LEFT_KEY:
                        c_x += event->hold_time*v_x; break;
                    case MOVE_DOWN_KEY:
                        c_y += event->hold_time*-v_y; break;
                    case MOVE_UP_KEY:
                        c_y += event->hold_time*v_y; break;
                    default:
                        break;
                }
            }
            else if (event->type == KEY_RELEASED) {
                switch((int)event->code) {
                    case MOVE_RIGHT_KEY:
                        c_x = 0;    p_x += event->hold_time*-v_x; break;
                    case MOVE_LEFT_KEY:
                        c_x = 0;    p_x += event->hold_time*v_x; break;
                    case MOVE_DOWN_KEY:
                        c_y = 0;    p_y += event->hold_time*-v_y; break;
                    case MOVE_UP_KEY:
                        c_y = 0;    p_y += event->hold_time*v_y; break;
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
    for (i = 0; i < OBJECTS_COUNT; i++) {
        obj_3d_free(objects[i]);
    }
    map_free(wood_map);    map_free(mountains_map);    map_free(specular_map);
    bump_map_free(bump_map);
    engine_cleanup();
    return 0;
}