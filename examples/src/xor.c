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
#include <math.h>

#include "engine.h"

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC XOR example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int x, y, xoffs=0, yoffs=0, offs, c;
    RENDER_BUFFER *sine;

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    sine = render_buffer(display_buffer()->width, display_buffer()->height, Z_BUFFER_OFF);

    RGB_PIXEL sc;
    for(y=0, offs=0; y<sine->height; y++) {
        sc = (RGB_PIXEL)(127.5*sin(y*2.5*6.2832/(double)sine->height)+127.5);
        sc = (sc) | (sc<<8) | (sc<<16);
        for(x=0; x<sine->width; x++, offs++) {
            (sine->p)[offs] = sc;
        }
    }

    while (!quit_flag) {
        for(y=0, offs=0; y<display_buffer()->height; y++)
            for(x=0; x<display_buffer()->width; x++, offs++)
            {
                c = (((x+xoffs)&0xFF) ^ ((y+yoffs)&0xFF)) ^ (((x/2)&0xFF) ^ ((y/2)&0xFF));
                (display_buffer()->p)[offs] = (c/8<<16) | (c/2<<8) | (c);
            }
        render_buffer_add_saturation(display_buffer(), sine);
        display_show(0);
        xoffs += 2;
        yoffs -= 1;
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

    render_buffer_free(sine);
    engine_cleanup();
    return 0;
}