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

    typedef struct {
        INT x;
#if USE_Z
        INT z;
#endif
#if USE_MAP_BASE
        INT ub, vb;
    #if USE_INTERP
        #if USE_DIFF
        INT rd, gd, bd;
        #endif
        #if USE_SPEC
        INT rs, gs, bs;
        #endif
    #elif USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
        INT ur, vr;
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
        INT r, g, b;
#endif
    } POLY_EDGE;

    POLY_EDGE *polygon_edge = (POLY_EDGE *)polygon_edge_poll;
    POLY_EDGE *edge_ptr = NULL, *edge_end_ptr = NULL; //pointer to currently colored edge cell
    ARGB_PIXEL *draw_ptr = NULL, *end_draw_ptr = NULL;
    INT row_offset;
    INT e, evi; //Edge Vertex Increment
    INT vrt1, vrt2; //vertex indices for the beginning and the end of the edge
    INT xmin; //X minimum
    INT xmax; //X maximum
    INT ymin, ymin_v; //Y minimum, y minimum vertex index
    INT ymax, ymax_v; //Y maximum, y maximum vertex index

    INT x, x1, dx;
    INT y, y1, dy;
    ARGB_PIXEL pix_val; //final pixel value
    INT bar_length;

#if USE_Z
    Z_PIXEL *zbuf_ptr = NULL;
    INT z, z1, dz; //Z buffer pixel value
#endif

#if USE_MAP_BASE
    #if USE_MAP_BUMP
        BUMP_PIXEL *map_bs = mbase->data;
    #else
        ARGB_PIXEL *map_bs = mbase->data;
    #endif
    INT ub, ub1, dub; //U texture map coordinate
    INT vb, vb1, dvb; //V texture map coordinate
    INT vb_shift;
    #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
        //ASSUMPTION: map dimensions and coordinates are the same for "mul" and "add" maps
        INT ur, ur1, dur;
        INT vr, vr1, dvr;
        INT vr_shift;
    #endif
    #if USE_MAP_MUL
        ARGB_PIXEL *map_m = mmul->data;
        ARGB_PIXEL map_m_val;
        INT r, g, b;
    #endif
    #if USE_MAP_ADD
        ARGB_PIXEL *map_a = madd->data;
        ARGB_PIXEL map_a_val;
    #endif
    #if USE_MAP_BUMP
        ARGB_PIXEL *map_r = mref->data; //reflection map
        INT urc, vrc; //bump vector corrections read from map_bs[ub, vb]
        INT bu, bv; //bump vector along u and v axes
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
    INT r, r1, dr;
    INT g, g1, dg;
    INT b, b1, db;
#endif

#if USE_MAP_BASE
    #if USE_INTERP
        #if USE_DIFF
            //diffuse components
            INT rd, rd1, drd;
            INT gd, gd1, dgd;
            INT bd, bd1, dbd;
            //texture map components
            INT rm, gm, bm;
        #endif
        #if USE_SPEC
            //specular components
            INT rs, rs1, drs;
            INT gs, gs1, dgs;
            INT bs, bs1, dbs;
            ARGB_PIXEL spec_val;
        #endif
    #elif USE_FLAT
        #if USE_DIFF
            ARGB_PIXEL diff_val = COLOR_to_ARGB_PIXEL((COLOR*)diff); //diffuse component value
            ARGB_PIXEL diff_r = ARGB_PIXEL_RED(diff_val);
            ARGB_PIXEL diff_g = ARGB_PIXEL_GREEN(diff_val);
            ARGB_PIXEL diff_b = ARGB_PIXEL_BLUE(diff_val);
            ARGB_PIXEL r, g, b;
        #endif
        #if USE_SPEC
            ARGB_PIXEL spec_val = COLOR_to_ARGB_PIXEL((COLOR*)spec); //specular component value
        #endif
    #endif
#endif
//For flat shading without texture mapping calculate single value for every polygon pixel.
#if USE_SOLID
    pix_val = COLOR_to_ARGB_PIXEL((COLOR*)color);
#endif


    // Start of polygon drawing
    xmin = (*vp[0])[0];
    xmax = (*vp[0])[0];
    ymin = (*vp[0])[1]; ymin_v=0;
    ymax = (*vp[0])[1]; ymax_v=0;

    for (vrt1=1; vrt1<vcnt; vrt1++) { //get bounding rectangle coordinates from vertexes
        if ((*vp[vrt1])[0] < xmin) {
            xmin = (*vp[vrt1])[0]; }
        else if ((*vp[vrt1])[0] > xmax) {
            xmax = (*vp[vrt1])[0]; }

        if ((*vp[vrt1])[1] < ymin) {
            ymin = (*vp[vrt1])[1]; ymin_v = vrt1; }
        else if ((*vp[vrt1])[1] > ymax) {
            ymax = (*vp[vrt1])[1]; ymax_v = vrt1; }
    }

    // Skip drawing this polygon if it's outside of the screen or it's single horizontal line
    if (xmax < 0 || xmin > vrb_width-1 || ymax < 0 || ymin > vrb_height-1 || ymin == ymax) return;
    if (ymin < 0) ymin = 0;
    if (ymax > vrb_height-1) ymax = vrb_height-1;

#if USE_MAP_BASE
    //Calculate vb_shift: bit shift length for map v coordinate
    //This number is derived from log_2 of texture width
    vb_shift = FRACT_SHIFT;
    INT t = mbase->width-1;
    while(t) {
        vb_shift--;
        t >>= 1; }
    #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
        vr_shift = FRACT_SHIFT;
        #if USE_MAP_MUL
            t = mmul->width-1;
        #elif USE_MAP_ADD
            t = madd->width-1;
        #elif USE_MAP_BUMP
            t = mref->width-1;
        #endif
        while(t) {
            vr_shift--;
            t >>= 1; }
    #endif
#endif

    // Store edge vertex increment - needed to distinguish left and right edges
    evi = -1;

    /*
     Inside poly_edge calculate right edge first and left edge second
     */
    for (e=0; e<2; e++) {
        for (vrt1 = ymin_v; vrt1 != ymax_v; vrt1 = vrt2) {
            vrt2 = vrt1 + evi;
            if (vrt2 >= vcnt)
                vrt2 -= vcnt;
            else if (vrt2 < 0)
                vrt2 += vcnt;
            x  = (*vp[vrt1])[0] << FRACT_SHIFT;
            x1 = (*vp[vrt2])[0] << FRACT_SHIFT;
            y  = (*vp[vrt1])[1];
            y1 = (*vp[vrt2])[1];
            //Skip edges outside of the screen
            if (y > vrb_height-1 || y1 < 0) continue;
#if USE_Z
            z  = (*vp[vrt1])[2];    z1 = (*vp[vrt2])[2];
            dz = z1 - z;
#endif
#if USE_MAP_BASE
            ub  = mbc[vrt1].u << FRACT_SHIFT;
            vb  = mbc[vrt1].v << FRACT_SHIFT;
            ub1 = mbc[vrt2].u << FRACT_SHIFT;
            vb1 = mbc[vrt2].v << FRACT_SHIFT;
            dub = ub1 - ub;
            dvb = vb1 - vb;
    #if USE_INTERP
        #if USE_DIFF
            rd  = (INT)(255.*(*vdiff[vrt1]).r) << FRACT_SHIFT;
            gd  = (INT)(255.*(*vdiff[vrt1]).g) << FRACT_SHIFT;
            bd  = (INT)(255.*(*vdiff[vrt1]).b) << FRACT_SHIFT;
            rd1  = (INT)(255.*(*vdiff[vrt2]).r) << FRACT_SHIFT;
            gd1  = (INT)(255.*(*vdiff[vrt2]).g) << FRACT_SHIFT;
            bd1  = (INT)(255.*(*vdiff[vrt2]).b) << FRACT_SHIFT;
            drd = rd1 - rd;
            dgd = gd1 - gd;
            dbd = bd1 - bd;
        #endif
        #if USE_SPEC
            rs  = (INT)(255.*(*vspec[vrt1]).r) << FRACT_SHIFT;
            gs  = (INT)(255.*(*vspec[vrt1]).g) << FRACT_SHIFT;
            bs  = (INT)(255.*(*vspec[vrt1]).b) << FRACT_SHIFT;
            rs1  = (INT)(255.*(*vspec[vrt2]).r) << FRACT_SHIFT;
            gs1  = (INT)(255.*(*vspec[vrt2]).g) << FRACT_SHIFT;
            bs1  = (INT)(255.*(*vspec[vrt2]).b) << FRACT_SHIFT;
            drs = rs1 - rs;
            dgs = gs1 - gs;
            dbs = bs1 - bs;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
            ur  = mrc[vrt1].u << FRACT_SHIFT;
            vr  = mrc[vrt1].v << FRACT_SHIFT;
            ur1 = mrc[vrt2].u << FRACT_SHIFT;
            vr1 = mrc[vrt2].v << FRACT_SHIFT;
            dur = ur1 - ur;
            dvr = vr1 - vr;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
            r  = (INT)(255.*(*vcolor[vrt1]).r) << FRACT_SHIFT;
            g  = (INT)(255.*(*vcolor[vrt1]).g) << FRACT_SHIFT;
            b  = (INT)(255.*(*vcolor[vrt1]).b) << FRACT_SHIFT;
            r1 = (INT)(255.*(*vcolor[vrt2]).r) << FRACT_SHIFT;
            g1 = (INT)(255.*(*vcolor[vrt2]).g) << FRACT_SHIFT;
            b1 = (INT)(255.*(*vcolor[vrt2]).b) << FRACT_SHIFT;
            dr = r1 - r;
            dg = g1 - g;
            db = b1 - b;
#endif
            dx = x1 - x;    dy = y1 - y;
            if (dy != 0) {
                dx /= dy;
#if USE_Z
                dz /= dy;
#endif
#if USE_MAP_BASE
                dub /= dy;
                dvb /= dy;
    #if USE_INTERP
        #if USE_DIFF
                drd /= dy;
                dgd /= dy;
                dbd /= dy;
        #endif
        #if USE_SPEC
                drs /= dy;
                dgs /= dy;
                dbs /= dy;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                dur /= dy;
                dvr /= dy;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                dr /= dy;
                dg /= dy;
                db /= dy;
#endif
            }

            x += (1 << (FRACT_SHIFT-1));
#if USE_MAP_BASE
            ub += (1 << (FRACT_SHIFT-1));
            vb += (1 << (FRACT_SHIFT-1));
    #if USE_INTERP
        #if USE_DIFF
            rd += (1 << (FRACT_SHIFT-1));
            gd += (1 << (FRACT_SHIFT-1));
            bd += (1 << (FRACT_SHIFT-1));
        #endif
        #if USE_SPEC
            rs += (1 << (FRACT_SHIFT-1));
            gs += (1 << (FRACT_SHIFT-1));
            bs += (1 << (FRACT_SHIFT-1));
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
            ur += (1 << (FRACT_SHIFT-1));
            vr += (1 << (FRACT_SHIFT-1));
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
            r += (1 << (FRACT_SHIFT-1));
            g += (1 << (FRACT_SHIFT-1));
            b += (1 << (FRACT_SHIFT-1));
#endif

            if (y < 0) { //y1>=0 assured above by if (y > vrb_height-1 || y1 < 0)
                x += dx*-y;
#if USE_Z
                z += dz*-y;
#endif
#if USE_MAP_BASE
                ub += dub*-y;
                vb += dvb*-y;
    #if USE_INTERP
        #if USE_DIFF
                rd += drd*-y;
                gd += dgd*-y;
                bd += dbd*-y;
        #endif
        #if USE_SPEC
                rs += drs*-y;
                gs += dgs*-y;
                bs += dbs*-y;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                ur += dur*-y;
                vr += dvr*-y;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                r += dr*-y;
                g += dg*-y;
                b += db*-y;
#endif
                y = 0;
            }

            if (y1 > vrb_height-1) { //Clip the edge with the bottom edge of the screen
                y1 = vrb_height-1; }

            edge_ptr = polygon_edge + 2*y;
            edge_end_ptr = polygon_edge + 2*y1;

            if (e == RIGHT_EDGE) { //Right edge
                //The right edge is stored in odd cells (2*y + 1, 3, 5, 7, ...) of polygon_edge
                edge_ptr++;
                edge_end_ptr++; }

            // If there's more than one row to draw - store the first one
            if (dy != 0) {
                edge_ptr->x = x >> FRACT_SHIFT;
#if USE_Z
                edge_ptr->z = z;
#endif
#if USE_MAP_BASE
                edge_ptr->ub = ub;
                edge_ptr->vb = vb;
    #if USE_INTERP
        #if USE_DIFF
                edge_ptr->rd = rd;
                edge_ptr->gd = gd;
                edge_ptr->bd = bd;
        #endif
        #if USE_SPEC
                edge_ptr->rs = rs;
                edge_ptr->gs = gs;
                edge_ptr->bs = bs;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                edge_ptr->ur = ur;
                edge_ptr->vr = vr;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                edge_ptr->r = r;
                edge_ptr->g = g;
                edge_ptr->b = b;
#endif
                edge_ptr += 2;
            }

            //Calculate whole edge between vrt1 and vrt2
            while (edge_ptr <= edge_end_ptr) {
                x += dx;
                edge_ptr->x = x >> FRACT_SHIFT;
#if USE_Z
                z += dz;
                edge_ptr->z = z;
#endif
#if USE_MAP_BASE
                ub += dub;
                vb += dvb;
                edge_ptr->ub = ub;
                edge_ptr->vb = vb;
    #if USE_INTERP
        #if USE_DIFF
                rd += drd;
                gd += dgd;
                bd += dbd;
                edge_ptr->rd = rd;
                edge_ptr->gd = gd;
                edge_ptr->bd = bd;
        #endif
        #if USE_SPEC
                rs += drs;
                gs += dgs;
                bs += dbs;
                edge_ptr->rs = rs;
                edge_ptr->gs = gs;
                edge_ptr->bs = bs;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                ur += dur;
                vr += dvr;
                edge_ptr->ur = ur;
                edge_ptr->vr = vr;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                r += dr;
                g += dg;
                b += db;
                edge_ptr->r = r;
                edge_ptr->g = g;
                edge_ptr->b = b;
#endif
                edge_ptr += 2; }
        }
        evi = -evi; //switch to opposite side edges (when starting from ymin_v)
    }

    //Actual polygon drawing.
    //Each iteration of the loop draws one horizontal bar,
    //starting from the top(ymin), to the bottom one(ymax).
    edge_ptr = polygon_edge + 2*ymin;
    edge_end_ptr = polygon_edge + 2*ymax;
    row_offset = ymin*vrb_width;
    while (edge_ptr <= edge_end_ptr) {
        x = edge_ptr[0].x;
        x1 = edge_ptr[1].x;

        if (x1 >= 0 && x <= vrb_width -1) {
            bar_length = x1 - x + 1;
            if (bar_length < 1) {
                //Apparently polygon is not fully planar. Skip drawing it.
                //This might be also caused by coordinates roundoff inconsistencies.
                return;
            }
#if USE_Z
            z  = edge_ptr[0].z;
            z1 = edge_ptr[1].z;
            dz = (z1 - z)/bar_length;
#endif
#if USE_MAP_BASE
            ub  = edge_ptr[0].ub;
            vb  = edge_ptr[0].vb;
            ub1 = edge_ptr[1].ub;
            vb1 = edge_ptr[1].vb;
            dub = (ub1 - ub)/bar_length;
            dvb = (vb1 - vb)/bar_length;
    #if USE_INTERP
        #if USE_DIFF
            rd  = edge_ptr[0].rd;
            gd  = edge_ptr[0].gd;
            bd  = edge_ptr[0].bd;
            rd1 = edge_ptr[1].rd;
            gd1 = edge_ptr[1].gd;
            bd1 = edge_ptr[1].bd;
            drd = (rd1 - rd)/bar_length;
            dgd = (gd1 - gd)/bar_length;
            dbd = (bd1 - bd)/bar_length;
        #endif
        #if USE_SPEC
            rs  = edge_ptr[0].rs;
            gs  = edge_ptr[0].gs;
            bs  = edge_ptr[0].bs;
            rs1 = edge_ptr[1].rs;
            gs1 = edge_ptr[1].gs;
            bs1 = edge_ptr[1].bs;
            drs = (rs1 - rs)/bar_length;
            dgs = (gs1 - gs)/bar_length;
            dbs = (bs1 - bs)/bar_length;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
            ur  = edge_ptr[0].ur;
            vr  = edge_ptr[0].vr;
            ur1 = edge_ptr[1].ur;
            vr1 = edge_ptr[1].vr;
            dur = (ur1 - ur)/bar_length;
            dvr = (vr1 - vr)/bar_length;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
            r  = edge_ptr[0].r;
            g  = edge_ptr[0].g;
            b  = edge_ptr[0].b;
            r1 = edge_ptr[1].r;
            g1 = edge_ptr[1].g;
            b1 = edge_ptr[1].b;
            dr = (r1 - r)/bar_length;
            dg = (g1 - g)/bar_length;
            db = (b1 - b)/bar_length;
#endif

            //Clip drawing coefficients with left screen edge
            if (x < 0) {
                bar_length -= -x; //Adjust polygon bar length after clipping
#if USE_Z
                z += dz*-x;
#endif
#if USE_MAP_BASE
                ub += dub*-x;
                vb += dvb*-x;
    #if USE_INTERP
        #if USE_DIFF
                rd += drd*-x;
                gd += dgd*-x;
                bd += dbd*-x;
        #endif
        #if USE_SPEC
                rs += drs*-x;
                gs += dgs*-x;
                bs += dbs*-x;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                ur += dur*-x;
                vr += dvr*-x;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                r += dr*-x;
                g += dg*-x;
                b += db*-x;
#endif
                x = 0;
            }
            //Clip drawing coefficients with right screen edge
            if (x1 > vrb_width - 1) {
                bar_length = vrb_width - x; //Adjust polygon bar length after clipping
            }

            draw_ptr = vrb + row_offset + x;
#if USE_Z
            zbuf_ptr = vzb + row_offset + x;
#endif
            end_draw_ptr = draw_ptr + bar_length;
            while(draw_ptr < end_draw_ptr) {
#if USE_Z
                if (*zbuf_ptr > z) {
                    *zbuf_ptr = z;
#endif

#if USE_MAP_BASE
    #if USE_MAP_BUMP
                    //Extraction of bump reflection vectors. Correct version:
                    //bv = map_bs[(vb&~FRACT_MASK)>>vb_shift | ub>>FRACT_SHIFT]&0xFFFF0000;
                    //bu = (map_bs[(vb&~FRACT_MASK)>>vb_shift | ub>>FRACT_SHIFT]&0x0000FFFF)<<FRACT_SHIFT;
                    //Incorrect, but faster version:
                    bv = map_bs[(vb&~FRACT_MASK)>>vb_shift | ub>>FRACT_SHIFT];
                    bu = bv << FRACT_SHIFT;

                    vrc = vr+bv;
                    urc = ur+bu;
                    pix_val = map_r[(vrc&~FRACT_MASK)>>vr_shift | urc>>FRACT_SHIFT];
    #else
                    pix_val = map_bs[(vb&~FRACT_MASK)>>vb_shift | ub>>FRACT_SHIFT];
    #endif
    #if USE_FLAT
        #if USE_DIFF
                    r = ARGB_PIXEL_RED(pix_val);
                    g = ARGB_PIXEL_GREEN(pix_val);
                    b = ARGB_PIXEL_BLUE(pix_val);
                    pix_val = A_MASK |
                        (diff_r*r >> 8) << R_SHIFT |
                        (diff_g*g >> 8) << G_SHIFT |
                        (diff_b*b >> 8) << B_SHIFT;
        #endif
        #if USE_SPEC
                    pix_val = (pix_val&0xFEFEFEFF) + (spec_val&0x00FEFEFF);
                    if (pix_val & R_OVFL) {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|G_MASK|B_MASK;
                            else
                                pix_val |= R_MASK|G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|B_MASK;
                            else
                                pix_val |= R_MASK;
                        }
                    }
                    else {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= G_MASK|B_MASK;
                            else
                                pix_val |= G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= B_MASK;
                        }
                    }
        #endif
    #elif USE_INTERP
        #if USE_DIFF
                    rm = ARGB_PIXEL_RED(pix_val);
                    gm = ARGB_PIXEL_GREEN(pix_val);
                    bm = ARGB_PIXEL_BLUE(pix_val);
                    pix_val = A_MASK |
                            ((rd>>FRACT_SHIFT)*rm >> 8) << R_SHIFT |
                            ((gd>>FRACT_SHIFT)*gm >> 8) << G_SHIFT |
                            ((bd>>FRACT_SHIFT)*bm >> 8) << B_SHIFT;
        #endif
        #if USE_SPEC
                    //TODO wrap in macrodefinitions ths spec_val expression
                    spec_val = (rs & 0x0FF0000) | ((gs & 0x0FF0000)>>8) | (bs>>16);
                    pix_val = (pix_val&0xFEFEFEFF) + (spec_val&0x00FEFEFF);
                    if (pix_val & R_OVFL) {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|G_MASK|B_MASK;
                            else
                                pix_val |= R_MASK|G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|B_MASK;
                            else
                                pix_val |= R_MASK;
                        }
                    }
                    else {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= G_MASK|B_MASK;
                            else
                                pix_val |= G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= B_MASK;
                        }
                    }
        #endif
    #else
        #if USE_MAP_MUL
                    map_m_val = map_m[(vr&~FRACT_MASK)>>vr_shift | ur>>FRACT_SHIFT];
                    r = ARGB_PIXEL_RED(pix_val)*ARGB_PIXEL_RED(map_m_val) >> 8;
                    g = ARGB_PIXEL_GREEN(pix_val)*ARGB_PIXEL_GREEN(map_m_val) >> 8;
                    b = ARGB_PIXEL_BLUE(pix_val)*ARGB_PIXEL_BLUE(map_m_val) >> 8;
                    pix_val = A_MASK | (r << R_SHIFT) | (g << G_SHIFT) | (b << B_SHIFT);
        #endif
        #if USE_MAP_ADD
                    map_a_val = map_a[(vr&~FRACT_MASK)>>vr_shift | ur>>FRACT_SHIFT];
                    pix_val = (pix_val&0xFEFEFEFF) + (map_a_val&0x00FEFEFF);
                    if (pix_val & R_OVFL) {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|G_MASK|B_MASK;
                            else
                                pix_val |= R_MASK|G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= R_MASK|B_MASK;
                            else
                                pix_val |= R_MASK;
                        }
                    }
                    else {
                        if (pix_val & G_OVFL) {
                            if (pix_val & B_OVFL)
                                pix_val |= G_MASK|B_MASK;
                            else
                                pix_val |= G_MASK;
                        }
                        else {
                            if (pix_val & B_OVFL)
                                pix_val |= B_MASK;
                        }
                    }
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                    pix_val = A_MASK | 
                              ((r >> FRACT_SHIFT) << R_SHIFT) | 
                              ((g >> FRACT_SHIFT) << G_SHIFT) | 
                              ((b >> FRACT_SHIFT) << B_SHIFT);
#endif

                    *draw_ptr = pix_val;
#if USE_Z
                }
                z += dz;
                zbuf_ptr++;
#endif
#if USE_MAP_BASE
                ub += dub;    vb += dvb;
    #if USE_INTERP
        #if USE_DIFF
                rd += drd;    gd += dgd;    bd += dbd;
        #endif
        #if USE_SPEC
                rs += drs;    gs += dgs;    bs += dbs;
        #endif
    #else
        #if USE_MAP_MUL || USE_MAP_ADD || USE_MAP_BUMP
                ur += dur;    vr += dvr;
        #endif
    #endif
#elif USE_INTERP //&& !USE_MAP_BASE
                r += dr;    g += dg;    b += db;
#endif
                draw_ptr++;
            }
        }
        edge_ptr += 2;
        row_offset += vrb_width;
    }