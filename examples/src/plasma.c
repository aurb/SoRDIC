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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine.h"
#include "render_buffer.h"
#include "maps.h"

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC Plasma example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int i=0, fcnt=0;
    INT xo1=0, yo1=0;
    INT xo2=0, yo2=0;
    RENDER_BUFFER *srbuf = NULL;

    /** Initialize whole engine (including display system) */
    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    /** Allocate buffer for rendering second pattern */
    srbuf = render_buffer(display_buffer()->width, display_buffer()->height, Z_BUFFER_OFF);

    /** Prepare data structures for generation of both patterns */
    PLASMA_DATA sp1, sp2;
    plasma_precalc(&sp1, 284, 300, 672, 200, 199);
    plasma_precalc(&sp2, 411, 220, 541, 350, 409);

    /** Calculate gradients for both sine pattern.
      * The gradient table is where actual colors for the pattern come from.
      */
    const int spike_size = sp1.gradient_size/4;
    RGB_PIXEL *spike = calloc(spike_size, sizeof(RGB_PIXEL));
    for (i = 0; i<spike_size; i++)
        spike[i] = 255*(-0.5*cos(i*6.2832/spike_size)+0.5);
    for (i = 0; i<spike_size; i++) {
        sp1.gradient[i+(int)((sp1.gradient_size-spike_size)/2)] = (spike[i]<<16) | (spike[i]/2<<8) | (spike[i]/7);
        sp2.gradient[i+(int)((sp2.gradient_size-spike_size)/2)] = (spike[i]/6<<16) | (spike[i]/2<<8) | (spike[i]);
    }
    free(spike);

    /** Loop rendering patterns animation. */
    while (!quit_flag) {
        /** Calculate reference coordinates for both patterns. */
        yo1 = 150*sin(fcnt*6.2832/3254)+150;
        xo1 = 233*cos(fcnt*6.2832/2871)+233;
        yo2 = 298*cos(fcnt*6.2832/2354)+298;
        xo2 = 104*sin(fcnt*6.2832/5271)+104;
        /** Render patterns (each to separate buffer) */
        plasma_rasterize(&sp2, display_buffer()->rgb, xo1, yo1);
        plasma_rasterize(&sp1, srbuf->rgb, xo2, yo2);
        /** Add both buffers. */
        rgb_map_sat_add(display_buffer()->rgb, srbuf->rgb);
        /** Display the result */
        display_show(0);
        fcnt++;
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

    render_buffer_free(srbuf);
    plasma_free(&sp1);
    plasma_free(&sp2);
    engine_cleanup();
    return 0;
}