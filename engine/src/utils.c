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

#include "engine_types.h"
#include <stdio.h>
#include <string.h>
RGB_PIXEL utils_RGB_2_pix(VEC_3 *c) {
    return (RGB_PIXEL)(255.*(*c)[2])<<16 | (RGB_PIXEL)(255.*(*c)[1])<<8 | (RGB_PIXEL)(255.*(*c)[0]);
}

FLOAT utils_pix_2_L(RGB_PIXEL p) {
    return (FLOAT)((p&0x000000FF)+2*((p>>8)&0x000000FF)+((p>>16)&0x000000FF))/1024.;
}

VEC_3* utils_blend_RGB(VEC_3 *c, VEC_3 *a, VEC_3 *b, FLOAT p) {
    for (INT i=0; i < 3; i++) {
        (*c)[i] = (*a)[i]*p + (*b)[i]*(1-p);
    }
    return c;
}

#define FILE_PATH_MAX_LENGTH 256
char call_path[FILE_PATH_MAX_LENGTH];

const char *runtime_file_path(const char *argv0, const char* relative_file_path) {
    strcpy(call_path, argv0);
    char *last_slash = strrchr(call_path, '/');
    if (last_slash != NULL) {
        last_slash++;
        strcpy(last_slash, relative_file_path);
        return call_path;
    }
    else {
        return relative_file_path;
    }
}