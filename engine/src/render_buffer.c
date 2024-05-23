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

#include <stdlib.h>
#include <string.h>
#include "engine_types.h"
#include "engine.h"

RENDER_BUFFER *render_buffer(INT width, INT height, INT z_buf_on) {
    RENDER_BUFFER *buf = calloc(1, sizeof(RENDER_BUFFER));
    buf->width = width;
    buf->height = height;
    buf->p = calloc(width*height, sizeof(RGB_PIXEL));
    if (z_buf_on == Z_BUFFER_ON) {
        buf->z = calloc(width*height, sizeof(RGB_PIXEL));
    }
    else {
        buf->z = NULL;
    }
    return buf;
}

void render_buffer_free(RENDER_BUFFER *buf) {
    if (buf->p != NULL) {
        free(buf->p);
        buf->p = NULL;
    }
    if (buf->z != NULL) {
        free(buf->z);
        buf->z = NULL;
    }
    free(buf);
}

void render_buffer_zero(RENDER_BUFFER *buf) {
    memset(buf->p, 0, buf->width*buf->height*sizeof(RGB_PIXEL));
    if (buf->z != NULL)
        memset(buf->z, 0xFF, buf->width*buf->height*sizeof(RGB_PIXEL));
}

void render_buffer_fill(RENDER_BUFFER *buf, VEC_3 *color) {
    RGB_PIXEL pix_val = utils_RGB_2_pix(color);
    for (INT offs = 0; offs < buf->width*buf->height; offs++)
        buf->p[offs] = pix_val;
    if (buf->z != NULL)
        memset(buf->z, 0xFF, buf->width*buf->height*sizeof(RGB_PIXEL));
}

void render_buffer_copy(RENDER_BUFFER *dst, RENDER_BUFFER *src) {
    if (dst->width != src->width || dst->height != src->height) {
        return;
    }
    memcpy(dst->p, src->p, dst->width*dst->height*sizeof(RGB_PIXEL));
    if (dst->z != NULL && src->z != NULL) {
        memcpy(dst->z, src->z, dst->width*dst->height*sizeof(RGB_PIXEL));
    }
}

void render_buffer_fixed_mask(RENDER_BUFFER *dst, RENDER_BUFFER **src, RENDER_BUFFER *mask) {
    for (INT offs = 0; offs < mask->height*mask->width; offs++)
        dst->p[offs] = src[mask->p[offs]]->p[offs];
}

void render_buffer_dither_opacity(RENDER_BUFFER *dst, RENDER_BUFFER *src_ch0,
                                  RENDER_BUFFER *src_ch1, RENDER_BUFFER *opacity) {
    //INT seed = display_run_stats().frames; //better at full framerate
    INT seed = (display_run_stats().frames%7) * 1000; //better with lower framerates
    RENDER_BUFFER *src[2] = {src_ch0, src_ch1};
    for (INT offs = 0; offs < opacity->height*opacity->width; offs++)
        dst->p[offs] = src[LIMIT_255(PRN(seed)) < opacity->p[offs] ? 1 : 0]->p[offs];
}

void render_buffer_full_opacity(RENDER_BUFFER *dst, RENDER_BUFFER *src_ch0,
                                  RENDER_BUFFER *src_ch1, RENDER_BUFFER *opacity) {
    RGB_PIXEL r0, r1, g0, g1, b0, b1;
    for (INT offs = 0; offs < opacity->height*opacity->width; offs++) {
        b0 = src_ch0->p[offs]&0x000000FF;
        b1 = src_ch1->p[offs]&0x000000FF;
        g0 = src_ch0->p[offs]&0x0000FF00;
        g1 = src_ch1->p[offs]&0x0000FF00;
        r0 = src_ch0->p[offs]&0x00FF0000;
        r1 = src_ch1->p[offs]&0x00FF0000;
        dst->p[offs] = ((b1*opacity->p[offs] + b0*(256-opacity->p[offs])) |
                       ((g1*opacity->p[offs] + g0*(256-opacity->p[offs]))& 0x00FF0000) |
                       ((r1*opacity->p[offs] + r0*(256-opacity->p[offs]))& 0xFF000000)) >> 8;
    }
}

/**
 * Calculates a + b with saturation for rgb parts of a and b. Result stored in a.
 * Addition starts at position (x, y) in a
 * Buffers have to have same dimensions.
 */
void render_buffer_add_saturation(RENDER_BUFFER *a, RENDER_BUFFER *b) {
    INT o; /** Offset for traversing buffers*/
    RGB_PIXEL s;
    if (a->width != b->width || a->height != b->height) {
        return;
    }
    for (o = 0; o < a->width * a->height; o++) {
        /** Add all components */
        s = (a->p[o]&0x00FEFEFF) + (b->p[o]&0x00FEFEFF);
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
        ((RGB_PIXEL*)(a->p))[o] = s;
    }
}