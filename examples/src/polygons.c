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

#define MOVE_LEFT_KEY 'a'
#define MOVE_RIGHT_KEY 'd'
#define MOVE_UP_KEY 'w'
#define MOVE_DOWN_KEY 's'

inline uint64_t my_abs(const int x) {
    return x<0 ? -x : x;
}
int main(int argc, char *argv[])
{
    const char *window_title = "SoRDIC 2D Random Polygons Example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int PPF = 1000; //polygons per frame
    int polygon_cnt = 0;
    double r = 0.0;
    const double angmarg = 6.2831 / 180.0;
    double vx = 200.0, vy = 200.0; //object linear velocities when corresponding key pressed
    int px = 0, py = 0; //object position
    int cx = 0, cy = 0;

    #define VCOUNT 5
    PROJECTION_COORD vv[VCOUNT]; //vertex values
    PROJECTION_COORD *vp[VCOUNT]; //vertex pointers

    if (argc != 2) {
        printf("Usage: %s polygons_per_frame\n", argv[0]);
        printf("Starting with default polygons_per_frame=%d\n", PPF);
    }
    else {
        PPF = atoi(argv[1]);
    }

    printf("Control keys\n");
    printf("Move left: %c, right: %c, up: %c, down: %c\n", MOVE_LEFT_KEY, MOVE_RIGHT_KEY, MOVE_UP_KEY, MOVE_DOWN_KEY);

    for (int i=0; i<VCOUNT; i++)
        vp[i] = vv + i;

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    vr_set_render_buffer(display_buffer());

    r = display_buffer()->height/2-6;
    px = display_buffer()->width/2; py = display_buffer()->height/2;

    while (!quit_flag) {
        RENDER_BUFFER_zero(display_buffer());

        for (int i=0; i<PPF; i++) {
            double ang = ((double)rand()/(double)(RAND_MAX))*6.2831;
            double ang_range = (6.2831-angmarg*(double)(VCOUNT))/(double)(VCOUNT);
            for (int i=0; i<VCOUNT; i++) {
                vv[i][0] = r*sin(ang) + cx+px;
                vv[i][1] = r*cos(ang) + cy+py;
                ang -= angmarg + ((double)rand()/(double)(RAND_MAX))*ang_range;
            }
            polygon_solid(
                VCOUNT, vp,
                &(COLOR){.r = (rand()&255)/255., .g = (rand()&255)/255., .b = (rand()&255)/255.}
            );
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

    engine_cleanup();
    return 0;
}