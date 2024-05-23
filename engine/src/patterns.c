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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "patterns.h"

void sine_pattern_make_tables(SINE_PATTERN *p, INT s1T, INT s2xA, INT s2xT, INT s2yA, INT s2yT) {
    const INT GRADIENT_SIZE = 1024;
    const INT SIN_SIZE = 4096;
    p->gradient = calloc(GRADIENT_SIZE, sizeof(RGB_PIXEL));
    p->gradient_size = GRADIENT_SIZE;
    memset(p->gradient, 0, p->gradient_size*sizeof(RGB_PIXEL));

    p->sin1 = calloc(SIN_SIZE, sizeof(INT));
    p->sin2_x = calloc(SIN_SIZE, sizeof(INT));
    p->sin2_y = calloc(SIN_SIZE, sizeof(INT));
    p->sin_size = SIN_SIZE;
    for (INT i = 0; i<p->sin_size; i++) {
        p->sin1[i] = p->gradient_size*(0.25*sin(i*6.2832/s1T)+0.25);
        p->sin2_x[i] = s2xA*(0.5*sin(i*6.2832/s2xT)+0.5);
        p->sin2_y[i] = s2yA*(0.5*sin(i*6.2832/s2yT)+0.5);
    }
}

void sine_pattern_free(SINE_PATTERN *p) {
    free(p->gradient);
    free(p->sin1);
    free(p->sin2_x);
    free(p->sin2_y);
}

void sine_pattern_render(SINE_PATTERN *p, RENDER_BUFFER *rb, INT xo, INT yo) {
    INT x=0, y=0, offs=0, sin2_y_b=0;
    for(y=yo, offs=0; y < rb->height+yo; y++) {
        sin2_y_b = xo+p->sin2_y[y-yo];
        for(x=0; x < rb->width; x++, offs++)
            ((RGB_PIXEL*)rb->p)[offs] = p->gradient[p->sin1[y+p->sin2_x[x]] + p->sin1[x+sin2_y_b]];
    }
}