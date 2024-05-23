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
#include "assets_2d.h"

MAP *read_map_from_image(const char * const map_filename, INT u_wrap_margin) {
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

    MAP *map = calloc(1, sizeof(MAP));
    map->map = calloc(w*h, sizeof(RGB_PIXEL));
    for (INT y = 0; y < h; y++)
        for (INT x = 0; x < w; x++)
            map->map[w*y + x] = ((RGB_PIXEL*)(map_surface->pixels + pitch * (y%w)))[x];
    map->width = w;
    map->height = h-u_wrap_margin;
    map->height_with_margin = h;

    SDL_FreeSurface(map_surface);
    SDL_FreeSurface(image_surface);
    return map;
}

BUMP_MAP *convert_map_to_bump_map(MAP* in_map, FLOAT margin) {
    BUMP_MAP *bump_map = calloc(1, sizeof(BUMP_MAP));
    INT w = in_map->width;
    INT h = in_map->height_with_margin;
    FLOAT dlx = 0., dly = 0., dl_max = 0., dl_scale = 0.;
    bump_map->map = calloc(w*h, sizeof(BUMP_PIXEL));
    bump_map->width = w;
    bump_map->height = h;
    bump_map->margin = margin;
    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (utils_pix_2_L(in_map->map[w*y + (x+2)%w]) - utils_pix_2_L(in_map->map[w*y + x]));
            dly = (utils_pix_2_L(in_map->map[w*((y+2)%h) + x]) - utils_pix_2_L(in_map->map[w*y + x]));
            if (dl_max < dlx) dl_max = dlx;
            if (dl_max < dly) dl_max = dly;
        }
    }

    dl_scale = dl_max != 0.0 ? (FLOAT)(w*margin) / dl_max : 0.0;

    for (INT y = 0; y < h; y++) {
        for (INT x = 0; x < w; x++) {
            dlx = (utils_pix_2_L(in_map->map[w*y + (x+2)%w]) - utils_pix_2_L(in_map->map[w*y + x]));
            dly = (utils_pix_2_L(in_map->map[w*((y+2)%h) + x]) - utils_pix_2_L(in_map->map[w*y + x]));

            bump_map->map[w*y + x] = (((BUMP_PIXEL)(dly*dl_scale)<<16)&0xFFFF0000) | ((BUMP_PIXEL)(dlx*dl_scale)&0x0000FFFF);
        }
    }
    return bump_map;
}

void map_free(MAP* map) {
    if (map != NULL) {
        if (map->map != NULL) {
            free(map->map);
        }
        free(map);
    }
}

void bump_map_free(BUMP_MAP* map) {
    if (map != NULL) {
        if (map->map != NULL) {
            free(map->map);
        }
        free(map);
    }
}
