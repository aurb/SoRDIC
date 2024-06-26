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

INT *map_filter_buffer = NULL;
DISCRETE_GRADIENT *map_filter_dg = NULL;

void map_filters_init() {
    map_filter_buffer = calloc(100000, sizeof(INT));
    map_filter_dg = DISCRETE_GRADIENT_alloc(1024);
}

void map_filters_cleanup() {
    free(map_filter_buffer);
    DISCRETE_GRADIENT_free(map_filter_dg);
}

void ARGB_MAP_green_gradient_global_copy(ARGB_MAP *out, ARGB_MAP *in, GRADIENT *g, const INT p) {
    INT x=0, y=0, offs=0;
    INT l00, l01, l10, dl;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);
    if (out->height != in->height || out->width != in->width)
        return;

    for(y=0, offs=0; y < in->height-p; y++) {
        for(x=0; x < in->width-p; x++, offs++) {
            l00 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs]);
            l10 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+p]);
            l01 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+p*in->width]);

            dl = l10 + l01 - 2*l00;
            if (dl < 0) dl = -dl;
            ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[dl];
        }
        for(; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[0];
        }
    }
    for(; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[0];
        }
    }
}

void ARGB_MAP_green_gradient_global_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *in, GRADIENT *g, const INT p) {
    INT x=0, y=0, offs=0;
    INT l00, l01, l10, dl;
    ARGB_PIXEL Ae, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);
    if (out->height != bg->height || out->width != bg->width || out->height != in->height || out->width != in->width) {
        return;
    }

    for(y=0, offs=0; y < in->height-p; y++) {
        for(x=0; x < in->width-p; x++, offs++) {
            l00 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs]);
            l10 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+p]);
            l01 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+p*in->width]);

            dl = l10 + l01 - 2*l00; //TODO: should the discrete gradient used be 512 elements long?
            if (dl < 0) dl = -dl;

            pixval = map_filter_dg->pixval[dl];
            Ae = ARGB_PIXEL_ALPHA(pixval);
            if (Ae > 0) {
                Rf = (ARGB_PIXEL_RED(pixval)*Ae >> 8) << R_SHIFT;
                Gf = (ARGB_PIXEL_GREEN(pixval)*Ae >> 8) << G_SHIFT;
                Bf = (ARGB_PIXEL_BLUE(pixval)*Ae >> 8) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Ae) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Ae) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Ae) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
        for(; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
        }
    }
    for(; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
        }
    }
}

void ARGB_MAP_green_gradient_per_pixel_copy(ARGB_MAP *out, ARGB_MAP *in, GRADIENT *g, ARGB_MAP *p) {
    INT x=0, y=0, offs=0;
    INT l00, l01, l10, dl, da;
    INT in_buf_size = in->height*in->width;
    INT pa;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);
    if (out->height != in->height || out->width != in->width)
        return;

    for(y=0, offs=0; y < in->height-MAX_EDGE_WIDTH; y++) {
        for(x=0; x < in->width-MAX_EDGE_WIDTH; x++, offs++) {
            pa = ARGB_PIXEL_ALPHA(((ARGB_PIXEL*)p->data)[offs]);
            da = pa >> 4; //max edge thickness (for pa==255) is MAX_EDGE_WIDTH
            if (offs+da*in->width < in_buf_size) {
                l00 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs]);
                l10 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+da]);
                l01 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+da*in->width]);

                dl = l10 + l01 - 2*l00;
                if (dl < 0) dl = -dl;
                ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[dl*pa >> 8];
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[0];
            }
        }
        for(; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[0];
        }
    }
    for(; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[0];
        }
    }
}

void ARGB_MAP_green_gradient_per_pixel_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *in, GRADIENT *g, ARGB_MAP *p) {
    INT x=0, y=0, offs=0;
    INT l00, l01, l10, dl, da;
    INT in_buf_size = in->height*in->width;
    INT pa;
    ARGB_PIXEL Ae, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);
    if (out->height != in->height || out->width != in->width)
        return;

    for(y=0, offs=0; y < in->height-MAX_EDGE_WIDTH; y++) {
        for(x=0; x < in->width-MAX_EDGE_WIDTH; x++, offs++) {
            pa = ARGB_PIXEL_ALPHA(((ARGB_PIXEL*)p->data)[offs]);
            da = pa >> 4; //max edge thickness (for pa==255) is MAX_EDGE_WIDTH
            if (offs+da*in->width < in_buf_size) {
                l00 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs]);
                l10 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+da]);
                l01 = ARGB_PIXEL_GREEN(((ARGB_PIXEL*)in->data)[offs+da*in->width]);

                dl = l10 + l01 - 2*l00;
                if (dl < 0) dl = -dl;
                //((ARGB_PIXEL*)out->data)[offs] = map_filter_dg->pixval[dl*pa >> 8];
                pixval = map_filter_dg->pixval[dl*pa >> 8];
                Ae = ARGB_PIXEL_ALPHA(pixval);
                if (Ae > 0) {
                    Rf = (ARGB_PIXEL_RED(pixval)*Ae >> 8) << R_SHIFT;
                    Gf = (ARGB_PIXEL_GREEN(pixval)*Ae >> 8) << G_SHIFT;
                    Bf = (ARGB_PIXEL_BLUE(pixval)*Ae >> 8) << B_SHIFT;
                    pixval = ((ARGB_PIXEL*)bg->data)[offs];
                    Rb = (ARGB_PIXEL_RED(pixval)*(255-Ae) >> 8) << R_SHIFT;
                    Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Ae) >> 8) << G_SHIFT;
                    Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Ae) >> 8) << B_SHIFT;
                    ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
                }
                else {
                    ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
                }
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
        for(; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
        }
    }
    for(; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
        }
    }
}

void ARGB_MAP_blur_nx1_global_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p) {
    INT x = 0, y = 0, offs = 0;
    ARGB_PIXEL pixval;
    ARGB_PIXEL r, g, b, rb, gb, bb, mul_f = (1<<FRACT_SHIFT)/p;
    const INT lp = (p-1)/2; //left half of p
    const INT rp = p/2; //right half of p

    if (p < 1 || out->height != in->height || out->width != in->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_copy(out, in);
        return;
    }

    for(y = 0, offs = 0; y < in->height; y++) {
        r = g = b = 0;
        for(x = 0; x < rp; x++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+x];
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
        }
        for(x = 0; x < lp+1; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+rp];
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            rb = (r*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
        for(x = lp+1; x < in->width-rp; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+rp];
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            pixval = ((ARGB_PIXEL*)in->data)[offs-(lp+1)];
            r -= ARGB_PIXEL_RED(pixval);
            g -= ARGB_PIXEL_GREEN(pixval);
            b -= ARGB_PIXEL_BLUE(pixval);
            rb = (r*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
        for(x = in->width-rp; x < in->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs-(lp+1)];
            r -= ARGB_PIXEL_RED(pixval);
            g -= ARGB_PIXEL_GREEN(pixval);
            b -= ARGB_PIXEL_BLUE(pixval);
            rb = (r*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
    }
}

void ARGB_MAP_blur_nx1_global_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p) {
    INT x = 0, y = 0, offs = 0;
    ARGB_PIXEL pixval;
    ARGB_PIXEL a, r, g, b, Aavg, Rf, Gf, Bf, Rb, Gb, Bb, mul_f[256];
    ARGB_PIXEL mfa; //multiplication factor for alpha channel (inverse of blur distance)
    ARGB_PIXEL mfrgb; //multiplication factor for rgb channels (inverse of alpha total)
    const INT lp = (p-1)/2; //left half of p
    const INT rp = p/2; //right half of p

    if (out->height != bg->height || out->width != bg->width || out->height != fg->height || out->width != fg->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_blend_mul_global(out, 0, 0, fg, 1.0);
        return;
    }
    mul_f[0] = 1<<FRACT_SHIFT;
    for (x = 1; x < 256; x++) {
        mul_f[x] = (1<<FRACT_SHIFT)/x;
    }
    mfa = mul_f[p];

    for(y = 0, offs = 0; y < fg->height; y++) {
        a = r = g = b = 0;
        for(x = 0; x < rp; x++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+x];
            a += ARGB_PIXEL_ALPHA(pixval);
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
        }
        for(x = 0; x < lp+1; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+rp];
            a += ARGB_PIXEL_ALPHA(pixval);
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            Aavg = a*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a >> 8) + 1];
                Rf = (r*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
        for(x = lp+1; x < fg->width-rp; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+rp];
            a += ARGB_PIXEL_ALPHA(pixval);
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            pixval = ((ARGB_PIXEL*)fg->data)[offs-(lp+1)];
            a -= ARGB_PIXEL_ALPHA(pixval);
            r -= ARGB_PIXEL_RED(pixval);
            g -= ARGB_PIXEL_GREEN(pixval);
            b -= ARGB_PIXEL_BLUE(pixval);
            Aavg = a*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a >> 8) + 1];
                Rf = (r*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
        for(x = fg->width-rp; x < fg->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs-(lp+1)];
            a -= ARGB_PIXEL_ALPHA(pixval);
            r -= ARGB_PIXEL_RED(pixval);
            g -= ARGB_PIXEL_GREEN(pixval);
            b -= ARGB_PIXEL_BLUE(pixval);
            Aavg = a*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a >> 8) + 1];
                Rf = (r*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
    }
}

void ARGB_MAP_blur_nx1_per_pixel_copy(ARGB_MAP *out, ARGB_MAP *in, ARGB_MAP *p) {
    INT x = 0, y = 0, offs = 0, pa, lx, rx;
    ARGB_PIXEL pixval, r, g, b, mul_f[256], mf;

    UINT *r_buf = (UINT*)map_filter_buffer;
    UINT *g_buf = (UINT*)map_filter_buffer + in->width;
    UINT *b_buf = (UINT*)map_filter_buffer + 2*in->width;

    if (out->height != in->height || out->width != in->width) {
        return;
    }

    mul_f[0] = 1<<FRACT_SHIFT;
    for (x = 1; x < 256; x++) {
        mul_f[x] = (1<<FRACT_SHIFT)/x;
    }

    for(y = 0, offs = 0; y < in->height; y++) {
        pixval = ((ARGB_PIXEL*)in->data)[offs];
        r_buf[0] = ARGB_PIXEL_RED(pixval);
        g_buf[0] = ARGB_PIXEL_GREEN(pixval);
        b_buf[0] = ARGB_PIXEL_BLUE(pixval);
        for(x = 1; x < in->width; x++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+x];
            r_buf[x] = r_buf[x-1] + ARGB_PIXEL_RED(pixval);
            g_buf[x] = g_buf[x-1] + ARGB_PIXEL_GREEN(pixval);
            b_buf[x] = b_buf[x-1] + ARGB_PIXEL_BLUE(pixval);
        }

        for(x = 0; x < in->width; x++, offs++) {
            pa = ARGB_PIXEL_ALPHA(p->data[offs]) + 1;
            if (pa > 1) {
                lx = x - (pa-1)/2 - 1;
                if (lx < 0)
                    lx = 0;
                rx = x + pa/2;
                if (rx > in->width-1)
                    rx = in->width-1;
                mf = mul_f[rx - lx];
                r = ((r_buf[rx]-r_buf[lx])*mf >> FRACT_SHIFT) << R_SHIFT;
                g = ((g_buf[rx]-g_buf[lx])*mf >> FRACT_SHIFT) << G_SHIFT;
                b = ((b_buf[rx]-b_buf[lx])*mf >> FRACT_SHIFT) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = r | g | b;
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)in->data)[offs];
            }
        }
    }
}

void ARGB_MAP_blur_nx1_per_pixel_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, ARGB_MAP *p) {
    //function assumes that objects to be blured are rendered on the black background
    //also alpha channel in ARGB_MAP *in is either 0x00 or 0xFF
    INT x = 0, y = 0, offs = 0, pa, lx, rx;
    ARGB_PIXEL pixval, mul_f[256];
    ARGB_PIXEL Aavg; //Average alpha component
    ARGB_PIXEL Rf, Gf, Bf; //Average foreground color components
    ARGB_PIXEL Rb, Gb, Bb; //Average background color components
    ARGB_PIXEL mfa; //multiplication factor for alpha channel (inverse of blur distance)
    ARGB_PIXEL mfrgb; //multiplication factor for rgb channels (inverse of alpha total)

    //Integration buffers for color components
    UINT *a_buf = (UINT*)map_filter_buffer;
    UINT *r_buf = (UINT*)map_filter_buffer + fg->width;
    UINT *g_buf = (UINT*)map_filter_buffer + 2*fg->width;
    UINT *b_buf = (UINT*)map_filter_buffer + 3*fg->width;

    if (out->height != bg->height || out->width != bg->width || out->height != fg->height || out->width != fg->width) {
        return;
    }

    mul_f[0] = 1<<FRACT_SHIFT;
    for (x = 1; x < 256; x++) {
        mul_f[x] = (1<<FRACT_SHIFT)/x;
    }

    for(y = 0, offs = 0; y < fg->height; y++) {
        pixval = ((ARGB_PIXEL*)fg->data)[offs];
        a_buf[0] = ARGB_PIXEL_ALPHA(pixval);
        r_buf[0] = ARGB_PIXEL_RED(pixval);
        g_buf[0] = ARGB_PIXEL_GREEN(pixval);
        b_buf[0] = ARGB_PIXEL_BLUE(pixval);
        for(x = 1; x < fg->width; x++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+x];
            a_buf[x] = a_buf[x-1] + ARGB_PIXEL_ALPHA(pixval);
            r_buf[x] = r_buf[x-1] + ARGB_PIXEL_RED(pixval);
            g_buf[x] = g_buf[x-1] + ARGB_PIXEL_GREEN(pixval);
            b_buf[x] = b_buf[x-1] + ARGB_PIXEL_BLUE(pixval);
        }

        for(x = 0; x < fg->width; x++, offs++) {
            //blur distance for this pixel
            pa = ARGB_PIXEL_ALPHA(p->data[offs]) + 1;
            lx = x - (pa-1)/2 - 1;
            if (lx < 0)
                lx = 0;
            rx = x + pa/2;
            if (rx > fg->width-1)
                rx = fg->width-1;
            mfa = mul_f[rx - lx]; //inverse of blur distance
            //Component average values
            Aavg = (a_buf[rx]-a_buf[lx])*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                mfrgb = mul_f[((a_buf[rx]-a_buf[lx]) >> 8) + 1];
                Rf = ((r_buf[rx]-r_buf[lx])*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = ((g_buf[rx]-g_buf[lx])*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = ((b_buf[rx]-b_buf[lx])*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
    }
}

void ARGB_MAP_blur_1xn_global_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p) {
    INT x = 0, y = 0, offs = 0;
    ARGB_PIXEL pixval;
    INT *r = map_filter_buffer;
    INT *g = map_filter_buffer + in->width;
    INT *b = map_filter_buffer + 2*in->width;
    INT rb, gb, bb, mul_f = (1<<FRACT_SHIFT)/p;

    const INT tp = (p-1)/2;
    const INT bp = p/2;

    if (p < 1 || out->height != in->height || out->width != in->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_copy(out, in);
        return;
    }

    for (x = 0; x < in->width; x++) {
        r[x] = g[x] = b[x] = 0;
    }

    for(y = 0, offs = 0; y < bp; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
        }
    }

    for(y = 0, offs = 0; y < tp+1; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+bp*in->width];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
    }

    for(y = tp+1; y < in->height-bp; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs+bp*in->width];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            pixval = ((ARGB_PIXEL*)in->data)[offs-(tp+1)*in->width];
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
    }

    for(y = in->height-bp; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)in->data)[offs-(tp+1)*in->width];
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            ((ARGB_PIXEL*)out->data)[offs] = rb | gb | bb;
        }
    }
}

void ARGB_MAP_blur_1xn_global_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p) {
    INT x = 0, y = 0, offs = 0;
    ARGB_PIXEL pixval;
    ARGB_PIXEL Aavg, Rf, Gf, Bf, Rb, Gb, Bb, mul_f[256];
    ARGB_PIXEL *a = (ARGB_PIXEL*)map_filter_buffer;
    ARGB_PIXEL *r = (ARGB_PIXEL*)map_filter_buffer + fg->width;
    ARGB_PIXEL *g = (ARGB_PIXEL*)map_filter_buffer + 2*fg->width;
    ARGB_PIXEL *b = (ARGB_PIXEL*)map_filter_buffer + 3*fg->width;
    ARGB_PIXEL mfa; //multiplication factor for alpha channel (inverse of blur distance)
    ARGB_PIXEL mfrgb; //multiplication factor for rgb channels (inverse of alpha total)

    const INT tp = (p-1)/2;
    const INT bp = p/2;

    if (out->height != bg->height || out->width != bg->width || out->height != fg->height || out->width != fg->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_blend_mul_global(out, 0, 0, fg, 1.0);
        return;
    }
    mul_f[0] = 1<<FRACT_SHIFT;
    for (x = 1; x < 256; x++) {
        mul_f[x] = (1<<FRACT_SHIFT)/x;
    }
    mfa = mul_f[p];

    for (x = 0; x < fg->width; x++) {
        a[x] = r[x] = g[x] = b[x] = 0;
    }

    for(y = 0, offs = 0; y < bp; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs];
            a[x] += ARGB_PIXEL_ALPHA(pixval);
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
        }
    }

    for(y = 0, offs = 0; y < tp+1; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+bp*fg->width];
            a[x] += ARGB_PIXEL_ALPHA(pixval);
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            Aavg = a[x]*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a[x] >> 8) + 1];
                Rf = (r[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
    }

    for(y = tp+1; y < fg->height-bp; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs+bp*fg->width];
            a[x] += ARGB_PIXEL_ALPHA(pixval);
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            pixval = ((ARGB_PIXEL*)fg->data)[offs-(tp+1)*fg->width];
            a[x] -= ARGB_PIXEL_ALPHA(pixval);
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            Aavg = a[x]*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a[x] >> 8) + 1];
                Rf = (r[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
    }

    for(y = fg->height-bp; y < fg->height; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = ((ARGB_PIXEL*)fg->data)[offs-(tp+1)*fg->width];
            a[x] -= ARGB_PIXEL_ALPHA(pixval);
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            Aavg = a[x]*mfa >> FRACT_SHIFT;
            if (Aavg > 0) {
                //inverse of total alpha along blur section
                //This corresponds to number of "on" pixels in foreground blur sample
                mfrgb = mul_f[(a[x] >> 8) + 1];
                Rf = (r[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << R_SHIFT;
                Gf = (g[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << G_SHIFT;
                Bf = (b[x]*mfrgb*Aavg >> (FRACT_SHIFT+8)) << B_SHIFT;
                pixval = ((ARGB_PIXEL*)bg->data)[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
            }
        }
    }
}

void ARGB_MAP_pixelize_copy(ARGB_MAP *out, ARGB_MAP *in, const INT p) {
    INT x = 0, y = 0, i = 0, offs = 0;
    ARGB_PIXEL *line_buf = (ARGB_PIXEL*)map_filter_buffer;

    if (p < 1 || out->height != in->height || out->width != in->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_copy(out, in);
        return;
    }

    for (y = 0, offs = 0; y < in->height; y+=p) {
        for (x = 0; x < in->width; x+=p) {
            for (i = 0; i < p && x+i < in->width; i++) {
                line_buf[x+i] = ((ARGB_PIXEL*)in->data)[offs+x];
            }
        }
        for (i = 0; i < p; i++, offs+=in->width) {
            memcpy((void*)(out->data+offs), (void*)line_buf, in->width*sizeof(ARGB_PIXEL));
        }
    }
}

void ARGB_MAP_pixelize_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg, const INT p) {
    INT x = 0, y = 0, i = 0, offs = 0;
    ARGB_PIXEL *line_buf = (ARGB_PIXEL*)map_filter_buffer;
    ARGB_PIXEL Af, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    if (p < 1 || out->height != fg->height || out->width != fg->width) {
        return;
    }
    else if (p == 1) {
        ARGB_MAP_blend_mul_global(out, 0, 0, fg, 1.0);
        return;
    }

    for (y = 0, offs = 0; y < fg->height; y+=p) {
        for (x = 0; x < fg->width; x+=p) {
            for (i = 0; i < p && x+i < fg->width; i++) {
                line_buf[x+i] = ((ARGB_PIXEL*)fg->data)[offs+x];
            }
        }
        for (i = 0; i < p; i++) {
            for (x = 0; x < fg->width; x++, offs++) {
                pixval = line_buf[x];
                Af = ARGB_PIXEL_ALPHA(pixval);
                if (Af > 0) {
                    Rf = (ARGB_PIXEL_RED(pixval)*Af >> 8) << R_SHIFT;
                    Gf = (ARGB_PIXEL_GREEN(pixval)*Af >> 8) << G_SHIFT;
                    Bf = (ARGB_PIXEL_BLUE(pixval)*Af >> 8) << B_SHIFT;
                    pixval = ((ARGB_PIXEL*)bg->data)[offs];
                    Rb = (ARGB_PIXEL_RED(pixval)*(255-Af) >> 8) << R_SHIFT;
                    Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Af) >> 8) << G_SHIFT;
                    Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Af) >> 8) << B_SHIFT;
                    ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
                }
                else {
                    ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
                }
            }
        }
    }
}

void ARGB_MAP_rand_pixelize_copy(ARGB_MAP *out, ARGB_MAP *in,
                                const INT l_min, const INT l_max, UINT seed_l,
                                const INT h_min, const INT h_max, UINT seed_h) {
    INT x = 0, y = 0, i = 0, offs = 0, l = 0, h = 0;
    ARGB_PIXEL *line_buf = (ARGB_PIXEL*)map_filter_buffer;

    if (l_min < 1 || l_max < l_min || h_min < 1 || h_max < h_min ||
        out->height != in->height || out->width != in->width) {
        return;
    }
    else if (l_min == 1 && l_max == 1 && h_min == 1 && h_max == 1) {
        ARGB_MAP_copy(out, in);
        return;
    }

    for (y = 0, h = 0, offs = 0; y < in->height; y += h) {
        for (x = 0, l = 0; x < in->width; x += l) {
            if (l_max == l_min)
                l = l_min;
            else
                l = l_min + PRN(seed_l)%(l_max-l_min);
            if (x+l > in->width)
                l = in->width - x;
            for (i = 0; i < l; i++) {
                line_buf[x+i] = ((ARGB_PIXEL*)in->data)[offs+x];
            }
        }

        if (h_max == h_min)
            h = h_min;
        else
            h = h_min + PRN(seed_h)%(h_max-h_min);
        if (y+h > in->height)
            h = in->height - y;
        for (i = 0; i < h; i++, offs+=in->width) {
            memcpy((void*)(out->data+offs), (void*)line_buf, in->width*sizeof(ARGB_PIXEL));
        }
    }
}

void ARGB_MAP_rand_pixelize_blend(ARGB_MAP *out, ARGB_MAP *bg, ARGB_MAP *fg,
                                  const INT l_min, const INT l_max, UINT seed_l,
                                  const INT h_min, const INT h_max, UINT seed_h) {
    INT x = 0, y = 0, i = 0, offs = 0, l = 0, h = 0;
    ARGB_PIXEL *line_buf = (ARGB_PIXEL*)map_filter_buffer;
    ARGB_PIXEL Af, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    if (l_min < 1 || l_max < l_min || h_min < 1 || h_max < h_min ||
        out->height != fg->height || out->width != fg->width) {
        return;
    }
    else if (l_min == 1 && l_max == 1 && h_min == 1 && h_max == 1) {
        ARGB_MAP_blend_mul_global(out, 0, 0, fg, 1.0);
        return;
    }

    for (y = 0, h = 0, offs = 0; y < fg->height; y += h) {
        for (x = 0, l = 0; x < fg->width; x += l) {
            if (l_max == l_min)
                l = l_min;
            else
                l = l_min + PRN(seed_l)%(l_max-l_min);
            if (x+l > fg->width)
                l = fg->width - x;
            for (i = 0; i < l; i++) {
                line_buf[x+i] = ((ARGB_PIXEL*)fg->data)[offs+x];
            }
        }

        if (h_max == h_min)
            h = h_min;
        else
            h = h_min + PRN(seed_h)%(h_max-h_min);
        if (y+h > fg->height)
            h = fg->height - y;
        for (i = 0; i < h; i++) {
            for (x = 0; x < fg->width; x++, offs++) {
                pixval = line_buf[x];
                Af = ARGB_PIXEL_ALPHA(pixval);
                if (Af > 0) {
                    Rf = (ARGB_PIXEL_RED(pixval)*Af >> 8) << R_SHIFT;
                    Gf = (ARGB_PIXEL_GREEN(pixval)*Af >> 8) << G_SHIFT;
                    Bf = (ARGB_PIXEL_BLUE(pixval)*Af >> 8) << B_SHIFT;
                    pixval = ((ARGB_PIXEL*)bg->data)[offs];
                    Rb = (ARGB_PIXEL_RED(pixval)*(255-Af) >> 8) << R_SHIFT;
                    Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Af) >> 8) << G_SHIFT;
                    Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Af) >> 8) << B_SHIFT;
                    ((ARGB_PIXEL*)out->data)[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
                }
                else {
                    ((ARGB_PIXEL*)out->data)[offs] = ((ARGB_PIXEL*)bg->data)[offs];
                }
            }
        }
    }
}
