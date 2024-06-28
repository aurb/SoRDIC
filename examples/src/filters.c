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

#include "engine.h"

#define NO_FILTER_KEY ' '
#define COPY_FILTER_1_KEY '1'
#define COPY_FILTER_2_KEY '2'
#define COPY_FILTER_3_KEY '3'
#define COPY_FILTER_4_KEY '4'
#define COPY_FILTER_5_KEY '5'
#define COPY_FILTER_6_KEY '6'
#define COPY_FILTER_7_KEY '7'
#define COPY_FILTER_8_KEY '8'
#define BLEND_FILTER_1_KEY 'q'
#define BLEND_FILTER_2_KEY 'w'
#define BLEND_FILTER_3_KEY 'e'
#define BLEND_FILTER_4_KEY 'r'
#define BLEND_FILTER_5_KEY 't'
#define BLEND_FILTER_6_KEY 'y'
#define BLEND_FILTER_7_KEY 'u'
#define BLEND_FILTER_8_KEY 'i'

#define ASSETS_DIR "assets/"
#define WOOD_MAP "wood_1024.jpg"

typedef enum {NO_FILTER, COPY_FILTER_1, COPY_FILTER_2, COPY_FILTER_3, COPY_FILTER_4, COPY_FILTER_5, COPY_FILTER_6, COPY_FILTER_7,
              BLEND_FILTER_1, BLEND_FILTER_2, BLEND_FILTER_3, BLEND_FILTER_4, BLEND_FILTER_5, BLEND_FILTER_6, BLEND_FILTER_7} DEMO_MODE;

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC Map Filters example";
    EVENT *event = NULL;
    bool quit_flag = false;
    FLOAT rotation_t = 0.0;          // Current rotation time
    FLOAT omega_x, omega_y, omega_z; // object angular velocities (constant)
    ARGB_MAP *wood_map = NULL, *edge_p_map = NULL, *blur_p_map = NULL, *background_map = NULL;
    SCENE_3D *scene = NULL;
    RENDER_BUFFER *tx_render_buffer = NULL;
    GRADIENT edge_p_gradient, edge_copy_gradient, edge_blend_gradient, blur_p_gradient, back_gradient;

    OBJ_3D_CONTAINER *container = NULL;
    DEMO_MODE mode = NO_FILTER;

    printf("Map Filters example\n");
    printf("No filter key: %c\n", NO_FILTER_KEY);
    printf("Copy Filter keys: %c, %c, %c, %c, %c, %c, %c\n",
           COPY_FILTER_1_KEY, COPY_FILTER_2_KEY, COPY_FILTER_3_KEY, COPY_FILTER_4_KEY,
           COPY_FILTER_5_KEY, COPY_FILTER_6_KEY, COPY_FILTER_7_KEY);
    printf("Blend Filter keys: %c, %c, %c, %c, %c, %c, %c\n",
           BLEND_FILTER_1_KEY, BLEND_FILTER_2_KEY, BLEND_FILTER_3_KEY, BLEND_FILTER_4_KEY,
           BLEND_FILTER_5_KEY, BLEND_FILTER_6_KEY, BLEND_FILTER_7_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    tx_render_buffer = RENDER_BUFFER_alloc(
            display_buffer()->width, display_buffer()->height, Z_BUFFER_ON);

    edge_p_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    blur_p_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    background_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    wood_map = ARGB_MAP_read_image(runtime_file_path(argv[0], ASSETS_DIR WOOD_MAP), 100);

    edge_p_gradient.count = 0;
    GRADIENT_add_point(&edge_p_gradient, 0.0,  &(COLOR){.a = 1.0});
    GRADIENT_add_point(&edge_p_gradient, 0.2, &(COLOR){.a = 1.0});
    GRADIENT_add_point(&edge_p_gradient, 0.8, &(COLOR){.a = 0.0});
    GRADIENT_add_point(&edge_p_gradient, 1.0, &(COLOR){.a = 0.0});
    ARGB_MAP_vertical_pattern(edge_p_map, &edge_p_gradient);

    blur_p_gradient.count = 0;
    GRADIENT_add_point(&blur_p_gradient, 0.0,  &(COLOR){.a = 0.8});
    GRADIENT_add_point(&blur_p_gradient, 0.333, &(COLOR){.a = 0.8});
    GRADIENT_add_point(&blur_p_gradient, 0.666, &(COLOR){.a = 0.0});
    GRADIENT_add_point(&blur_p_gradient, 1.0, &(COLOR){.a = 0.0});
    ARGB_MAP_horizontal_pattern(blur_p_map, &blur_p_gradient);

    edge_copy_gradient.count = 0;
    GRADIENT_add_point(&edge_copy_gradient, 0.0,  &(COLOR){.a = 1.0, .r = 0.2, .g = 0.1, .b = 0.3});
    GRADIENT_add_point(&edge_copy_gradient, 0.8, &(COLOR){.a = 1.0, .r = 1.0, .g = 1.0, .b = 1.0});
    GRADIENT_add_point(&edge_copy_gradient, 1.0, &(COLOR){.a = 1.0, .r = 1.0, .g = 1.0, .b = 1.0});

    edge_blend_gradient.count = 0;
    GRADIENT_add_point(&edge_blend_gradient, 0.0,  &(COLOR){.a = 0.0, .r = 1.0, .g = 1.0, .b = 1.0});
    GRADIENT_add_point(&edge_blend_gradient, 0.8, &(COLOR){.a = 1.0,  .r = 1.0, .g = 1.0, .b = 1.0});
    GRADIENT_add_point(&edge_blend_gradient, 1.0, &(COLOR){.a = 1.0,  .r = 1.0, .g = 1.0, .b = 1.0});

    back_gradient.count = 0;
    GRADIENT_add_point(&back_gradient, 0.0,  &(COLOR){.a = 1.0, .r = 0.25, .g = 0.25, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 1.0,  &(COLOR){.a = 0.0, .r = 0.5, .g = 0.3, .b = 0.5});
    ARGB_MAP_xor_pattern(background_map, &back_gradient);

    /* Building scene data structures */
    scene = scene_3d(display_buffer(), 1, 0); // Allocate the scene for 1 renderable object, no lights, undefined display buffer
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){ .enabled = false });
    // Add still camera
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, -4.0, 0.0},
        .roll = 0.0,    .fov = 90.0,    .near_z = 0.5,    .far_z = 15.0});

    // Create object for the example
    const FLOAT TOR_PHI = 4.5; //outer diameter of toroid
    container = obj_3d_container(
        obj_3d_toroid(QUAD, 0.8*(TOR_PHI/2), 0.2*(TOR_PHI/2), 150, 30, 5, 1), 0);
    scene_3d_add_root_container(scene, container);

    obj_3d_set_properties(container->obj, &(OBJ_3D){
        .wireframe_on = false,
        .base_map = wood_map,
        .type = TX_MAP_BASE });

    /** Initial demo settings */
    omega_x = 8.0;
    omega_y = 16.0;
    omega_z = 24.0;
    mode = NO_FILTER;

    while (!quit_flag)
    {
        rotation_t = engine_run_stats().time;
        INT cx = display_buffer()->width*(0.3*cos(rotation_t*TWOPI/5.0));
        INT cy = display_buffer()->height*(0.3*sin(rotation_t*TWOPI/5.0));

        // rotate and perspective transform the cube
        obj_3d_container_set_transform(
            container,
            omega_x * rotation_t,
            omega_y * rotation_t,
            omega_z * rotation_t,
            0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
        scene_3d_transform_and_light(scene);


        if (mode == NO_FILTER) {
            scene->render_buf = display_buffer();
            RENDER_BUFFER_zero(display_buffer());
            scene_3d_render(scene);
        }
        else if (mode == COPY_FILTER_1) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_green_gradient_global_copy(display_buffer()->map, cx, cy, tx_render_buffer->map, &edge_copy_gradient, MAX_EDGE_WIDTH);
        }
        else if (mode == BLEND_FILTER_1) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            RENDER_BUFFER_ARGB_MAP_copy(display_buffer(), background_map);
            ARGB_MAP_green_gradient_global_blend(display_buffer()->map, cx, cy, background_map, tx_render_buffer->map, &edge_blend_gradient, MAX_EDGE_WIDTH);
        }
        else if (mode == COPY_FILTER_2) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_green_gradient_per_pixel_copy(display_buffer()->map, cx, cy, tx_render_buffer->map, &edge_copy_gradient, edge_p_map);
        }
        else if (mode == BLEND_FILTER_2) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            RENDER_BUFFER_ARGB_MAP_copy(display_buffer(), background_map);
            ARGB_MAP_green_gradient_per_pixel_blend(display_buffer()->map, cx, cy, background_map, tx_render_buffer->map, &edge_blend_gradient, edge_p_map);
        }
        else if (mode == COPY_FILTER_3) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_ARGB_MAP_copy(tx_render_buffer, background_map);
            scene_3d_render(scene);
            ARGB_MAP_blur_1xn_global_copy(display_buffer()->map, tx_render_buffer->map, 100);
        }
        else if (mode == BLEND_FILTER_3) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_blur_1xn_global_blend(display_buffer()->map, background_map, tx_render_buffer->map, 100);
        }
        else if (mode == COPY_FILTER_4) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_ARGB_MAP_copy(tx_render_buffer, background_map);
            scene_3d_render(scene);
            ARGB_MAP_blur_nx1_global_copy(display_buffer()->map, tx_render_buffer->map, 100);
        }
        else if (mode == BLEND_FILTER_4) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_blur_nx1_global_blend(display_buffer()->map, background_map, tx_render_buffer->map, 100);
        }
        else if (mode == COPY_FILTER_5) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_ARGB_MAP_copy(tx_render_buffer, background_map);
            scene_3d_render(scene);
            ARGB_MAP_blur_nx1_per_pixel_copy(display_buffer()->map, tx_render_buffer->map, blur_p_map);
        }
        else if (mode == BLEND_FILTER_5) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_blur_nx1_per_pixel_blend(display_buffer()->map, background_map, tx_render_buffer->map, blur_p_map);
        }
        else if (mode == COPY_FILTER_6) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_ARGB_MAP_copy(tx_render_buffer, background_map);
            scene_3d_render(scene);
            ARGB_MAP_pixelize_copy(display_buffer()->map, tx_render_buffer->map, 9);
        }
        else if (mode == BLEND_FILTER_6) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_pixelize_blend(display_buffer()->map, background_map, tx_render_buffer->map, 9);
        }
        else if (mode == COPY_FILTER_7) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_ARGB_MAP_copy(tx_render_buffer, background_map);
            scene_3d_render(scene);
            ARGB_MAP_rand_pixelize_copy(display_buffer()->map, tx_render_buffer->map,
                                       5, 60, 0,  5, 10, 0);
        }
        else if (mode == BLEND_FILTER_7) {
            scene->render_buf = tx_render_buffer;
            RENDER_BUFFER_zero(tx_render_buffer);
            scene_3d_render(scene);
            ARGB_MAP_rand_pixelize_blend(display_buffer()->map, background_map, tx_render_buffer->map,
                                       5, 60, 0,  5, 10, 0);
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
                    case NO_FILTER_KEY:
                        mode = NO_FILTER;
                        break;
                    case COPY_FILTER_1_KEY:
                        mode = COPY_FILTER_1;
                        break;
                    case COPY_FILTER_2_KEY:
                        mode = COPY_FILTER_2;
                        break;
                    case COPY_FILTER_3_KEY:
                        mode = COPY_FILTER_3;
                        break;
                    case COPY_FILTER_4_KEY:
                        mode = COPY_FILTER_4;
                        break;
                    case COPY_FILTER_5_KEY:
                        mode = COPY_FILTER_5;
                        break;
                    case COPY_FILTER_6_KEY:
                        mode = COPY_FILTER_6;
                        break;
                    case COPY_FILTER_7_KEY:
                        mode = COPY_FILTER_7;
                        break;

                    case BLEND_FILTER_1_KEY:
                        mode = BLEND_FILTER_1;
                        break;
                    case BLEND_FILTER_2_KEY:
                        mode = BLEND_FILTER_2;
                        break;
                    case BLEND_FILTER_3_KEY:
                        mode = BLEND_FILTER_3;
                        break;
                    case BLEND_FILTER_4_KEY:
                        mode = BLEND_FILTER_4;
                        break;
                    case BLEND_FILTER_5_KEY:
                        mode = BLEND_FILTER_5;
                        break;
                    case BLEND_FILTER_6_KEY:
                        mode = BLEND_FILTER_6;
                        break;
                    case BLEND_FILTER_7_KEY:
                        mode = BLEND_FILTER_7;
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
    RENDER_BUFFER_free(tx_render_buffer);
    scene_3d_free(scene);
    ARGB_MAP_free(wood_map);
    ARGB_MAP_free(edge_p_map);
    ARGB_MAP_free(blur_p_map);
    ARGB_MAP_free(background_map);
    engine_cleanup();
    return 0;
}