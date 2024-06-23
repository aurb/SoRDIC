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

FLOAT transit_line(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT ret = t/max_t;
    if (ret < 0.0) ret = 0.0;
    else if (ret > 1.0) ret = 1.0;
    return ret;
}

FLOAT transit_square_low(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = t/max_t;
    FLOAT ret = x*x;
    if (ret < 0.0) ret = 0.0;
    else if (ret > 1.0) ret = 1.0;
    return ret;
}

FLOAT transit_square_high(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = 1.0 - t/max_t;
    if (x < 0.0) x = 0.0;
    else if (x > 1.0) x = 1.0;
    FLOAT ret = 1.0 - x*x;
    return ret;
}

FLOAT transit_cube_low(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = t/max_t;
    if (x < 0.0) x = 0.0;
    else if (x > 1.0) x = 1.0;
    FLOAT ret = x*x*x;
    return ret;
}

FLOAT transit_cube_high(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = 1.0 - t/max_t;
    if (x < 0.0) x = 0.0;
    else if (x > 1.0) x = 1.0;
    FLOAT ret = 1.0 - x*x*x;
    return ret;
}

FLOAT transit_quarter_wave(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = t*PI_2/max_t;
    if (x > PI_2)
        return 1.0;
    else if (x < 0.0)
        return 0.0;
    else
        return sin(x);
}

FLOAT transit_half_wave(FLOAT t, FLOAT max_t) {
    if (max_t == 0.0) return 0.0;
    FLOAT x = t*PI/max_t;
    if (x > PI)
        return 1.0;
    else if (x < 0.0)
        return 0.0;
    else
        return 0.5*-cos(x)+0.5;
}
