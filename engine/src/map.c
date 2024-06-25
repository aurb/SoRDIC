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
