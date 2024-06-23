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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "engine.h"

ARGB_MAP *ARGB_MAP_alloc(INT width, INT height, INT wrap_margin) {
    ARGB_MAP *map = calloc(1, sizeof(ARGB_MAP));
    map->data = calloc(width*height, sizeof(ARGB_PIXEL));
    map->width = width;
    map->height = height-wrap_margin;
    map->height_with_margin = height;
    return map;
}

BUMP_MAP *BUMP_MAP_alloc(INT width, INT height) {
    BUMP_MAP *map = calloc(1, sizeof(BUMP_MAP));
    map->data = calloc(width*height, sizeof(BUMP_PIXEL));
    map->width = width;
    map->height = height;
    return map;
}

Z_MAP *Z_MAP_alloc(INT width, INT height) {
    Z_MAP *map = calloc(1, sizeof(Z_MAP));
    map->data = calloc(width*height, sizeof(Z_PIXEL));
    map->width = width;
    map->height = height;
    return map;
}

void ARGB_MAP_clear(ARGB_MAP *map) {
    if (map == NULL)
        return;
    memset(map->data, 0, map->width*map->height*sizeof(ARGB_PIXEL));
}

void Z_MAP_clear(Z_MAP *map) {
    if (map == NULL)
        return;
    memset(map->data, 0xFF, map->width*map->height*sizeof(Z_PIXEL));
}

void ARGB_MAP_fill(ARGB_MAP *map, COLOR *color) {
    if (map == NULL)
        return;
    ARGB_PIXEL pix_val = COLOR_to_ARGB_PIXEL(color);
    for (INT offs = 0; offs < map->width*map->height; offs++)
        map->data[offs] = pix_val;
}

void ARGB_MAP_copy(ARGB_MAP *dst, ARGB_MAP *src) {
    if (dst == NULL || src == NULL || dst->data == NULL || src->data==NULL ||
        dst->width != src->width || dst->height != src->height) {
        return;
    }
    memcpy(dst->data, src->data, dst->width*dst->height*sizeof(ARGB_PIXEL));
}

void Z_MAP_copy(Z_MAP *dst, Z_MAP *src) {
    if (dst == NULL || src == NULL || dst->data == NULL || src->data==NULL ||
        dst->width != src->width || dst->height != src->height) {
        return;
    }
    memcpy(dst->data, src->data, dst->width*dst->height*sizeof(Z_PIXEL));
}

void ARGB_MAP_multiplex(ARGB_MAP *out, ARGB_MAP **in, ARGB_MAP *mask) {
    for (INT offs = 0; offs < mask->height * mask->width; offs++)
        out->data[offs] = in[mask->data[offs]]->data[offs];
}

void ARGB_MAP_fade_dither_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                            FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        out->data[offs] = r < blend ? map->data[offs] : (r == 255 && blend == 255 ? map->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                         FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT pfa = p*255.99;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pixval = ((ARGB_PIXEL*)map->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_global(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                             FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        out->data[offs] = (r < blend ? map1 : (r == 255 && blend == 255 ? map1 : map0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_global(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                          FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    INT pfa = p*255.99;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pixval = ((ARGB_PIXEL*)map0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)map1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_fade_dither_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                                ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color), a;
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        a = p->data[offs]>>24;
        out->data[offs] = r < a ? map->data[offs] : (r == 255 && a == 255 ? map->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                             ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] );
        pixval = ((ARGB_PIXEL*)map->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_per_pixel(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                                 ARGB_MAP *p) {
    ARGB_PIXEL a;
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        a = p->data[offs]>>24;
        r = LIMIT_255(PRN(seed));
        out->data[offs] = (r < a ? map1 : (r == 255 && a == 255 ? map1 : map0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_per_pixel(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                              ARGB_MAP *p) {
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa, pixval;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] );
        pixval = ((ARGB_PIXEL*)map0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)map1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_fade_dither_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                                  FLOAT f, ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*(p->data[offs]>>24);
        r = LIMIT_65535(PRN(seed));
        out->data[offs] = r < pfa ? map->data[offs] : (r == 65535 && pfa == 65535 ? map->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *map,
                               FLOAT f, ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] ) >> 8;
        pixval = ((ARGB_PIXEL*)map->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_f_per_pixel(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                                   FLOAT f, ARGB_MAP *p) {
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*(p->data[offs]>>24);
        r = LIMIT_65535(PRN(seed));
        out->data[offs] = (r < pfa ? map1 : (r == 65535 && pfa == 65535 ? map1 : map0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_f_per_pixel(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1,
                                FLOAT f, ARGB_MAP *p) {
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] ) >> 8;
        pixval = ((ARGB_PIXEL*)map0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)map1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

/**
 * Calculates a + b with saturation for map parts of a and b. Result stored in a.
 * Addition starts at position (x, y) in a
 * Buffers have to have same dimensions.
 */
void ARGB_MAP_sat_add(ARGB_MAP *out, ARGB_MAP *map0, ARGB_MAP *map1) {
    INT offs; /** Offset for traversing buffers*/
    ARGB_PIXEL sum;
    if (map0->width != map1->width || map0->height != map1->height) {
        return;
    }
    for (offs = 0; offs < map0->width * map0->height; offs++) {
        /** Add all components */
        sum = (map0->data[offs]&0x00FEFEFF) + (map1->data[offs]&0x00FEFEFF);
        /** Saturate sums of each component */
        if (sum & 0x01000000) {
            if (sum & 0x00010000) {
                if (sum & 0x00000100) {
                    sum |= 0x00FFFFFF;
                }
                else {
                    sum |= 0x00FFFF00;
                }
            }
            else {
                if (sum & 0x00000100) {
                    sum |= 0x00FF00FF;
                }
                else {
                    sum |= 0x00FF0000;
                }
            }
        }
        else {
            if (sum & 0x00010000) {
                if (sum & 0x00000100) {
                    sum |= 0x0000FFFF;
                }
                else {
                    sum |= 0x0000FF00;
                }
            }
            else {
                if (sum & 0x00000100) {
                    sum |= B_MASK;
                }
                // else do nothing (no saturation at any component)
            }
        }
        ((ARGB_PIXEL*)(out->data))[offs] = sum;
    }
}

ARGB_MAP *ARGB_MAP_read_image(const char * const map_filename, INT u_wrap_margin) {
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
    pix_format.Bmask = B_MASK;
    //After conversion input images that have no alpha channel
    //have it by default set to 0xFF on all pixels.
    SDL_Surface* map_surface = SDL_ConvertSurface(image_surface, &pix_format, 0);
    if (map_surface == NULL) {
        printf("%s\n", SDL_GetError());
        return NULL;
    }
    INT w = map_surface->w;
    INT h = map_surface->h+u_wrap_margin;
    INT pitch = map_surface->pitch;

    ARGB_MAP *map = ARGB_MAP_alloc(w, h, u_wrap_margin);
    for (INT y = 0; y < h; y++)
        for (INT x = 0; x < w; x++)
            map->data[w*y + x] = ((ARGB_PIXEL*)(map_surface->pixels + pitch * (y%w)))[x];

    SDL_FreeSurface(map_surface);
    SDL_FreeSurface(image_surface);
    return map;
}

BUMP_MAP *BUMP_MAP_from_ARGB_MAP(ARGB_MAP* in_map, FLOAT margin) {
    INT w = in_map->width;
    INT h = in_map->height_with_margin;
    FLOAT dlx = 0., dly = 0., dl_max = 0., dl_scale = 0.;
    BUMP_MAP *bump_map = BUMP_MAP_alloc(w, h);
    bump_map->margin = margin;
    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (ARGB_PIXEL_to_l(in_map->data[w*y + (x+2)%w]) - ARGB_PIXEL_to_l(in_map->data[w*y + x]));
            dly = (ARGB_PIXEL_to_l(in_map->data[w*((y+2)%h) + x]) - ARGB_PIXEL_to_l(in_map->data[w*y + x]));
            if (dl_max < dlx) dl_max = dlx;
            if (dl_max < dly) dl_max = dly;
        }
    }

    dl_scale = dl_max != 0.0 ? (FLOAT)(w*margin) / dl_max : 0.0;

    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (ARGB_PIXEL_to_l(in_map->data[w*y + (x+2)%w]) - ARGB_PIXEL_to_l(in_map->data[w*y + x]));
            dly = (ARGB_PIXEL_to_l(in_map->data[w*((y+2)%h) + x]) - ARGB_PIXEL_to_l(in_map->data[w*y + x]));

            bump_map->data[w*y + x] = (((BUMP_PIXEL)(dly*dl_scale)<<16)&0xFFFF0000) | ((BUMP_PIXEL)(dlx*dl_scale)&0x0000FFFF);
        }
    }
    return bump_map;
}


void ARGB_MAP_free(ARGB_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}

void BUMP_MAP_free(BUMP_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}

void Z_MAP_free(Z_MAP* map) {
    if (map != NULL) {
        if (map->data != NULL) {
            free(map->data);
        }
        free(map);
    }
}
