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

INT *map_gen_buffer = NULL;
DISCRETE_GRADIENT *map_gen_dg_1024 = NULL;
DISCRETE_GRADIENT *map_gen_dg_256 = NULL;

void map_generator_init() {
    map_gen_buffer = calloc(100000, sizeof(INT));
    map_gen_dg_1024 = DISCRETE_GRADIENT_alloc(1024);
    map_gen_dg_256 = DISCRETE_GRADIENT_alloc(256);
}

void map_generator_cleanup() {
    free(map_gen_buffer);
    DISCRETE_GRADIENT_free(map_gen_dg_1024);
    DISCRETE_GRADIENT_free(map_gen_dg_256);
}

/**
 * @brief Rasterize sine plasma pattern using provided color gradient
 * Rasterize sine plasma pattern using provided color gradient
 * For floating-point parameters(except scale) value 1.0 corresponds to map->width or map->height (depending which one is bigger).
 * @param map Target map for rasterizing
 * @param g Color gradient to rasterize plasma with
 * @param scale Scale factor [0.0 - +inf]
 * @param s2xA Amplitude of X deformation sine wave [0.0 - 1.0]
 * @param s2xT Period of X deformation sine wave [0.0 - 1.0]
 * @param xo Phase of X deformation sine wave [0.0 - 1.0]
 * @param s2yA Amplitude of Y deformation sine wave [0.0 - 1.0]
 * @param s2yT Period of Y deformation sine wave [0.0 - 1.0]
 * @param yo Phase of Y deformation sine wave [0.0 - 1.0]
 */
void ARGB_MAP_plasma_pattern(ARGB_MAP *map, GRADIENT *g, FLOAT scale, FLOAT s2xA, FLOAT s2xT, FLOAT xo, FLOAT s2yA, FLOAT s2yT, FLOAT yo) {
    INT i = 0, x = 0, y = 0, x0 = 0, y0 = 0, offs = 0, sin2_y_b = 0;
    DISCRETE_GRADIENT_from_GRADIENT(map_gen_dg_1024, g);
    const INT base_length = map->height > map->width ? map->height : map->width;
    const INT sin1_length = (s2xA+yo > s2yA+xo ? s2xA+1.0+yo : s2yA+1.0+xo) * base_length;
    const INT sin2x_length = map->width;
    const INT sin2y_length = map->height;
    s2xA *= base_length;
    s2xT *= base_length;
    s2yA *= base_length;
    s2yT *= base_length;
    x0 = xo * base_length;
    y0 = yo * base_length;

    INT *sin1 = map_gen_buffer;
    INT *sin2_x = sin1 + sin1_length;
    INT *sin2_y = sin2_x + sin2x_length;

    for (i = 0; i < sin1_length; i++) {
        sin1[i] = map_gen_dg_1024->length*(0.25*(sin(i*scale*TWOPI/map->width)+1.0));
    }
    for (i = 0; i < sin2x_length; i++) {
        sin2_x[i] = s2xA*(0.5*(sin(i*TWOPI/s2xT)+1.0));
    }
    for (i = 0; i < sin2y_length; i++) {
        sin2_y[i] = s2yA*(0.5*(sin(i*TWOPI/s2yT)+1.0));
    }

    for(y=y0, offs=0; y < map->height+y0; y++) {
        sin2_y_b = x0+sin2_y[y-y0];
        for(x=0; x < map->width; x++, offs++)
            ((ARGB_PIXEL*)map->data)[offs] = map_gen_dg_1024->pixval[sin1[y+sin2_x[x]] + sin1[x+sin2_y_b]];
    }
}

void ARGB_MAP_vertical_pattern(ARGB_MAP *map, GRADIENT *g) {
    INT x=0, y=0, offs=0;
    ARGB_PIXEL pixval;
    for(y=0, offs=0; y < map->height; y++) {
        pixval = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, (FLOAT)y/(FLOAT)(map->height-1)));
        for(x=0; x < map->width; x++, offs++) {
            ((ARGB_PIXEL*)map->data)[offs] = pixval;
        }
    }
}

void ARGB_MAP_horizontal_pattern(ARGB_MAP *map, GRADIENT *g) {
    INT x=0, y=0, offs=0;
    for(y=0, offs=0; y < map->height; y++) {
        for(x=0; x < map->width; x++, offs++) {
            ((ARGB_PIXEL*)map->data)[offs] = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, (FLOAT)x/(FLOAT)(map->width-1)));
        }
    }
}

void ARGB_MAP_diagonal_pattern(ARGB_MAP *map, GRADIENT *g) {
    INT x = 0, y = 0, xpy = map->width+map->height-2, offs = 0;
    for(y = 0, offs=0; y < map->height; y++) {
        for(x=0; x < map->width; x++, offs++) {
            ((ARGB_PIXEL*)map->data)[offs] = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, (FLOAT)(x+y)/(FLOAT)(xpy)));
        }
    }
}

void ARGB_MAP_radial_pattern(ARGB_MAP *map, GRADIENT *g, INT x0, INT y0) {
    INT x=0, y=0, offs=0;
    FLOAT d = 0.0, r = map->width > map->height ? map->width/2. : map->height/2.;
    for(y=0, offs=0; y < map->height; y++) {
        for(x=0; x < map->width; x++, offs++) {
            d = sqrt((x-x0)*(x-x0) + (y-y0)*(y-y0))/r;
            ((ARGB_PIXEL*)map->data)[offs] = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, d));
        }
    }
}

void ARGB_MAP_xor_pattern(ARGB_MAP *map, GRADIENT *g) {
    INT x=0, y=0, offs=0;
    for(y=0, offs=0; y < map->height; y++) {
        for(x=0; x < map->width; x++, offs++) {
            ((ARGB_PIXEL*)map->data)[offs] = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, (FLOAT)((x^y)&255)/255.0));
        }
    }
}

