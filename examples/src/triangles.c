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

uint64_t my_abs(const int x) {
    return x<0 ? -x : x;
}
int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 2D Random Triangles Example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int PPF = 1000; //polygons per frame
    if (argc != 2) {
        printf("Usage: %s polygons_per_frame\n", argv[0]);
        printf("Starting with default polygons_per_frame=%d\n", PPF);
    }
    else {
        PPF = atoi(argv[1]);
    }

    printf("Control keys\n");
    printf("Move left: %c, right: %c, up: %c, down: %c\n", MOVE_LEFT_KEY, MOVE_RIGHT_KEY, MOVE_UP_KEY, MOVE_DOWN_KEY);

    uint64_t polygon_cnt = 0, pixel_cnt = 0;
    int x, y, dx1, dy1, dx2, dy2;
    double vx = 200.0, vy = 200.0; //object linear velocities when corresponding key pressed
    int px = 0, py = 0; //drawing area position
    int cx = 0, cy = 0; //drawing area position change

    #define VCOUNT 3
    PROJECTION_COORD vv[VCOUNT]; //vertex values
    PROJECTION_COORD *vp[VCOUNT]; //vertex pointers
    for (int i=0; i<VCOUNT; i++)
        vp[i] = vv + i;

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    vr_set_render_buffer(display_buffer());

    while (!quit_flag) {
        render_buffer_zero(display_buffer());

        for (int i=0; i<PPF; i++) {
            for (int i=0; i<VCOUNT; i++) {
                vv[i][0] = rand()%display_buffer()->width + cx+px;
                vv[i][1] = rand()%display_buffer()->height + cy+py;
            }

            // Check orentation of vertices in vv.
            // If it's not counter-clockwise, change it.
            dx1 = vv[1][0] - vv[0][0];
            dy1 = vv[1][1] - vv[0][1];
            dx2 = vv[2][0] - vv[0][0];
            dy2 = vv[2][1] - vv[0][1];
            if (dx1*dy2 < dx2*dy1) {
                x = vv[1][0];
                y = vv[1][1];
                vv[1][0] = vv[2][0];
                vv[1][1] = vv[2][1];
                vv[2][0] = x;
                vv[2][1] = y;
            }

            polygon_solid(
                VCOUNT, vp,
                &(VEC_3){(rand()&255)/255., (rand()&255)/255., (rand()&255)/255.}
            );

            pixel_cnt += my_abs((int64_t)(vv[1][0]-vv[0][0])*(int64_t)(vv[2][1]-vv[0][1]) - (int64_t)(vv[2][0]-vv[0][0])*(int64_t)(vv[1][1]-vv[0][1]))/2;
        }
        polygon_cnt += PPF;
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

    printf("\n%lu polygons_per_second\n", (uint64_t)(polygon_cnt / engine_run_stats().time));
    printf("%lu pixels_per_second\n", (uint64_t)(pixel_cnt / engine_run_stats().time));

    engine_cleanup();
    return 0;
}