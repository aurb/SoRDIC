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
#include "v_renderer.h"
#include "v_geometry.h"
#include "v_obj_3d.h"
#include "v_obj_3d_container.h"
#include "v_obj_3d_generators.h"
#include "v_scene.h"
#include "assets_2d.h"

#define MASK_KEY 'q'
#define DITHER_OPACITY_KEY 'w'
#define FULL_OPACITY_KEY 'e'

#define ASSETS_DIR "assets/"
#define WOOD_MAP "wood_1024.jpg"
#define RED_LIGHT_MAP "lightmap_red_512.jpg"

typedef enum {MASK, DITHER_OPACITY, FULL_OPACITY} DEMO_MODE;

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC Render Buffers Layering example";
    int i = 0;
    EVENT *event = NULL;
    bool quit_flag = false;
    FLOAT rotation_t = 0.0;          // Current rotation time
    FLOAT omega_x, omega_y, omega_z; // object angular velocities (constant)
    VEC_3 color_blue = {1.0, 0.5, 0.25};
    VEC_3 wire_color = {1.0, 1.0, 1.0};
    MAP *wood_map = NULL, *red_light_map = NULL;
    SCENE_3D *scene = NULL;
    const int LAYER_COUNT = 3;
    RENDER_BUFFER *render_layers[LAYER_COUNT];
    RENDER_BUFFER *mask_layer = NULL, *opacity_layer = NULL;
    OBJ_3D_CONTAINER *container = NULL;
    DEMO_MODE mode = MASK;

    printf("Render Buffers Layering example\n");
    printf("Mask: %c, Dither opacity: %c, Full opacity: %c\n", MASK_KEY, DITHER_OPACITY_KEY, FULL_OPACITY_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    for (i = 0; i < LAYER_COUNT; i++)
        render_layers[i] = render_buffer(
            display_buffer()->width, display_buffer()->height, Z_BUFFER_ON);
    mask_layer = render_buffer(
        display_buffer()->width, display_buffer()->height, Z_BUFFER_OFF);
    opacity_layer = render_buffer(
        display_buffer()->width, display_buffer()->height, Z_BUFFER_OFF);

    INT x = 0, y = 0;
    int offs = 0;
    //generate vertical gradient for opacity rendering
    for (y = 0, offs = 0; y < opacity_layer->height; y++) {
        for (x = 0; x < 0.4*opacity_layer->width; x++, offs++) {
            opacity_layer->p[offs] = 0;
        }
        for (; x < 0.6*opacity_layer->width; x++, offs++) {
            opacity_layer->p[offs] = 256*(x-0.4*opacity_layer->width)/(0.2*opacity_layer->width);
        }
        for (; x < opacity_layer->width; x++, offs++) {
            opacity_layer->p[offs] = 256;
        }
    }
    //generate diagonal bars for mask rendering
    for (y = 0, offs = 0; y < mask_layer->height; y++) {
        for (x = 0; x < mask_layer->width; x++, offs++) {
            mask_layer->p[offs] = 6*(x+y)/(mask_layer->width+mask_layer->height)%3;
        }
    }

    wood_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR WOOD_MAP), 100);
    red_light_map = read_map_from_image(runtime_file_path(argv[0], ASSETS_DIR RED_LIGHT_MAP), 0);
    /* Building scene data structures */
    scene = scene_3d(display_buffer(), 1, 0); // Allocate the scene for 1 renderable object, no lights, undefined display buffer
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){ .enabled = false });
    scene->rotate_all_objects_vertex_normals = true;
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, -4.0, 0.0},
        .roll = 0.0,    .fov = 90.0,    .near_z = 0.5,    .far_z = 15.0});

    // Create object for the example
    const FLOAT TOR_PHI = 5.0; //outer diameter of toroid
    container = obj_3d_container(
        obj_3d_toroid(QUAD, 0.8*(TOR_PHI/2), 0.2*(TOR_PHI/2), 150, 30, 5, 1), 0);
    scene_3d_add_root_container(scene, container);

    obj_3d_set_properties(container->obj, &(OBJ_3D){
        .surface_color = COMPOUND_3(color_blue),
        .wireframe_color = COMPOUND_3(wire_color),
        .base_map = wood_map,
        .reflection_map = red_light_map,
        .type = TX_MAP_BASE });

    /** Initial demo settings */
    omega_x = 8.0;
    omega_y = 16.0;
    omega_z = 24.0;
    mode = MASK;

    while (!quit_flag)
    {
        rotation_t = display_run_stats().time;

        // rotate and perspective transform the cube
        obj_3d_container_set_transform(
            container,
            omega_x * rotation_t,
            omega_y * rotation_t,
            omega_z * rotation_t,
            0.0, 0.0, 0.0);
        scene_3d_transform_and_light(scene);

        render_buffer_fill(render_layers[0], &(VEC_3){0.15, 0.1, 0.2});
        render_buffer_fill(render_layers[1], &(VEC_3){0.1, 0.2, 0.15});

        container->obj->type = REFLECTION;
        container->obj->wireframe_on = false;
        scene->render_buf = render_layers[0];
        scene_3d_render(scene);

        container->obj->type = TX_MAP_BASE;
        container->obj->wireframe_on = false;
        scene->render_buf = render_layers[1];
        scene_3d_render(scene);

        if (mode == MASK) {
            render_buffer_fill(render_layers[2], &(VEC_3){0.2, 0.15, 0.1});
            container->obj->type = SOLID_UNSHADED;
            container->obj->wireframe_on = true;
            scene->render_buf = render_layers[2];
            scene_3d_render(scene);
            render_buffer_fixed_mask(display_buffer(), (RENDER_BUFFER**)render_layers, mask_layer);
        }
        else if (mode == DITHER_OPACITY) {
            render_buffer_dither_opacity(display_buffer(), render_layers[0], render_layers[1], opacity_layer);
        }
        else if (mode == FULL_OPACITY) {
            render_buffer_full_opacity(display_buffer(), render_layers[0], render_layers[1], opacity_layer);
        }

        display_show(0);
        periodic_fps_printf(1.0);

        event = engine_poll_events();
        while (event->type != NO_EVENT)
        { // iterate events until none is left to process
            if (event->type == QUIT_REQUEST)
            {
                quit_flag = true;
            }
            else if (event->type == KEY_PRESSED) {
                switch((int)event->code) {
                    // Switch the rendering type depending on the user selection
                    case MASK_KEY:
                        mode = MASK;
                        break;
                    case DITHER_OPACITY_KEY:
                        mode = DITHER_OPACITY;
                        break;
                    case FULL_OPACITY_KEY:
                        mode = FULL_OPACITY;
                        break;
                    default:
                        break;
                }
            }
            event++; // skip to next event
        }
        #ifdef RUN_ONE_FRAME
            quit_flag = true;
        #endif
    }

    printf("\n");
    for (i = 0; i < LAYER_COUNT; i++)
        render_buffer_free(render_layers[i]);
    render_buffer_free(mask_layer);
    render_buffer_free(opacity_layer);
    scene_3d_free(scene);
    map_free(wood_map);    map_free(red_light_map);

    engine_cleanup();
    return 0;
}