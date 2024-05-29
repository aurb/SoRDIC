#include <math.h>
#include "engine_types.h"

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
