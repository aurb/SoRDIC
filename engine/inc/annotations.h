#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include "engine_types.h"

FLOAT* annotations_last_t();
INT* annotations_last_v();
INT* annotations_prev_v();

int annotations_read(const char *filename);
void annotations_track(FLOAT t);
void annotations_free();

#endif