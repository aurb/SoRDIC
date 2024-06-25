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

void ARGB_MAP_fade_dither_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                            FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        out->data[offs] = r < blend ? in->data[offs] : (r == 255 && blend == 255 ? in->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_global(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                         FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT pfa = p*255.99;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pixval = ((ARGB_PIXEL*)in->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_global(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                             FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT blend = p*255.99;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        out->data[offs] = (r < blend ? in1 : (r == 255 && blend == 255 ? in1 : in0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_global(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                          FLOAT p) {
    p = p < 0.0 ? 0.0 : (p > 1.0 ? 1.0 : p);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    INT pfa = p*255.99;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pixval = ((ARGB_PIXEL*)in0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)in1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_fade_dither_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                                ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color), a;
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        r = LIMIT_255(PRN(seed));
        a = p->data[offs]>>24;
        out->data[offs] = r < a ? in->data[offs] : (r == 255 && a == 255 ? in->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                             ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa;
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] );
        pixval = ((ARGB_PIXEL*)in->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                                 ARGB_MAP *p) {
    ARGB_PIXEL a;
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        a = p->data[offs]>>24;
        r = LIMIT_255(PRN(seed));
        out->data[offs] = (r < a ? in1 : (r == 255 && a == 255 ? in1 : in0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                              ARGB_MAP *p) {
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pfa, pixval;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] );
        pixval = ((ARGB_PIXEL*)in0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)in1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_fade_dither_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                                  FLOAT f, ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    INT seed = (engine_run_stats().frames%7) * 1000;
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*(p->data[offs]>>24);
        r = LIMIT_65535(PRN(seed));
        out->data[offs] = r < pfa ? in->data[offs] : (r == 65535 && pfa == 65535 ? in->data[offs] : pixval);
    }
}

void ARGB_MAP_fade_mul_f_per_pixel(ARGB_MAP *out, COLOR* color, ARGB_MAP *in,
                               FLOAT f, ARGB_MAP *p) {
    ARGB_PIXEL pixval = COLOR_to_ARGB_PIXEL(color);
    ARGB_PIXEL r0, g0, b0, r1, g1, b1;
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    r0 = ARGB_PIXEL_RED(pixval);
    g0 = ARGB_PIXEL_GREEN(pixval);
    b0 = ARGB_PIXEL_BLUE(pixval);
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] ) >> 8;
        pixval = ((ARGB_PIXEL*)in->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

void ARGB_MAP_blend_dither_f_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                                   FLOAT f, ARGB_MAP *p) {
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    //INT seed = engine_run_stats().frames; //better at full framerate
    INT seed = (engine_run_stats().frames%7) * 1000; //better with lower framerates
    INT r = 0;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*(p->data[offs]>>24);
        r = LIMIT_65535(PRN(seed));
        out->data[offs] = (r < pfa ? in1 : (r == 65535 && pfa == 65535 ? in1 : in0))->data[offs];
    }
}

void ARGB_MAP_blend_mul_f_per_pixel(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1,
                                FLOAT f, ARGB_MAP *p) {
    INT ff = f*257.99; //fixed-point f
    INT pfa = 0; //f*p[pixel]
    ARGB_PIXEL r0, g0, b0, r1, g1, b1, pixval;
    for (INT offs = 0; offs < out->height*out->width; offs++) {
        pfa = ff*ARGB_PIXEL_ALPHA( ((ARGB_PIXEL*)p->data)[offs] ) >> 8;
        pixval = ((ARGB_PIXEL*)in0->data)[offs];
        r0 = ARGB_PIXEL_RED(pixval);
        g0 = ARGB_PIXEL_GREEN(pixval);
        b0 = ARGB_PIXEL_BLUE(pixval);
        pixval = ((ARGB_PIXEL*)in1->data)[offs];
        r1 = ARGB_PIXEL_RED(pixval);
        g1 = ARGB_PIXEL_GREEN(pixval);
        b1 = ARGB_PIXEL_BLUE(pixval);
        out->data[offs] = (((r1*pfa + r0*(255-pfa)) >> 8) << R_SHIFT) |
                          (((g1*pfa + g0*(255-pfa)) >> 8) << G_SHIFT) |
                          (((b1*pfa + b0*(255-pfa)) >> 8) << B_SHIFT);
    }
}

/**
 * Calculates a + b with saturation for map parts of a and b. Result stored in a.
 * Addition starts at position (x, y) in a
 * Buffers have to have same dimensions.
 */
void ARGB_MAP_sat_add(ARGB_MAP *out, ARGB_MAP *in0, ARGB_MAP *in1) {
    INT offs; /** Offset for traversing buffers*/
    ARGB_PIXEL sum;
    if (in0->width != in1->width || in0->height != in1->height) {
        return;
    }
    for (offs = 0; offs < in0->width * in0->height; offs++) {
        /** Add all components */
        sum = (in0->data[offs]&0x00FEFEFF) + (in1->data[offs]&0x00FEFEFF);
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
        ((ARGB_PIXEL*)(out->data))[offs] = sum;
    }
}
