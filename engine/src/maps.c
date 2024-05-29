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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include "engine_types.h"
#include "engine.h"
#include "maps.h"

RGB_MAP *rgb_map_alloc(INT width, INT height, INT wrap_margin) {
    RGB_MAP *map = calloc(1, sizeof(RGB_MAP));
    map->data = calloc(width*height, sizeof(RGB_PIXEL));
    map->width = width;
    map->height = height-wrap_margin;
    map->height_with_margin = height;
    return map;
}

BUMP_MAP *bump_map_alloc(INT width, INT height) {
    BUMP_MAP *map = calloc(1, sizeof(BUMP_MAP));
    map->data = calloc(width*height, sizeof(BUMP_PIXEL));
    map->width = width;
    map->height = height;
    return map;
}

Z_MAP *z_map_alloc(INT width, INT height) {
    Z_MAP *map = calloc(1, sizeof(Z_MAP));
    map->data = calloc(width*height, sizeof(Z_PIXEL));
    map->width = width;
    map->height = height;
    return map;
}

void rgb_map_clear(RGB_MAP *map) {
    if (map == NULL)
        return;
    memset(map->data, 0, map->width*map->height*sizeof(RGB_PIXEL));
}

void z_map_clear(Z_MAP *map) {
    if (map == NULL)
        return;
    memset(map->data, 0xFF, map->width*map->height*sizeof(Z_PIXEL));
}

void rgb_map_fill(RGB_MAP *map, VEC_3 *color) {
    if (map == NULL)
        return;
    RGB_PIXEL pix_val = utils_RGB_2_pix(color);
    for (INT offs = 0; offs < map->width*map->height; offs++)
        map->data[offs] = pix_val;
}

void rgb_map_copy(RGB_MAP *dst, RGB_MAP *src) {
    if (dst == NULL || src == NULL || dst->data == NULL || src->data==NULL ||
        dst->width != src->width || dst->height != src->height) {
        return;
    }
    memcpy(dst->data, src->data, dst->width*dst->height*sizeof(RGB_PIXEL));
}

void z_map_copy(Z_MAP *dst, Z_MAP *src) {
    if (dst == NULL || src == NULL || dst->data == NULL || src->data==NULL ||
        dst->width != src->width || dst->height != src->height) {
        return;
    }
    memcpy(dst->data, src->data, dst->width*dst->height*sizeof(Z_PIXEL));
}

void rgb_map_fixed_mask(RGB_MAP *dst, RGB_MAP **src, RGB_MAP *mask) {
    for (INT offs = 0; offs < mask->height * mask->width; offs++)
        dst->data[offs] = src[mask->data[offs]]->data[offs];
}

void rgb_map_blend_dither(RGB_MAP *dst, RGB_MAP *rgb0_ch,
                                  RGB_MAP *rgb1_ch, RGB_MAP *alpha_ch) {
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    RGB_MAP *src[2] = {rgb0_ch, rgb1_ch};
    for (INT offs = 0; offs < alpha_ch->height*alpha_ch->width; offs++)
        dst->data[offs] = src[LIMIT_255(PRN(seed)) < alpha_ch->data[offs] ? 1 : 0]->data[offs];
}

void rgb_map_blend_mul(RGB_MAP *dst, RGB_MAP *rgb0_ch,
                                  RGB_MAP *rgb1_ch, RGB_MAP *alpha_ch) {
    RGB_PIXEL r0, r1, g0, g1, b0, b1;
    for (INT offs = 0; offs < alpha_ch->height*alpha_ch->width; offs++) {
        b0 = rgb0_ch->data[offs]&0x000000FF;
        b1 = rgb1_ch->data[offs]&0x000000FF;
        g0 = rgb0_ch->data[offs]&0x0000FF00;
        g1 = rgb1_ch->data[offs]&0x0000FF00;
        r0 = rgb0_ch->data[offs]&0x00FF0000;
        r1 = rgb1_ch->data[offs]&0x00FF0000;
        dst->data[offs] = ((b1*alpha_ch->data[offs] + b0*(256-alpha_ch->data[offs])) |
                       ((g1*alpha_ch->data[offs] + g0*(256-alpha_ch->data[offs]))& 0x00FF0000) |
                       ((r1*alpha_ch->data[offs] + r0*(256-alpha_ch->data[offs]))& 0xFF000000)) >> 8;
    }
}

/**
 * Calculates a + b with saturation for rgb parts of a and b. Result stored in a.
 * Addition starts at position (x, y) in a
 * Buffers have to have same dimensions.
 */
void rgb_map_sat_add(RGB_MAP *a, RGB_MAP *b) {
    INT o; /** Offset for traversing buffers*/
    RGB_PIXEL s;
    if (a->width != b->width || a->height != b->height) {
        return;
    }
    for (o = 0; o < a->width * a->height; o++) {
        /** Add all components */
        s = (a->data[o]&0x00FEFEFF) + (b->data[o]&0x00FEFEFF);
        /** Saturate sums of each component */
        if (s & 0x01000000) {
            if (s & 0x00010000) {
                if (s & 0x00000100) {
                    s |= 0x00FFFFFF;
                }
                else {
                    s |= 0x00FFFF00;
                }
            }
            else {
                if (s & 0x00000100) {
                    s |= 0x00FF00FF;
                }
                else {
                    s |= 0x00FF0000;
                }
            }
        }
        else {
            if (s & 0x00010000) {
                if (s & 0x00000100) {
                    s |= 0x0000FFFF;
                }
                else {
                    s |= 0x0000FF00;
                }
            }
            else {
                if (s & 0x00000100) {
                    s |= 0x000000FF;
                }
                // else do nothing (no saturation at any component)
            }
        }
        ((RGB_PIXEL*)(a->data))[o] = s;
    }
}

RGB_MAP *read_map_from_image(const char * const map_filename, INT u_wrap_margin) {
    if (map_filename == NULL)
        return NULL;

    SDL_Surface* image_surface = IMG_Load(map_filename);
    if (image_surface == NULL) {
        printf("Error IMG_Load(%s): %s\n", map_filename, SDL_GetError());
        return NULL;
    }

    SDL_PixelFormat pix_format;
    pix_format.format = SDL_PIXELFORMAT_ARGB8888;
    pix_format.palette = NULL;
    pix_format.BitsPerPixel = 32;
    pix_format.BytesPerPixel = 4;
    pix_format.Amask = 0xFF000000;
    pix_format.Rmask = 0x00FF0000;
    pix_format.Gmask = 0x0000FF00;
    pix_format.Bmask = 0x000000FF;
    SDL_Surface* map_surface = SDL_ConvertSurface(image_surface, &pix_format, 0);
    if (map_surface == NULL) {
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    INT w = map_surface->w;
    INT h = map_surface->h+u_wrap_margin;
    INT pitch = map_surface->pitch;

    RGB_MAP *map = rgb_map_alloc(w, h, u_wrap_margin);
    for (INT y = 0; y < h; y++)
        for (INT x = 0; x < w; x++)
            map->data[w*y + x] = ((RGB_PIXEL*)(map_surface->pixels + pitch * (y%w)))[x];

    SDL_FreeSurface(map_surface);
    SDL_FreeSurface(image_surface);
    return map;
}

BUMP_MAP *convert_map_to_bump_map(RGB_MAP* in_map, FLOAT margin) {
    INT w = in_map->width;
    INT h = in_map->height_with_margin;
    FLOAT dlx = 0., dly = 0., dl_max = 0., dl_scale = 0.;
    BUMP_MAP *bump_map = bump_map_alloc(w, h);
    bump_map->margin = margin;
    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (utils_pix_2_L(in_map->data[w*y + (x+2)%w]) - utils_pix_2_L(in_map->data[w*y + x]));
            dly = (utils_pix_2_L(in_map->data[w*((y+2)%h) + x]) - utils_pix_2_L(in_map->data[w*y + x]));
            if (dl_max < dlx) dl_max = dlx;
            if (dl_max < dly) dl_max = dly;
        }
    }

    dl_scale = dl_max != 0.0 ? (FLOAT)(w*margin) / dl_max : 0.0;

    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (utils_pix_2_L(in_map->data[w*y + (x+2)%w]) - utils_pix_2_L(in_map->data[w*y + x]));
            dly = (utils_pix_2_L(in_map->data[w*((y+2)%h) + x]) - utils_pix_2_L(in_map->data[w*y + x]));

            bump_map->data[w*y + x] = (((BUMP_PIXEL)(dly*dl_scale)<<16)&0xFFFF0000) | ((BUMP_PIXEL)(dlx*dl_scale)&0x0000FFFF);
        }
    }
    return bump_map;
}

void plasma_precalc(PLASMA_DATA *p, INT s1T, INT s2xA, INT s2xT, INT s2yA, INT s2yT) {
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

void plasma_rasterize(PLASMA_DATA *p, RGB_MAP *map, INT xo, INT yo) {
    INT x=0, y=0, offs=0, sin2_y_b=0;
    for(y=yo, offs=0; y < map->height+yo; y++) {
        sin2_y_b = xo+p->sin2_y[y-yo];
        for(x=0; x < map->width; x++, offs++)
            ((RGB_PIXEL*)map->data)[offs] = p->gradient[p->sin1[y+p->sin2_x[x]] + p->sin1[x+sin2_y_b]];
    }
}

void rgb_map_free(RGB_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}

void bump_map_free(BUMP_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}

void z_map_free(Z_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}

void plasma_free(PLASMA_DATA *p) {
    free(p->gradient);
    free(p->sin1);
    free(p->sin2_x);
    free(p->sin2_y);
}
