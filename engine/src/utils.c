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