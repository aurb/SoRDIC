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

#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "v_renderer.h"
#include "v_geometry.h"

#define RIGHT_EDGE (1)
#define LEFT_EDGE (0)
#define FRACT_BITS (16)
#define FRACT_MASK ((1<<16)-1)

RGB_PIXEL *vhbb = NULL; //vector horizontal bar buffer
void *polygon_edge_poll = NULL;

RGB_PIXEL *vrb = NULL; //vector renderer render buffer
Z_PIXEL *vzb = NULL; //vector renderer z buffer
INT vrb_width = 0;
INT vrb_height = 0;

INT get_abs(const INT x) {
    return x<0 ? -x : x;
}

INT get_sign(const INT x) {
    return x==0 ? 0 : (x<0 ? -1 : 1);
}

void swap_int(INT *a, INT *b) {
    INT t = *a; *a = *b; *b = t;
}

void vr_init() {
    vhbb = calloc(10000, sizeof(RGB_PIXEL)); //bigger than any possible rendering buffer width
    polygon_edge_poll = malloc(100000); //bigger than any possible rendering buffer 2*height
    vr_geometry_init();
}

void vr_set_render_buffer(const RENDER_BUFFER* rb) {
    vrb = rb->p;
    vzb = rb->z;
    vrb_width = rb->width;
    vrb_height = rb->height;
}

void vr_cleanup() {
    free(vhbb);
    free(polygon_edge_poll);
    vhbb = NULL;
    polygon_edge_poll = NULL;
    vr_geometry_cleanup();
}

void line_flat_draw_h_bar(RGB_PIXEL* ptr, const INT offset, const RGB_PIXEL v)
{
    RGB_PIXEL* curr_ptr = ptr;
    RGB_PIXEL* end_ptr = ptr + offset;
    if (offset == 0) {
        *curr_ptr = v;
    }
    else {
        while(curr_ptr != end_ptr)
        {
            if (offset > 0)
                *curr_ptr++ = v;
            else
                *curr_ptr-- = v;
        }
    }
}

void line_flat_draw_v_bar(RGB_PIXEL* ptr, const INT offset, const RGB_PIXEL v)
{
    RGB_PIXEL* curr_ptr = ptr;
    RGB_PIXEL* end_ptr = ptr + offset*vrb_width;
    if (offset == 0) {
        *curr_ptr = v;
    }
    else {
        while(curr_ptr != end_ptr)
        {
            *curr_ptr = v;
            if (offset > 0)
                curr_ptr+=vrb_width;
            else
                curr_ptr-=vrb_width;
        }
    }
}
//////////////////////////////////////////////
// Fixed point arithmetcs line drawing algorithm
// FRACT_BITS lower bits is used for keeping fractional part
//////////////////////////////////////////////
void line_flat(INT x0, INT y0, INT x1, INT y1, VEC_3 *color)
{
    INT dx, dy, pdx, pdy, px_prev, py_prev;
    INT py, py1, px, px1; //x & y coordinate pixel offsets
    INT bar_length;
    //line endings clipped by render buffer(screen) boundaries
    INT x0c, x1c, y0c, y1c;
    INT xmin=0, xmax=0;
    RGB_PIXEL pix_val = utils_RGB_2_pix((VEC_3*)color);

    /* Make sure that original point 0 is higher than original point 1 */
    if (y1 < y0) {
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
    }

    dx = x1 - x0;
    dy = y1 - y0;

    xmin = xmax = x0;
    if (xmin > x1)
        xmin = x1;
    else
        xmax = x1;
    /* If line is outside of the render buffer then skip drawing it altogether */
    if (xmax < 0 || xmin > vrb_width-1 || y1 < 0 || y0 > vrb_height-1) return;

    bool intersect = false; //flag indicating that at least one intersection was found

    INT top_x=-1, bottom_x=-1, left_y=-1, right_y=-1; //intersections coordinates
    /* For lines extending beyond screen limits find
       where they intersect with either of lines:
       y==0, y==vrb_height-1, x==0, x==vrb_width-1 */
    if (y0 < 0) {
        top_x = x0 + dx*(0-y0)/dy;
        intersect = true;
    }
    if (y1 > vrb_height-1) {
        bottom_x = x0 + dx*(vrb_height-1-y0)/dy;
        intersect = true;
    }
    if (x0 < 0 || x1 < 0) {
        left_y = y0 + dy*(0-x0)/dx;
        intersect = true;
    }
    if (x0 > vrb_width-1 || x1 > vrb_width-1) {
        right_y = y0 + dy*(vrb_width-1-x0)/dx;
        intersect = true;
    }
    //reject intersections to the right/below the screen
    if (top_x >= vrb_width-1)
        top_x = -1;
    if (bottom_x >= vrb_width-1)
        bottom_x = -1;
    if (left_y >= vrb_height)
        left_y = -1;
    if (right_y >= vrb_height)
        right_y = -1;

    // do not draw if all the detected intersects are outside of the screen limits
    if (intersect == 1 && top_x <= 0 && bottom_x < 0 && left_y < 0 && right_y < 0) return;

    x0c = x0;
    y0c = y0;
    x1c = x1;
    y1c = y1;

    /* Clip the line to the screen limits */
    /* If the point 0 is outside of the screen limits then
       change it for one of the intersections */
    if (x0c < 0 || x0c >= vrb_width || y0c < 0) {
        if (top_x >= 0) {
            x0c = top_x;
            y0c = 0;
        }
        else if (left_y >= 0) {
            x0c = 0;
            y0c = left_y;
        }
        else if (right_y >= 0) {
            x0c = vrb_width-1;
            y0c = right_y;
        }
    }
    /* If the point 1 is outside of the screen limits then
       change it for one of the intersections */
    if (x1c < 0 || x1c >= vrb_width || y1c >= vrb_height) {
        if (bottom_x >= 0) {
            x1c = bottom_x;
            y1c = vrb_height-1;
        }
        else if (right_y >= 0) {
            x1c = vrb_width-1;
            y1c = right_y;
        }
        else if (left_y >= 0) {
            x1c = 0;
            y1c = left_y;
        }
    }

    /* Make sure that clipped point 0 is higher than clipped point 1 */
    if (y1c < y0c) {
        swap_int(&x0c, &x1c);
        swap_int(&y0c, &y1c);
    }

    dx = x1c - x0c;
    dy = y1c - y0c;

    if (get_abs(dx) >= get_abs(dy)) {
        py = y0c*vrb_width;
        py1 = y1c*vrb_width;
        pdy = get_sign(dy)*vrb_width;

        px = (x0c << FRACT_BITS)+(1 << (FRACT_BITS-1));
        dx += get_sign(dx);
        pdx = (dx << FRACT_BITS)/(get_abs(dy)+1);

        while (py != py1) {
            px_prev = px;
            px += pdx;
            line_flat_draw_h_bar(vrb+py+(px_prev >> FRACT_BITS), (px >> FRACT_BITS)-(px_prev >> FRACT_BITS), pix_val);
            py += pdy;
        }
        bar_length = x1c-(px >> FRACT_BITS);
        line_flat_draw_h_bar(vrb+py+(px >> FRACT_BITS), bar_length + get_sign(bar_length), pix_val);
    }
    else {
        px = x0c;
        px1 = x1c;
        pdx = get_sign(dx);

        py = (y0c << FRACT_BITS)+(1 << (FRACT_BITS-1));
        dy += get_sign(dy);
        pdy = (dy << FRACT_BITS)/(get_abs(dx)+1);

        while (px != px1) {
            py_prev = py;
            py += pdy;
            line_flat_draw_v_bar(vrb+(py_prev >> FRACT_BITS)*vrb_width+px, (py >> FRACT_BITS)-(py_prev >> FRACT_BITS), pix_val);
            px += pdx;
        }
        bar_length = y1c-(py >> FRACT_BITS);
        line_flat_draw_v_bar(vrb+(py >> FRACT_BITS)*vrb_width+px, bar_length + get_sign(bar_length), pix_val);
    }
}

void line_flat_z(PROJECTION_COORD** v, VEC_3 *color)
{
    INT x0=0, y0=0, z0=0, x1=0, y1=0, z1=0;
    INT dx=0, dy=0, dz=0; //total delta x/y/z

    //line endings clipped by render buffer(screen) boundaries
    INT x0c, x1c, y0c, y1c, z0c, z1c;
    INT xmin=0, xmax=0;
    RGB_PIXEL pix_val = utils_RGB_2_pix((VEC_3*)color);

    x0 = (*v[0])[0];
    x1 = (*v[1])[0];
    y0 = (*v[0])[1];
    y1 = (*v[1])[1];
    z0 = (*v[0])[2];
    z1 = (*v[1])[2];

    /* Make sure that original point 0 is higher than original point 1 */
    if (y1 < y0) {
        swap_int(&x0, &x1);
        swap_int(&y0, &y1);
        swap_int(&z0, &z1);
    }

    dx = x1 - x0;
    dy = y1 - y0;
    dz = z1 - z0;

    xmin = xmax = x0;
    if (xmin > x1)
        xmin = x1;
    else
        xmax = x1;

    /* If line is outside of render buffer then skip drawing line altogether */
    if (xmax < 0 || xmin > vrb_width-1 || y1 < 0 || y0 > vrb_height-1) return;

    bool intersect = false; //flag indicating that at least one intersection was found

    INT top_x=-1, bottom_x=-1, left_y=-1, right_y=-1; //intersections coordinates
    INT top_z=-1, bottom_z=-1, left_z=-1, right_z=-1;
    /* For lines extending beyond screen limits find
       where they intersect with either of lines:
       y==0, y==vrb_height-1, x==0, x==vrb_width-1 */
    if (y0 < 0) {
        top_x = x0 + dx*(0-y0)/dy;
        top_z = z0 + dz*(0-y0)/dy;
        intersect = true;
    }
    if (y1 > vrb_height-1) {
        bottom_x = x0 + dx*(vrb_height-1-y0)/dy;
        bottom_z = z0 + dz*(vrb_height-1-y0)/dy;
        intersect = true;
    }
    if (x0 < 0 || x1 < 0) {
        left_y = y0 + dy*(0-x0)/dx;
        left_z = z0 + dz*(0-x0)/dx;
        intersect = true;
    }
    if (x0 > vrb_width-1 || x1 > vrb_width-1) {
        right_y = y0 + dy*(vrb_width-1-x0)/dx;
        right_z = z0 + dz*(vrb_width-1-x0)/dx;
        intersect = true;
    }
    //reject intersections to the right/below the screen
    if (top_x >= vrb_width-1)
        top_x = -1;
    if (bottom_x >= vrb_width-1)
        bottom_x = -1;
    if (left_y >= vrb_height)
        left_y = -1;
    if (right_y >= vrb_height)
        right_y = -1;

    // do not draw if all the detected intersects are outside of the screen limits
    if (intersect == 1 && top_x <= 0 && bottom_x < 0 && left_y < 0 && right_y < 0) return;

    x0c = x0;
    y0c = y0;
    z0c = z0;
    x1c = x1;
    y1c = y1;
    z1c = z1;

    /* Clip the line to the screen limits */
    /* If the point 0 is outside of the screen limits then
       change it for one of the intersections */
    if (x0c < 0 || x0c >= vrb_width || y0c < 0) {
        if (top_x >= 0) {
            x0c = top_x;
            y0c = 0;
            z0c = top_z;
        }
        else if (left_y >= 0) {
            x0c = 0;
            y0c = left_y;
            z0c = left_z;
        }
        else if (right_y >= 0) {
            x0c = vrb_width-1;
            y0c = right_y;
            z0c = right_z;
        }
    }
    /* If the point 1 is outside of the screen limits then
       change it for one of the intersections */
    if (x1c < 0 || x1c >= vrb_width || y1c >= vrb_height) {
        if (bottom_x >= 0) {
            x1c = bottom_x;
            y1c = vrb_height-1;
            z1c = bottom_z;
        }
        else if (right_y >= 0) {
            x1c = vrb_width-1;
            y1c = right_y;
            z1c = right_z;
        }
        else if (left_y >= 0) {
            x1c = 0;
            y1c = left_y;
            z1c = left_z;
        }
    }

    /* Make sure that clipped point 0 is higher than clipped point 1 */
    if (y1c < y0c) {
        swap_int(&x0c, &x1c);
        swap_int(&y0c, &y1c);
        swap_int(&z0c, &z1c);
    }

    dx = x1c - x0c;
    dy = y1c - y0c;
    dz = z1c - z0c;

    INT c=0, pdc=0, xi=0; //current x/y, pixel delta x/y, x increment
    INT z=0, pdz=0; //current z, pixel delta z
    INT fc = 0, fp = 0; //floor of current x/y, floor of previous x/y
    RGB_PIXEL* pix_ptr = NULL, *final_pix_ptr = NULL;
    Z_PIXEL *zbuf_ptr = NULL;
    INT ptr_delta_switch = 0, ptr_delta_no_switch = 0;

    pix_ptr = vrb + y0c*vrb_width + x0c; //initial drawing pixel
    zbuf_ptr = vzb + y0c*vrb_width + x0c; //initial Z buffer pixel
    final_pix_ptr = vrb + y1c*vrb_width + x1c; //final drawing pixel
    xi = dx >= 0 ? 1 : -1; //x increment
    z = z0;

    if (get_abs(dx) >= dy) { //line is longer horizontally than vertically
        c = (y0c << FRACT_BITS) + (1 << (FRACT_BITS-1)); //y coordinate (fixed point), later updated after each drawn pixel
        pdc = dx!=0 ? (dy<<FRACT_BITS)/get_abs(dx) : 0; //y coordinate delta (fixed point), for each line pixel
        pdz = dx!=0 ? dz/get_abs(dx) : 0; //y coordinate delta (fixed point), for each line pixel
        ptr_delta_switch = vrb_width+xi; //offset between adjacent pixels on the line. With y coordinate increment
        ptr_delta_no_switch = xi; //offset between adjacent pixels on the line. No y coordinate increment
    }
    else { //line is longer vertically than horizontally
        c = (x0c << FRACT_BITS) + (1 << (FRACT_BITS-1));
        pdc = dy!=0 ? (dx<<FRACT_BITS)/dy : 0;
        pdz = dy!=0 ? dz/dy : 0;
        ptr_delta_switch = vrb_width+xi;
        ptr_delta_no_switch = vrb_width;
    }
    fc = c >> FRACT_BITS; //first pixel integer coordinate

    while(pix_ptr != final_pix_ptr) {
        if (z <= *zbuf_ptr)
            *pix_ptr = pix_val; //draw current pixel
        fp = fc;
        c += pdc; //calculate next pixel coordinate (fixed point)
        z += pdz;
        fc = c >> FRACT_BITS; //get integer for next pixel coordinate
        if (fc != fp) { //switch to next pixel depending where it is located
            pix_ptr += ptr_delta_switch;
            zbuf_ptr += ptr_delta_switch;
        }
        else {
            pix_ptr += ptr_delta_no_switch;
            zbuf_ptr += ptr_delta_no_switch;
        }
    }
    if (z <= *zbuf_ptr)
        *pix_ptr = pix_val;
}

//////////////////////////////////////////////
// POLYGONS
//////////////////////////////////////////////
void polygon_solid(INT vcnt, PROJECTION_COORD** vp, VEC_3 *color)
{
#define USE_SOLID 1
#include "polygon.h"
#undef USE_SOLID
}

void polygon_solid_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 *color)
{
#define USE_Z 1
#define USE_SOLID 1
#include "polygon.h"
#undef USE_SOLID
#undef USE_Z
}

//////////////////////////////////////////////
//Gouraud shaded polygon with z test
//////////////////////////////////////////////
void polygon_interp_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 **vcolor)
{
#define USE_Z 1
#define USE_INTERP 1
#include "polygon.h"
#undef USE_INTERP
#undef USE_Z
}

//////////////////////////////////////////////
//Affine textured polygon with z test
//////////////////////////////////////////////
void polygon_texture_base_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#include "polygon.h"
#undef USE_MAP_BASE
#undef USE_Z
}




void polygon_texture_bump_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const BUMP_MAP * const mbase, MAP_COORD *mrc, const MAP * const mref)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_MAP_BUMP 1
#include "polygon.h"
#undef USE_MAP_BUMP
#undef USE_MAP_BASE
#undef USE_Z
}




void polygon_texture_base_mul_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const MAP * const mbase, MAP_COORD *mrc, const MAP * const mmul)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_MAP_MUL 1
#include "polygon.h"
#undef USE_MAP_MUL
#undef USE_MAP_BASE
#undef USE_Z
}

void polygon_texture_base_add_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const MAP * const mbase, MAP_COORD *mrc, const MAP * const madd)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_MAP_ADD 1
#include "polygon.h"
#undef USE_MAP_ADD
#undef USE_MAP_BASE
#undef USE_Z
}

void polygon_texture_base_mul_add_z(INT vcnt, PROJECTION_COORD** vp, MAP_COORD *mbc, const MAP * const mbase, MAP_COORD *mrc, const MAP * const mmul, const MAP * const madd)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_MAP_MUL 1
#define USE_MAP_ADD 1
#include "polygon.h"
#undef USE_MAP_ADD
#undef USE_MAP_MUL
#undef USE_MAP_BASE
#undef USE_Z
}

void polygon_solid_diff_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 *diff, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_FLAT 1
#define USE_DIFF 1
#define USE_MAP_BASE 1
#include "polygon.h"
#undef USE_MAP_BASE
#undef USE_DIFF
#undef USE_FLAT
#undef USE_Z
}

void polygon_solid_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 *spec, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_FLAT 1
#define USE_SPEC 1
#define USE_MAP_BASE 1
#include "polygon.h"
#undef USE_MAP_BASE
#undef USE_SPEC
#undef USE_FLAT
#undef USE_Z
}

void polygon_solid_diff_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 *diff, VEC_3 *spec, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_FLAT 1
#define USE_DIFF 1
#define USE_SPEC 1
#define USE_MAP_BASE 1
#include "polygon.h"
#undef USE_MAP_BASE
#undef USE_SPEC
#undef USE_DIFF
#undef USE_FLAT
#undef USE_Z
}

void polygon_interp_diff_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 **vdiff, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_INTERP 1
#define USE_DIFF 1
#include "polygon.h"
#undef USE_DIFF
#undef USE_INTERP
#undef USE_MAP_BASE
#undef USE_Z
}

void polygon_interp_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 **vspec, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_INTERP 1
#define USE_SPEC 1
#include "polygon.h"
#undef USE_SPEC
#undef USE_INTERP
#undef USE_MAP_BASE
#undef USE_Z
}

void polygon_interp_diff_spec_texture_z(INT vcnt, PROJECTION_COORD** vp, VEC_3 **vdiff, VEC_3 **vspec, MAP_COORD *mbc, const MAP * const mbase)
{
#define USE_Z 1
#define USE_MAP_BASE 1
#define USE_INTERP 1
#define USE_DIFF 1
#define USE_SPEC 1
#include "polygon.h"
#undef USE_SPEC
#undef USE_DIFF
#undef USE_INTERP
#undef USE_MAP_BASE
#undef USE_Z
}
