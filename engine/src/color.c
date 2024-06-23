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

#define COLOR_CNT (16) //power of 2
COLOR color_buf[COLOR_CNT];
static INT color_idx = 0;

#define A (*a)
#define B (*b)
#define C (*c)
#define V (color_buf[color_idx])

void inc_color_index() {
    color_idx = (color_idx + 1) & (COLOR_CNT-1);
}

ARGB_PIXEL COLOR_to_ARGB_PIXEL(COLOR *o) {
    return (ARGB_PIXEL)(255.99*o->a)<<A_SHIFT | (ARGB_PIXEL)(255.99*o->r)<<R_SHIFT | (ARGB_PIXEL)(255.99*o->g)<<G_SHIFT | (ARGB_PIXEL)(255.99*o->b)<<B_SHIFT;
}

FLOAT ARGB_PIXEL_to_l(ARGB_PIXEL p) {
    return (FLOAT)(((p&B_MASK)>>B_SHIFT)+2*((p&G_MASK)>>G_SHIFT)+((p&R_MASK)>>R_SHIFT))/1024.;
}

COLOR* COLOR_scale(COLOR *a, FLOAT s) {
    inc_color_index();
    V.r = a->r * s;
    V.g = a->g * s;
    V.b = a->b * s;
    return &V;
}

COLOR* COLOR_blend(COLOR *a, COLOR *b, FLOAT p) {
    inc_color_index();
    V.a = a->a*p + b->a*(1-p);
    V.r = a->r*p + b->r*(1-p);
    V.g = a->g*p + b->g*(1-p);
    V.b = a->b*p + b->b*(1-p);
    return &V;
}

COLOR* COLOR_add(COLOR *a, COLOR *b) {
    inc_color_index();
    V.a = (a->a + b->a)*0.5;
    V.r = a->r + b->r;
    V.g = a->g + b->g;
    V.b = a->b + b->b;
    return &V;
}

COLOR* COLOR_add_sat(COLOR *a, COLOR *b) {
    inc_color_index();
    V.a = (a->a + b->a)*0.5;
    V.r = a->r + b->r;
    V.g = a->g + b->g;
    V.b = a->b + b->b;
    if (V.r > 1.0) V.r = 1.0;
    if (V.g > 1.0) V.g = 1.0;
    if (V.b > 1.0) V.b = 1.0;
    return &V;
}

COLOR* COLOR_mul(COLOR *a, COLOR *b) {
    inc_color_index();
    V.r = a->r * b->r;
    V.g = a->g * b->g;
    V.b = a->b * b->b;
    return &V;
}

COLOR* COLOR_hsl_to_rgb(COLOR *o) {
    //HSL to RGB conversion algorithm as per https://en.wikipedia.org/wiki/HSL_and_HSV
    FLOAT c = ((FLOAT)1.0 - FABS((FLOAT)2.*o->l - (FLOAT)1.0)) * o->s;
    FLOAT hp = o->h*6.0;
    FLOAT x = c * ((FLOAT)1.0 - FABS(FMOD(hp, (FLOAT)2.0) - (FLOAT)1.0));
    FLOAT lm = o->l - c/2; //lightness match
    if (hp < 1.0) {
        o->r = lm + c;
        o->g = lm + x;
        o->b = lm;  }
    else if (hp < 2.0) {
        o->r = lm + x;
        o->g = lm + c;
        o->b = lm;  }
    else if (hp < 3.0) {
        o->r = lm;
        o->g = lm + c;
        o->b = lm + x;  }
    else if (hp < 4.0) {
        o->r = lm;
        o->g = lm + x;
        o->b = lm + c;  }
    else if (hp < 5.0) {
        o->r = lm + x;
        o->g = lm;
        o->b = lm + c;  }
    else if (hp < 6.0) {
        o->r = lm + c;
        o->g = lm;
        o->b = lm + x;  }

    return o;
}