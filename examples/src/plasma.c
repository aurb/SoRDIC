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

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC Plasma example";
    EVENT *event = NULL;
    bool quit_flag = false;
    GRADIENT plasma_gradient1, plasma_gradient2, back_gradient, alpha_gradient, xor_gradient;
    ARGB_MAP *background_map = NULL, *alpha_map = NULL, *plasma_map = NULL, *xor_map = NULL;
    COLOR black = {.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0};

    /** Initialize whole engine (including display system) */
    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    background_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    alpha_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    plasma_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);
    xor_map = ARGB_MAP_alloc(display_buffer()->width, display_buffer()->height, 0);

    // Gradient for rendering background plasma
    plasma_gradient1.count = 0;
    GRADIENT_add_point(&plasma_gradient1, 0.0,  &(COLOR){.a = 0.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient1, 0.4, &(COLOR){.a = 0.0, .b = 1.0, .g = 0.65, .r = 0.35});
    GRADIENT_add_point(&plasma_gradient1, 0.5, &(COLOR){.a = 1.0, .b = 1.0, .g = 0.5, .r = 0.5});
    GRADIENT_add_point(&plasma_gradient1, 0.6,  &(COLOR){.a = 0.0, .b = 1.0, .g = 0.35, .r = 0.65});
    GRADIENT_add_point(&plasma_gradient1, 1.0,  &(COLOR){.a = 0.0, .r = 0.0, .g = 0.0, .b = 0.0});

    // Background plasma alpha channel
    alpha_gradient.count = 0;
    alpha_gradient.background = (COLOR){.r = 0.0, .g = 0.0, .b = 0.0};
    GRADIENT_add_point(&alpha_gradient, 0.0,  &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&alpha_gradient, 0.6,  &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&alpha_gradient, 0.8,  &(COLOR){.a = 0.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&alpha_gradient, 1.0,  &(COLOR){.a = 0.0, .r = 0.0, .g = 0.0, .b = 0.0});
    ARGB_MAP_radial_pattern(alpha_map, &alpha_gradient, alpha_map->width/2, alpha_map->height/2);

    // Gradient for rendering foreground plasma
    plasma_gradient2.count = 0;
    GRADIENT_add_point(&plasma_gradient2, 0.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient2, 0.2, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient2, 0.3, &(COLOR){.a = 1.0, .r = 1.0/6.0, .g = 3.0/4.0, .b = 1.0});
    GRADIENT_add_point(&plasma_gradient2, 0.4, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient2, 0.5, &(COLOR){.a = 1.0, .r = 2.0/6.0, .g = 2.0/4.0, .b = 1.0});
    GRADIENT_add_point(&plasma_gradient2, 0.6, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient2, 0.7, &(COLOR){.a = 1.0, .r = 3.0/6.0, .g = 1.0/4.0, .b = 1.0});
    GRADIENT_add_point(&plasma_gradient2, 0.8, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});
    GRADIENT_add_point(&plasma_gradient2, 1.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.0, .b = 0.0});

    // Background map
    back_gradient.count = 0;
    back_gradient.background = (COLOR){.a = 1.0, .r = 0.0, .g = 0.5, .b = 0.6};
    GRADIENT_add_point(&back_gradient, 0.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 1.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 1.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 2.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 2.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 3.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 3.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 4.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 4.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 5.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 5.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 6.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 6.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 7.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.4, .b = 0.5});
    GRADIENT_add_point(&back_gradient, 7.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    GRADIENT_add_point(&back_gradient, 8.0/8.0, &(COLOR){.a = 1.0, .r = 0.0, .g = 0.25, .b = 0.3});
    ARGB_MAP_radial_pattern(background_map, &back_gradient, background_map->width/2, background_map->height/2);

    // XOR map
    xor_gradient.count = 0;
    GRADIENT_add_point(&xor_gradient, 0.0,  &(COLOR){.a = 1.0, .b = 1.0, .g = 0.5, .r = 0.5});
    GRADIENT_add_point(&xor_gradient, 1.0,  &(COLOR){.a = 0.0, .b = 1.0, .g = 0.6, .r = 1.0});
    ARGB_MAP_xor_pattern(xor_map, &xor_gradient);

    /** Loop rendering patterns animation. */
    while (!quit_flag) {
        FLOAT anim_t = engine_run_stats().time;

        /** Calculate reference coordinates for both plasmas. */
        FLOAT yo1 = 0.125*(sin(anim_t*TWOPI/4.2)+1.);
        FLOAT xo1 = 0.194*(cos(anim_t*TWOPI/4.8)+1.);
        FLOAT yo2 = 0.248*(cos(anim_t*TWOPI/3.15)+1.);
        FLOAT xo2 = 0.107*(sin(anim_t*TWOPI/3.66)+1.);

        // Layer 1 - Background
        ARGB_MAP_copy(display_buffer()->map, background_map);

        // Layer 2 - Semi-translucent XOR pattern blending in and out.
        FLOAT xor_t = FMOD(anim_t, (FLOAT)9.0);
        FLOAT f = 0.6*(xor_t < 3.0 ? xor_t/3.0 : (xor_t < 6.0 ? 1.0-(xor_t - 3.0)/3.0 : 0.0));
        ARGB_MAP_blend_dither_f_per_pixel(display_buffer()->map, display_buffer()->map, xor_map, f, xor_map);

        // Layer 3 - Background plasma
        ARGB_MAP_plasma_pattern(plasma_map, &plasma_gradient2, 4.0, 0.183, 0.225, xo1, 0.292, 0.341, yo1);
        ARGB_MAP_fade_mul_per_pixel(plasma_map, &black, plasma_map, alpha_map);
        ARGB_MAP_sat_add(display_buffer()->map, display_buffer()->map, plasma_map);

        // Layer 4 - Foreground plasma
        ARGB_MAP_plasma_pattern(plasma_map, &plasma_gradient1, 4.0, 0.25, 0.56, xo2, 0.167, 0.167, yo2);
        ARGB_MAP_blend_mul_per_pixel(display_buffer()->map, display_buffer()->map, plasma_map, plasma_map);

        // Show everything on the display.
        display_show(0);
        periodic_fps_printf(1.0);
        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            event++;
        }
        #ifdef RUN_ONE_FRAME
            quit_flag = true;
        #endif
    }
    printf("\n");

    ARGB_MAP_free(xor_map);
    ARGB_MAP_free(plasma_map);
    ARGB_MAP_free(alpha_map);
    ARGB_MAP_free(background_map);
    engine_cleanup();
    return 0;
}