#ifndef TRANSITIONS_H
#define TRANSITIONS_H

#include "engine_types.h"

FLOAT transit_line(FLOAT t, FLOAT max_t);
FLOAT transit_square_low(FLOAT t, FLOAT max_t);
FLOAT transit_square_high(FLOAT t, FLOAT max_t);
FLOAT transit_cube_low(FLOAT t, FLOAT max_t);
FLOAT transit_cube_high(FLOAT t, FLOAT max_t);
FLOAT transit_quarter_wave(FLOAT t, FLOAT max_t);
FLOAT transit_half_wave(FLOAT t, FLOAT max_t);

#endif