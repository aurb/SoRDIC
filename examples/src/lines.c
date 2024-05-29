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

#include "engine.h"
#include "v_rasterizer.h"

#define MOVE_LEFT_KEY 'a'
#define MOVE_RIGHT_KEY 'd'
#define MOVE_UP_KEY 'w'
#define MOVE_DOWN_KEY 's'

int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 2D Random Lines example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int LPF = 1000; //lines per frame
    INT x0, y0, x1, y1;
    FLOAT vx = 200.0, vy = 200.0; //drawing area linear velocities when corresponding key hold
    INT px = 0, py = 0; //drawing area position
    INT cx = 0, cy = 0; //drawing area position change

    if (argc != 2) {
        printf("Usage: %s lines_per_frame\n", argv[0]);
        printf("Starting with default lines_per_frame=%d\n", LPF);
    }
    else {
        LPF = atoi(argv[1]);
    }

    printf("Control keys\n");
    printf("Move left: %c, right: %c, up: %c, down: %c\n", MOVE_LEFT_KEY, MOVE_RIGHT_KEY, MOVE_UP_KEY, MOVE_DOWN_KEY);

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    vr_set_render_buffer(display_buffer());

    while (!quit_flag) {
        render_buffer_zero(display_buffer());

        for (int i=0; i<LPF; i++) {
            x0 = rand()%display_buffer()->width+cx+px;    y0 = rand()%display_buffer()->height+cy+py;
            x1 = rand()%display_buffer()->width+cx+px;    y1 = rand()%display_buffer()->height+cy+py;
            line_flat(x0, y0, x1, y1,
                &(VEC_3){(rand()&255)/255., (rand()&255)/255., (rand()&255)/255.}
            );
        }
        display_show(0);
        periodic_fps_printf(1.0);
        cx = cy = 0;
        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
            }
            else if (event->type == KEY_HOLD) {
                switch((int)event->code) {
                    case MOVE_RIGHT_KEY:
                        cx += event->hold_time*vx; break;
                    case MOVE_LEFT_KEY:
                        cx -= event->hold_time*vx; break;
                    case MOVE_DOWN_KEY:
                        cy += event->hold_time*vy; break;
                    case MOVE_UP_KEY:
                        cy -= event->hold_time*vy; break;
                    default:
                        break;
                }
            }
            else if (event->type == KEY_RELEASED) {
                switch((int)event->code) {
                    case MOVE_RIGHT_KEY:
                        cx = 0;    px += event->hold_time*vx; break;
                    case MOVE_LEFT_KEY:
                        cx = 0;    px -= event->hold_time*vx; break;
                    case MOVE_DOWN_KEY:
                        cy = 0;    py += event->hold_time*vy; break;
                    case MOVE_UP_KEY:
                        cy = 0;    py -= event->hold_time*vy; break;
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

    engine_cleanup();
    return 0;
}