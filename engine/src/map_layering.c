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

void ARGB_MAP_multiplex(ARGB_MAP *out, ARGB_MAP **in, ARGB_MAP *mask) {
    for (INT offs = 0; offs < mask->height * mask->width; offs++)
        out->data[offs] = in[mask->data[offs]]->data[offs];
}

#define PREAMBLE_OUT_IN                                                                                   \
    if (out_x >= out->width || out_y >= out->height || out_x+in->width <= 0 || out_y+in->height <= 0) \
        return;                                                                                       \
    ARGB_PIXEL *out_data = out->data;                                                                 \
    ARGB_PIXEL *in_data = in->data;                                                                   \
    INT t_w = in->width < out->width ? in->width : out->width;                                        \
    INT t_h = in->height < out->height ? in->height : out->height;                                    \
    INT x = 0, y = 0;                                                                                 \
    if (out_x < 0) { t_w += out_x; in_data += -out_x; }                                               \
    if (out_y < 0) { t_h += out_y; in_data += -out_y*in->width; }                                     \
    if (out_x > 0) { out_data += out_x;                                                               \
        if (out_x + t_w > out->width) t_w = out->width - out_x; }                                     \
    if (out_y > 0) { out_data += out_y*out->width;                                                    \
        if (out_y + t_h > out->height) t_h = out->height - out_y; }

#define PREAMBLE_OUT_IN0_IN1                                                                                \
    if (in0->width != in1->width || in0->height != in1->height)                                         \
        return;                                                                                         \
    if (out_x >= out->width || out_y >= out->height || out_x+in0->width <= 0 || out_y+in0->height <= 0) \
        return;                                                                                         \
    ARGB_PIXEL *out_data = out->data;                                                                   \
    ARGB_PIXEL *in0_data = in0->data;                                                                   \
    ARGB_PIXEL *in1_data = in1->data;                                                                   \
    INT t_w = in0->width < out->width ? in0->width : out->width;                                        \
    INT t_h = in0->height < out->height ? in0->height : out->height;                                    \
    INT x = 0, y = 0;                                                                                   \
    if (out_x < 0) { t_w += out_x; in1_data += -out_x; }                                                \
    if (out_y < 0) { t_h += out_y; in1_data += -out_y*in1->width; }                                     \
    if (out_x > 0) { out_data += out_x;    in0_data += out_x;                                           \
        if (out_x + t_w > out->width) t_w = out->width - out_x; }                                       \
    if (out_y > 0) { out_data += out_y*out->width;    in0_data += out_y*in0->width;                     \
        if (out_y + t_h > out->height) t_h = out->height - out_y; }

//TODO: PREAMBLE_P does not verify the equality of dimensions between p and in/in0/in1
#define PREAMBLE_P                            \
    ARGB_PIXEL *p_data = p->data;             \
    if (out_x < 0) p_data += -out_x;          \
    if (out_y < 0) p_data += -out_y*p->width;

#define LOOP_Y \
    for (y = 0; y < t_h; y++)
#define LOOP_X \
    for (x = 0; x < t_w; x++)

#define NEXT_Y_OUT out_data += out->width;
#define NEXT_Y_IN in_data += in->width;
#define NEXT_Y_IN0 in0_data += in0->width;
#define NEXT_Y_IN1 in1_data += in1->width;
#define NEXT_Y_P p_data += p->width;

void ARGB_MAP_fade_dither_global(ARGB_MAP *out, INT out_x, INT out_y,
                                 COLOR* color, ARGB_MAP *in,
                                 FLOAT p) {
    PREAMBLE_OUT_IN;
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;

    LOOP_Y {
        LOOP_X {
            r = LIMIT_255(PRN(seed));
            out_data[x] = r < blend ? in_data[x] : (r == 255 && blend == 255 ? in_data[x] : pixval);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
    }
}

void ARGB_MAP_fade_mul_global(ARGB_MAP *out, INT out_x, INT out_y,
                              COLOR* color, ARGB_MAP *in,
                              FLOAT p) {
    PREAMBLE_OUT_IN;
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT pfa = p*255.99;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    LOOP_Y {
        LOOP_X {
            pixval = in_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
    }
}

void ARGB_MAP_blend_dither_global(ARGB_MAP *out, INT out_x, INT out_y,
                                  ARGB_MAP *in0, ARGB_MAP *in1,
                                  FLOAT p) {
    PREAMBLE_OUT_IN0_IN1;
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;
    LOOP_Y {
        LOOP_X {
            r = LIMIT_255(PRN(seed));
            out_data[x] = (r < blend ? in1_data : (r == 255 && blend == 255 ? in1_data : in0_data))[x];
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
    }
}

void ARGB_MAP_blend_mul_global(ARGB_MAP *out, INT out_x, INT out_y,
                               ARGB_MAP *in0, ARGB_MAP *in1,
                               FLOAT p) {
    PREAMBLE_OUT_IN0_IN1;
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    INT pfa = p*255.99;
    LOOP_Y {
        LOOP_X {
            pixval = in0_data[x];
            r0 = ARGB_PIXEL_RED(pixval);
            g0 = ARGB_PIXEL_GREEN(pixval);
            b0 = ARGB_PIXEL_BLUE(pixval);
            pixval = in1_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                            (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                            (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
    }
}

void ARGB_MAP_fade_dither_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                    COLOR* color, ARGB_MAP *in,
                                    ARGB_MAP *p) {
    PREAMBLE_OUT_IN;
    PREAMBLE_P;
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color), a;
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    LOOP_Y {
        LOOP_X {
            r = LIMIT_255(PRN(seed));
            a = p_data[x]>>24;
            out_data[x] = r < a ? in_data[x] : (r == 255 && a == 255 ? in_data[x] : pixval);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
        NEXT_Y_P;
    }
}

void ARGB_MAP_fade_mul_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                 COLOR* color, ARGB_MAP *in,
                                 ARGB_MAP *p) {
    PREAMBLE_OUT_IN;
    PREAMBLE_P;
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    LOOP_Y {
        LOOP_X {
            pfa = ARGB_PIXEL_ALPHA(p_data[x]);
            pixval = in_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                        (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                        (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
        NEXT_Y_P;
    }
}

void ARGB_MAP_blend_dither_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                     ARGB_MAP *in0, ARGB_MAP *in1,
                                     ARGB_MAP *p) {
    PREAMBLE_OUT_IN0_IN1;
    PREAMBLE_P;
    ARGB_PIXEL a;
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    LOOP_Y {
        LOOP_X {
            a = p_data[x]>>24;
            r = LIMIT_255(PRN(seed));
            out_data[x] = (r < a ? in1_data : (r == 255 && a == 255 ? in1_data : in0_data))[x];
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
        NEXT_Y_P;
    }
}

void ARGB_MAP_blend_mul_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                  ARGB_MAP *in0, ARGB_MAP *in1,
                                  ARGB_MAP *p) {
    PREAMBLE_OUT_IN0_IN1;
    PREAMBLE_P;
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa, pixval;
    LOOP_Y {
        LOOP_X {
            pfa = ARGB_PIXEL_ALPHA( p_data[x] );
            pixval = in0_data[x];
            r0 = ARGB_PIXEL_RED(pixval);
            g0 = ARGB_PIXEL_GREEN(pixval);
            b0 = ARGB_PIXEL_BLUE(pixval);
            pixval = in1_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
        NEXT_Y_P;
    }
}

void ARGB_MAP_fade_dither_f_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                      COLOR* color, ARGB_MAP *in,
                                      FLOAT f, ARGB_MAP *p) {
    PREAMBLE_OUT_IN;
    PREAMBLE_P;
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    LOOP_Y {
        LOOP_X {
            pfa = ff*(p_data[x]>>24);
            r = LIMIT_65535(PRN(seed));
            out_data[x] = r < pfa ? in_data[x] : (r == 65535 && pfa == 65535 ? in_data[x] : pixval);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
        NEXT_Y_P;
    }
}

void ARGB_MAP_fade_mul_f_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                   COLOR* color, ARGB_MAP *in,
                                   FLOAT f, ARGB_MAP *p) {
    PREAMBLE_OUT_IN;
    PREAMBLE_P;
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    LOOP_Y {
        LOOP_X {
            pfa = ff*ARGB_PIXEL_ALPHA( p_data[x] ) >> 8;
            pixval = in_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN;
        NEXT_Y_P;
    }
}

void ARGB_MAP_blend_dither_f_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                       ARGB_MAP *in0, ARGB_MAP *in1,
                                       FLOAT f, ARGB_MAP *p) {
    PREAMBLE_OUT_IN0_IN1;
    PREAMBLE_P;
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    LOOP_Y {
        LOOP_X {
            pfa = ff*(p_data[x]>>24);
            r = LIMIT_65535(PRN(seed));
            out_data[x] = (r < pfa ? in1_data : (r == 65535 && pfa == 65535 ? in1_data : in0_data))[x];
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
        NEXT_Y_P;
    }
}

void ARGB_MAP_blend_mul_f_per_pixel(ARGB_MAP *out, INT out_x, INT out_y,
                                    ARGB_MAP *in0, ARGB_MAP *in1,
                                    FLOAT f, ARGB_MAP *p) {
    PREAMBLE_OUT_IN0_IN1;
    PREAMBLE_P;
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    LOOP_Y {
        LOOP_X {
            pfa = ff*ARGB_PIXEL_ALPHA( p_data[x] ) >> 8;
            pixval = in0_data[x];
            r0 = ARGB_PIXEL_RED(pixval);
            g0 = ARGB_PIXEL_GREEN(pixval);
            b0 = ARGB_PIXEL_BLUE(pixval);
            pixval = in1_data[x];
            r1 = ARGB_PIXEL_RED(pixval);
            g1 = ARGB_PIXEL_GREEN(pixval);
            b1 = ARGB_PIXEL_BLUE(pixval);
            out_data[x] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                            (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                            (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
        NEXT_Y_P;
    }
}

/**
 * Calculates a + b with saturation for map parts of a and b. Result stored in a.
 * Addition starts at position (x, y) in a
 * Buffers have to have same dimensions.
 */
void ARGB_MAP_sat_add(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *in0, ARGB_MAP *in1) {
    PREAMBLE_OUT_IN0_IN1;
    ARGB_PIXEL sum;
    if (in0->width != in1->width || in0->height != in1->height) {
        return;
    }
    LOOP_Y {
        LOOP_X {
            /** Add all components */
            sum = (in0_data[x]&0x00FEFEFF) + (in1_data[x]&0x00FEFEFF);
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
            out_data[x] = sum;
        }
        NEXT_Y_OUT;
        NEXT_Y_IN0;
        NEXT_Y_IN1;
    }
}
