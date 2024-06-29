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

#define PREAMBLE_OUT_FG(M)                                                                            \
    if (out_x >= out->width || out_y >= out->height || out_x+fg->width <= 0 || out_y+fg->height <= 0) \
        return;                                                                                       \
    ARGB_PIXEL *out_no_f = out->data, *out_f = out->data;                                             \
    ARGB_PIXEL *fg_f = fg->data;                                                                      \
    INT t_w = fg->width-M;                                                                            \
    INT t_h = fg->height-M;                                                                           \
    INT x = 0, y = 0, ox = 0, oy = 0;                                                                 \
    if (out_x < 0) { t_w -= -out_x; fg_f += -out_x; }                                                 \
    if (out_y < 0) { t_h -= -out_y; fg_f += -out_y*fg->width; }                                       \
    if (out_x > 0) { out_f += out_x; ox = out_x;                                                      \
        if (out_x + t_w > out->width) t_w = out->width - out_x; }                                     \
    if (out_y > 0) { out_f += out_y*out->width; oy = out_y;                                           \
        if (out_y + t_h > out->height) t_h = out->height - out_y; }

#define PREAMBLE_OUT_BG_FG(M)                                                                         \
    if (out->width != bg->width || out->height != bg->height)                                         \
        return;                                                                                       \
    if (out_x >= out->width || out_y >= out->height || out_x+fg->width <= 0 || out_y+fg->height <= 0) \
        return;                                                                                       \
    ARGB_PIXEL *out_no_f = out->data, *out_f = out->data;                                             \
    ARGB_PIXEL *bg_no_f = bg->data, *bg_f = bg->data;                                                 \
    ARGB_PIXEL *fg_f = fg->data;                                                                      \
    INT t_w = fg->width-M < bg->width ? fg->width-M : bg->width;                                      \
    INT t_h = fg->height-M < bg->height ? fg->height-M : bg->height;                                  \
    INT x = 0, y = 0, ox = 0, oy = 0;                                                                 \
    if (out_x < 0) { t_w -= -out_x; fg_f += -out_x; }                                                 \
    if (out_y < 0) { t_h -= -out_y; fg_f += -out_y*fg->width; }                                       \
    if (out_x > 0) { out_f += out_x; bg_f += out_x; ox = out_x;                                       \
        if (out_x + t_w > out->width) t_w = out->width - out_x; }                                     \
    if (out_y > 0) { out_f += out_y*out->width; bg_f += out_y*out->width; oy = out_y;                 \
        if (out_y + t_h > out->height) t_h = out->height - out_y; }

#define PREAMBLE_P                                        \
    if (fg->width != p->width || fg->height != p->height) \
        return;                                           \
    ARGB_PIXEL *p_f = p->data;                            \
    if (out_x < 0) { p_f += -out_x; }                     \
    if (out_y < 0) { p_f += -out_y*p->width; }


//out_no_f - output pointer for storing unfiltered (background only) data
//out_f - output pointer for storing filtered (background x foreground) data
//bg_no_f - input pointer for reading unfiltered background data
//bg_f - input pointer for reading filtered background data
//fg_f - input pointer for reading filtered foreground data
//p_f - input pointer for reading filterint parameter data
//ox, oy - filter target start coordinates in out/bg surface

#define NEXT_Y_OUT_NO_F out_no_f += out->width;
#define NEXT_Y_OUT_F out_f += out->width;
#define NEXT_Y_BG_NO_F bg_no_f += bg->width;
#define NEXT_Y_BG_F bg_f += bg->width;
#define NEXT_Y_FG_F fg_f += fg->width;
#define NEXT_Y_P_F p_f += p->width;

void ARGB_MAP_green_gradient_global_copy(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *fg, GRADIENT *g, const INT p) {
    PREAMBLE_OUT_FG(p);
    INT l00, l01, l10, dl;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);

    for(y = 0; y < oy; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_NO_F;
    }

    for(; y < oy + t_h; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        for(x = 0; x < t_w; x++) {
            l00 = ARGB_PIXEL_GREEN(fg_f[x]);
            l10 = ARGB_PIXEL_GREEN(fg_f[x+p]);
            l01 = ARGB_PIXEL_GREEN(fg_f[x+p*fg->width]);
            dl = l10 + l01 - 2*l00;
            if (dl < 0) dl = -dl;
            out_f[x] = map_filter_dg->pixval[dl];
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_F;
        NEXT_Y_FG_F;
        NEXT_Y_OUT_NO_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_NO_F;
    }
}

void ARGB_MAP_green_gradient_global_blend(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *bg, ARGB_MAP *fg, GRADIENT *g, const INT p) {
    PREAMBLE_OUT_BG_FG(p);
    INT l00, l01, l10, dl;
    ARGB_PIXEL Ae, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);

    for(y = 0; y < oy; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    for(; y < oy + t_h; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        for(x = 0; x < t_w; x++) {
            l00 = ARGB_PIXEL_GREEN(fg_f[x]);
            l10 = ARGB_PIXEL_GREEN(fg_f[x+p]);
            l01 = ARGB_PIXEL_GREEN(fg_f[x+p*fg->width]);
            dl = l10 + l01 - 2*l00;
            if (dl < 0) dl = -dl;
            pixval = map_filter_dg->pixval[dl];
            Ae = ARGB_PIXEL_ALPHA(pixval);
            if (Ae > 0) {
                Rf = (ARGB_PIXEL_RED(pixval)*Ae >> 8) << R_SHIFT;
                Gf = (ARGB_PIXEL_GREEN(pixval)*Ae >> 8) << G_SHIFT;
                Bf = (ARGB_PIXEL_BLUE(pixval)*Ae >> 8) << B_SHIFT;
                pixval = bg_f[x];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Ae) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Ae) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Ae) >> 8) << B_SHIFT;
                out_f[x] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_F;
        NEXT_Y_BG_F;
        NEXT_Y_FG_F;
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }
}

void ARGB_MAP_green_gradient_per_pixel_copy(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *fg, GRADIENT *g, ARGB_MAP *p) {
    PREAMBLE_OUT_FG(MAX_EDGE_WIDTH);
    PREAMBLE_P;
    INT l00, l01, l10, dl, da;
    INT pa;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);

    for(y = 0; y < oy; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_NO_F;
    }

    for(; y < oy + t_h; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        for(x = 0; x < t_w; x++) {
            pa = ARGB_PIXEL_ALPHA(p_f[x]);
            da = pa >> 4; //max edge thickness (for pa==255) is MAX_EDGE_WIDTH
            l00 = ARGB_PIXEL_GREEN(fg_f[x]);
            l10 = ARGB_PIXEL_GREEN(fg_f[x+da]);
            l01 = ARGB_PIXEL_GREEN(fg_f[x+da*fg->width]);
            dl = l10 + l01 - 2*l00;
            if (dl < 0) dl = -dl;
            out_f[x] = map_filter_dg->pixval[dl*pa >> 8];
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_F;
        NEXT_Y_OUT_NO_F;
        NEXT_Y_FG_F;
        NEXT_Y_P_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = map_filter_dg->pixval[0];
        }
        NEXT_Y_OUT_NO_F;
    }
}

void ARGB_MAP_green_gradient_per_pixel_blend(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *bg, ARGB_MAP *fg, GRADIENT *g, ARGB_MAP *p) {
    PREAMBLE_OUT_BG_FG(MAX_EDGE_WIDTH);
    PREAMBLE_P;
    INT l00, l01, l10, dl, da, pa;
    ARGB_PIXEL Ae, Rf, Gf, Bf, Rb, Gb, Bb, pixval;

    map_filter_dg->length = 511;
    DISCRETE_GRADIENT_from_GRADIENT(map_filter_dg, g);

    for(y = 0; y < oy; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    for(; y < oy + t_h; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        for(x = 0; x < t_w; x++) {
            pa = ARGB_PIXEL_ALPHA(p_f[x]);
            da = pa >> 4; //max edge thickness (for pa==255) is MAX_EDGE_WIDTH
            l00 = ARGB_PIXEL_GREEN(fg_f[x]);
            l10 = ARGB_PIXEL_GREEN(fg_f[x+da]);
            l01 = ARGB_PIXEL_GREEN(fg_f[x+da*fg->width]);
            dl = l10 + l01 - 2*l00;
            if (dl < 0) dl = -dl;
            pixval = map_filter_dg->pixval[dl*pa >> 8];
            Ae = ARGB_PIXEL_ALPHA(pixval);
            if (Ae > 0) {
                Rf = (ARGB_PIXEL_RED(pixval)*Ae >> 8) << R_SHIFT;
                Gf = (ARGB_PIXEL_GREEN(pixval)*Ae >> 8) << G_SHIFT;
                Bf = (ARGB_PIXEL_BLUE(pixval)*Ae >> 8) << B_SHIFT;
                pixval = bg_f[x];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Ae) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Ae) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Ae) >> 8) << B_SHIFT;
                out_f[x] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_F;
        NEXT_Y_BG_F;
        NEXT_Y_FG_F;
        NEXT_Y_P_F;
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }
}

void ARGB_MAP_blur_nx1_global_copy(ARGB_MAP *out, INT out_x, INT out_y, COLOR *bg, ARGB_MAP *fg, const INT p) {
    PREAMBLE_OUT_FG(0);
    ARGB_PIXEL pixval, bg_pixval;
    ARGB_PIXEL r, g, b, rb, gb, bb, mul_f = (1<<FRACT_SHIFT)/p;
    const INT lp = (p-1)/2; //left half of p
    const INT rp = p/2; //right half of p

    if (p == 1) {
        ARGB_MAP_copy(out, fg);
        return;
    }
    bg_pixval = COLOR_to_ARGB_PIXEL(bg);

    for(y = 0; y < oy+1; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_pixval;
        }
        NEXT_Y_OUT_NO_F;
    }

    //omit first line from fg
    NEXT_Y_OUT_F;
    NEXT_Y_FG_F;
    //fill color accumulator with initial sum for the first blurred pixel
    r = g = b = 0;
    for(x = -(lp+1); x < rp; x++) {
        pixval = fg_f[x];
        r += ARGB_PIXEL_RED(pixval);
        g += ARGB_PIXEL_GREEN(pixval);
        b += ARGB_PIXEL_BLUE(pixval);
    }

    //this loop does not process first and last lines from fg
    //in order to "overlap" blur at left and right edges
    for(; y < oy + t_h-1; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = bg_pixval;
        }
        for(x = 0; x < fg->width; x++) {
            pixval = fg_f[x+rp];
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            pixval = fg_f[x-(lp+1)];
            r -= ARGB_PIXEL_RED(pixval);
            g -= ARGB_PIXEL_GREEN(pixval);
            b -= ARGB_PIXEL_BLUE(pixval);
            rb = (r*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b*mul_f >> FRACT_SHIFT) << B_SHIFT;
            out_f[x] = rb | gb | bb;
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = bg_pixval;
        }
        NEXT_Y_OUT_F;
        NEXT_Y_FG_F;
        NEXT_Y_OUT_NO_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_pixval;
        }
        NEXT_Y_OUT_NO_F;
    }
}

void ARGB_MAP_blur_nx1_global_blend(ARGB_MAP *out, INT out_x, INT out_y, ARGB_MAP *bg, ARGB_MAP *fg, const INT p) {
    PREAMBLE_OUT_BG_FG(0);
    ARGB_PIXEL pixval;
    ARGB_PIXEL a, r, g, b, Aavg, Rf, Gf, Bf, Rb, Gb, Bb, mul_f[256];
    ARGB_PIXEL mfa; //multiplication factor for alpha channel (inverse of blur distance)
    ARGB_PIXEL mfrgb; //multiplication factor for rgb channels (inverse of alpha total)
    const INT lp = (p-1)/2; //left half of p
    const INT rp = p/2; //right half of p

    if (p == 1) {
        ARGB_MAP_copy(out, bg);
        ARGB_MAP_blend_mul_global(out, 0, 0, fg, 1.0);
        return;
    }
    mul_f[0] = 1<<FRACT_SHIFT;
    for (x = 1; x < 256; x++) {
        mul_f[x] = (1<<FRACT_SHIFT)/x;
    }
    mfa = mul_f[p];

    for(y = 0; y < oy+1; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    //omit first line from fg and bg
    NEXT_Y_OUT_F;
    NEXT_Y_FG_F;
    NEXT_Y_BG_F;
    //fill color accumulators with initial sum for the first blurred pixel
    a = r = g = b = 0;
    for(x = -(lp+1); x < rp; x++) {
        pixval = fg_f[x];
        a += ARGB_PIXEL_ALPHA(pixval);
        r += ARGB_PIXEL_RED(pixval);
        g += ARGB_PIXEL_GREEN(pixval);
        b += ARGB_PIXEL_BLUE(pixval);
    }

    //this loop does not process first and last lines from fg
    //in order to "overlap" blur at left and right edges
    for(; y < oy + t_h-1; y++) {
        for(x = 0; x < ox; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        for(x = 0; x < fg->width; x++) {
            pixval = fg_f[x+rp];
            a += ARGB_PIXEL_ALPHA(pixval);
            r += ARGB_PIXEL_RED(pixval);
            g += ARGB_PIXEL_GREEN(pixval);
            b += ARGB_PIXEL_BLUE(pixval);
            pixval = fg_f[x-(lp+1)];
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
                pixval = bg_f[x];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                out_f[x] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                out_f[x] = bg_f[x];
            }
        }
        for(x = ox + t_w; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_F;
        NEXT_Y_FG_F;
        NEXT_Y_BG_F;
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
    }

    for(; y < out->height; y++) {
        for(x = 0; x < out->width; x++) {
            out_no_f[x] = bg_no_f[x];
        }
        NEXT_Y_OUT_NO_F;
        NEXT_Y_BG_NO_F;
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
        pixval = in->data[offs];
        r_buf[0] = ARGB_PIXEL_RED(pixval);
        g_buf[0] = ARGB_PIXEL_GREEN(pixval);
        b_buf[0] = ARGB_PIXEL_BLUE(pixval);
        for(x = 1; x < in->width; x++) {
            pixval = in->data[offs+x];
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
                out->data[offs] = r | g | b;
            }
            else {
                out->data[offs] = in->data[offs];
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
        pixval = fg->data[offs];
        a_buf[0] = ARGB_PIXEL_ALPHA(pixval);
        r_buf[0] = ARGB_PIXEL_RED(pixval);
        g_buf[0] = ARGB_PIXEL_GREEN(pixval);
        b_buf[0] = ARGB_PIXEL_BLUE(pixval);
        for(x = 1; x < fg->width; x++) {
            pixval = fg->data[offs+x];
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
                pixval = bg->data[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                out->data[offs] = bg->data[offs];
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
            pixval = in->data[offs];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
        }
    }

    for(y = 0, offs = 0; y < tp+1; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = in->data[offs+bp*in->width];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            out->data[offs] = rb | gb | bb;
        }
    }

    for(y = tp+1; y < in->height-bp; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = in->data[offs+bp*in->width];
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            pixval = in->data[offs-(tp+1)*in->width];
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            out->data[offs] = rb | gb | bb;
        }
    }

    for(y = in->height-bp; y < in->height; y++) {
        for(x = 0; x < in->width; x++, offs++) {
            pixval = in->data[offs-(tp+1)*in->width];
            r[x] -= ARGB_PIXEL_RED(pixval);
            g[x] -= ARGB_PIXEL_GREEN(pixval);
            b[x] -= ARGB_PIXEL_BLUE(pixval);
            rb = (r[x]*mul_f >> FRACT_SHIFT) << R_SHIFT;
            gb = (g[x]*mul_f >> FRACT_SHIFT) << G_SHIFT;
            bb = (b[x]*mul_f >> FRACT_SHIFT) << B_SHIFT;
            out->data[offs] = rb | gb | bb;
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
            pixval = fg->data[offs];
            a[x] += ARGB_PIXEL_ALPHA(pixval);
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
        }
    }

    for(y = 0, offs = 0; y < tp+1; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = fg->data[offs+bp*fg->width];
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
                pixval = bg->data[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                out->data[offs] = bg->data[offs];
            }
        }
    }

    for(y = tp+1; y < fg->height-bp; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = fg->data[offs+bp*fg->width];
            a[x] += ARGB_PIXEL_ALPHA(pixval);
            r[x] += ARGB_PIXEL_RED(pixval);
            g[x] += ARGB_PIXEL_GREEN(pixval);
            b[x] += ARGB_PIXEL_BLUE(pixval);
            pixval = fg->data[offs-(tp+1)*fg->width];
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
                pixval = bg->data[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                out->data[offs] = bg->data[offs];
            }
        }
    }

    for(y = fg->height-bp; y < fg->height; y++) {
        for(x = 0; x < fg->width; x++, offs++) {
            pixval = fg->data[offs-(tp+1)*fg->width];
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
                pixval = bg->data[offs];
                Rb = (ARGB_PIXEL_RED(pixval)*(255-Aavg) >> 8) << R_SHIFT;
                Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Aavg) >> 8) << G_SHIFT;
                Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Aavg) >> 8) << B_SHIFT;
                out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
            }
            else {
                out->data[offs] = bg->data[offs];
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
                line_buf[x+i] = in->data[offs+x];
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
                line_buf[x+i] = fg->data[offs+x];
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
                    pixval = bg->data[offs];
                    Rb = (ARGB_PIXEL_RED(pixval)*(255-Af) >> 8) << R_SHIFT;
                    Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Af) >> 8) << G_SHIFT;
                    Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Af) >> 8) << B_SHIFT;
                    out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
                }
                else {
                    out->data[offs] = bg->data[offs];
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
                line_buf[x+i] = in->data[offs+x];
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
                line_buf[x+i] = fg->data[offs+x];
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
                    pixval = bg->data[offs];
                    Rb = (ARGB_PIXEL_RED(pixval)*(255-Af) >> 8) << R_SHIFT;
                    Gb = (ARGB_PIXEL_GREEN(pixval)*(255-Af) >> 8) << G_SHIFT;
                    Bb = (ARGB_PIXEL_BLUE(pixval)*(255-Af) >> 8) << B_SHIFT;
                    out->data[offs] = (Rb+Rf) | (Gb+Gf) | (Bb+Bf);
                }
                else {
                    out->data[offs] = bg->data[offs];
                }
            }
        }
    }
}
