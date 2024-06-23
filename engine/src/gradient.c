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

void GRADIENT_add_point(GRADIENT *g, FLOAT t, COLOR *color) {
    g->t[g->count] = t;
    g->color[g->count] = *color;
    g->count++;
}

COLOR *GRADIENT_get_value(GRADIENT *g, FLOAT t) {
    //assumption: array t is sorted in ascending order
    INT i = 0;
    FLOAT t_start = g->t[0];
    FLOAT t_end = g->t[g->count-1];

    if (t < 0.0 || t > 1.0) {
        return &g->background;
    }
    else if (t < t_start) {
        return COLOR_blend(&g->color[0], &g->background, t/t_start);
    }
    else if (t > t_end) {
        return COLOR_blend(&g->background, &g->color[g->count-1], (t-t_end) / (1.0-t_end));
    }

    while (i < g->count && t >= g->t[i]) {
        i++;
    }
    return COLOR_blend( &g->color[i], &g->color[i-1], (t-g->t[i-1])/(g->t[i]-g->t[i-1]) );
}

DISCRETE_GRADIENT *DISCRETE_GRADIENT_alloc(INT length) {
    DISCRETE_GRADIENT *dg = calloc(1, sizeof(DISCRETE_GRADIENT));
    dg->pixval = calloc(length, sizeof(ARGB_PIXEL));
    dg->length = length;
    return dg;
}

void DISCRETE_GRADIENT_from_GRADIENT(DISCRETE_GRADIENT *dg, GRADIENT *g) {
    for (INT i = 0; i < dg->length; i++) {
        dg->pixval[i] = COLOR_to_ARGB_PIXEL(GRADIENT_get_value(g, (FLOAT)i/(FLOAT)(dg->length-1)));
    }
}

void DISCRETE_GRADIENT_free(DISCRETE_GRADIENT *dg) {
    free(dg->pixval);
    free(dg);
}