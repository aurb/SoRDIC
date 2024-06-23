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
    const char *window_title = "SoRDIC Color conversions example";
    EVENT *event = NULL;
    bool quit_flag = false;
    int h_max = 0, s_max = 0, l_max = 0, x = 0, y = 0, x_tiles = 0, y_tiles = 0;
    COLOR color;

    #ifdef FULL_DESKTOP
        engine_init(0, 0, FULLSCREEN_CURRENT_MODE, window_title);
    #else
        engine_init(DISPLAY_W, DISPLAY_H, 0, window_title);
    #endif

    RENDER_BUFFER_zero(display_buffer());

    ARGB_PIXEL *display = display_buffer()->map->data;
    y_tiles = 5; // Number of tiles along vertical axis
    s_max = display_buffer()->height/y_tiles; //Number of different saturation values inside the tile
    x_tiles = display_buffer()->width/s_max; // Number of tiles along horizontal axis
    h_max = display_buffer()->width/x_tiles; //Number of different hue values inside the tile
    l_max = x_tiles * y_tiles; //Number of different lightness values on the display
    // Place tiles on the display surface. From left to right and then from top to bottom.
    for (y = 0; y < display_buffer()->height; y ++) {
        for (x = 0; x < display_buffer()->width; x ++) {
            // Each pixel on the tile represents one hue & saturation combination.
            color.h = (FLOAT)(x % h_max)/(FLOAT)(h_max-1);
            color.s = (FLOAT)(y % s_max)/(FLOAT)(s_max-1);
            // All pixels inside "tile" have same lightness value
            color.l = (FLOAT)((x/h_max)%x_tiles + ((y/s_max)%y_tiles)*x_tiles)/(FLOAT)(l_max-1);
            // Convert from HSL to RGB to ARGB_PIXEL and store it on target position on the screen
            display[y*display_buffer()->width + x] = COLOR_to_ARGB_PIXEL(COLOR_hsl_to_rgb(&color));
        }
    }

    display_show(0);

    while (!quit_flag) {
        event = engine_poll_events();
        while(event->type != NO_EVENT) { //iterate events until none is left to process
            if (event->type == QUIT_REQUEST) {
                quit_flag = true;
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