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

#define ASSETS_DIR "assets/"
#define ANNOTATIONS "tranceverse.ant"
#define SOUNDTRACK "tranceverse.mp3"

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC Music annotations example";
    EVENT *event = NULL;
    bool quit_flag = false;
    FLOAT omega_x = 24.0, omega_y = 36.0, omega_z = 48.0; // object angular velocities (constant)
    //object colors
    COLOR obj_colors[] = {
        {.a = 1., .r = 0.349, .g = 0.227, .b = 0.772},
        {.a = 1., .r = 0.227, .g = 0.773, .b = 0.623},
        {.a = 1., .r = 0.651, .g = 0.773, .b = 0.227},
        {.a = 1., .r = 0.773, .g = 0.227, .b = 0.376}};
    //background colors
    COLOR back_colors[] = {
        {.a = 1., .r = 0.2, .g = 0.1, .b = 0.15},
        {.a = 1., .r = 0.1, .g = 0.15, .b = 0.2},
        {.a = 1., .r = 0.15, .g = 0.2, .b = 0.1}};
    //background flash colors
    COLOR flash_colors[] = {
        {.a = 1., .r = 0.4, .g = 0.2, .b = 0.3},
        {.a = 1., .r = 0.2, .g = 0.3, .b = 0.4},
        {.a = 1., .r = 0.3, .g = 0.4, .b = 0.2}};
    //Lighting colors
    COLOR color_directional = {.a = 1., .r = 1.0, .g = 1.0, .b = 1.0};
    COLOR color_ambient = {.a = 1., .r = 0.0, .g = 0.0, .b = 0.0};

    SCENE_3D *scene = NULL;
    OBJ_3D_CONTAINER *container = NULL;

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    annotations_read(runtime_file_path(argv[0], ASSETS_DIR ANNOTATIONS));
    engine_load_music(runtime_file_path(argv[0], ASSETS_DIR SOUNDTRACK));

    scene = scene_3d(display_buffer(), 1, 0);
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){ .enabled = false });
    scene->rotate_all_objects_vertex_normals = true;
    scene_3d_camera_set_settings(scene, &(CAMERA_SETTINGS){
        .look_at = {0.0, 0.0, 0.0, 0.0},    .pos = {0.0, 0.0, -6.0, 0.0},
        .roll = 0.0,    .fov = 90.0,    .near_z = 0.5,    .far_z = 15.0});
    scene_3d_lighting_set_settings(scene, &(GLOBAL_LIGHT_SETTINGS){
        .enabled = true,
        .ambient = color_ambient,
        .directional = color_directional,
        .direction = COMPOUND_4(*norm_v(&(VEC_4){0.0, 0.0, 1.0, 1.0}))}
    );

    const FLOAT TOR_PHI = 5.0; //outer diameter of toroid
    container = obj_3d_container(
        obj_3d_toroid(QUAD, 0.8*(TOR_PHI/2), 0.2*(TOR_PHI/2), 50, 12, 5, 1), 0);
    obj_3d_set_properties(container->obj, &(OBJ_3D){
        .wireframe_on = false,
        .specular_power = 5.0,
        .type = SOLID_DIFF_SPEC });
    scene_3d_add_root_container(scene, container);

    //Start the actual demo
    engine_play_music();
    //If you need to start the demo from the specific time point, this is the way to do it.
    //engine_set_music_position(100);
    while (!quit_flag)
    {
        FLOAT t = engine_run_stats().music_position;
        FLOAT rotation_t = t;
        FLOAT beat_s = 1.0;
        annotations_track(t);
        //Visualise beat only when not on part 0 or 7
        if (annotations_last_v()[0] > 0 && annotations_last_v()[0] != 7) {
            // Depending on the type of the annotated beat, skip rotation angle or scale object
            if (annotations_last_v()[3] == 2)
                rotation_t += 0.3*(1.0-transit_line(t - annotations_last_t()[3], 0.5));
            else
                beat_s = 0.3*(1.0-transit_cube_high(t - annotations_last_t()[3], 0.5)) + 1.0;
        }

        obj_3d_container_set_transform(
            container,
            omega_x * rotation_t, omega_y * rotation_t, omega_z * rotation_t,
            0.0, 0.0, 0.0, beat_s, beat_s, beat_s);
        scene_3d_transform_and_light(scene);

        COLOR color;
        //Decide type of the background color change.
        //depending on part of the music (annotation value of channel 0)
        //The color itself depends on the annotation value of channel 2
        INT bcci = annotations_last_v()[2]%4; //Current background color index
        if (bcci < 0 || bcci == 3) bcci = 0;
        INT bcpi = annotations_prev_v()[2]%4; //Previous background color index
        if (bcpi < 0 || bcpi == 3) bcpi = 0;
        if ((annotations_last_v()[0] & 1) && annotations_last_v()[0] != 7) {
            //"Flash" background color change
            color = *COLOR_blend(&back_colors[bcci], &flash_colors[bcci],
                            transit_cube_high(t - annotations_last_t()[2], 0.3));
        } else {
            //"Blend" to the new background color from the previous one.
            color = *COLOR_blend(&back_colors[bcci], &back_colors[bcpi],
                            transit_cube_high(t - annotations_last_t()[2], 0.3));
        }
        RENDER_BUFFER_fill(display_buffer(), &color);

        //Change object color depending on annotation value of channel 1
        INT occi = annotations_last_v()[1]%4; //Current object color index
        INT ocpi = annotations_prev_v()[1]%4; //Previous object color index
        color = *COLOR_blend(&obj_colors[occi], &obj_colors[ocpi],
                        transit_cube_high(t - annotations_last_t()[1], 0.3));
        obj_3d_set_surface_color(container->obj, &color);

        scene_3d_render(scene);
        display_show(0);

        event = engine_poll_events();
        while (event->type != NO_EVENT)
        { // iterate events until none is left to process
            if (event->type == QUIT_REQUEST)
            {
                quit_flag = true;
            }
            event++; // skip to next event
        }
        //Quit after music ended playing
        if (!engine_playing()) {
            quit_flag = true;
        }
        #ifdef RUN_ONE_FRAME
            quit_flag = true;
        #endif
    }

    scene_3d_free(scene);

    engine_cleanup();
    return 0;
}