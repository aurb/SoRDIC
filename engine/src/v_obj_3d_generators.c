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

//Internal functions forward declarations
VEC_4* stub_surface(FLOAT u, FLOAT v);
VEC_4* cycloid_surface(CYCLOID *cld, FLOAT u, FLOAT v);
VEC_4* toroid_uv(FLOAT u, FLOAT v);
static FLOAT surf_params[5];

OBJ_3D * obj_3d_light(COLOR *color) {
    OBJ_3D *obj = obj_3d(1, 0);
    obj->type = POINT_LIGHTS;
    copy_v4(&obj->vertices[0].root, &(VEC_4){0.0, 0.0, 0.0, 1.0});
    obj->vertices[0].color_surf = *color;
    return obj;
}

OBJ_3D * obj_3d_regular_polyhedron(POLYHEDRON_TYPE type, FLOAT a) {
    const INT tetrahedronVerticesNum = 4;
    const INT tetrahedronFacesNum = 4;
    const INT cubeVerticesNum = 8;
    const INT cubeFacesNum = 6;
    const INT octahedronVerticesNum = 6;
    const INT octahedronFacesNum = 8;
    const INT dodecahedronVerticesNum = 20;
    const INT dodecahedronFacesNum = 12;
    const INT icosahedronVerticesNum = 12;
    const INT icosahedronFacesNum = 20;

    const FLOAT tetrahedronVertices[][3] = {
        {1, 1, 1}, {-1, -1, 1}, {-1, 1, -1}, {1, -1, -1}
    };
    const FLOAT cubeVertices[][3] = {
        {-1, -1, -1}, {-1, -1,  1}, {-1,  1, -1}, {-1,  1,  1}, { 1, -1, -1}, { 1, -1,  1},
        { 1,  1, -1}, { 1,  1,  1}
    };
    const FLOAT octahedronVertices[][3] = {
        { 0, -1,  0}, { 0,  0,  1}, { 1,  0,  0}, { 0,  0, -1}, {-1,  0,  0}, { 0,  1,  0},
    };
    #define B1 (1. / 1.618033989)
    #define C1 (2. - 1.618033989)
    const FLOAT dodecahedronVertices[][3] = {
        { 0,  1,  C1}, { 0,  1, -C1}, { 0, -1,  C1}, { 0, -1, -C1}, { 1,  C1,  0}, { 1, -C1,  0},
        {-1,  C1,  0}, {-1, -C1,  0}, { B1,  B1,  B1}, { B1,  B1, -B1}, { B1, -B1,  B1}, { B1, -B1, -B1},
        {-B1,  B1,  B1}, {-B1,  B1, -B1}, {-B1, -B1,  B1}, {-B1, -B1, -B1}, { C1,  0,  1}, { C1,  0, -1},
        {-C1,  0,  1}, {-C1,  0, -1}
    };
    #define B2 (0.618033989)
    #define C2 (1.0)
    FLOAT icosahedronVertices[][3] = {
        {  0,  B2, -C2}, { B2,  C2,   0}, {-B2,  C2,   0}, {  0,  B2,  C2}, {  0, -B2,  C2}, {-C2,   0,  B2},
        {  0, -B2, -C2}, { C2,   0, -B2}, { C2,   0,  B2}, {-C2,   0, -B2}, { B2, -C2,   0}, {-B2, -C2,   0}
    };

    const INT tetrahedronFaces[][3] = {
        {2, 0, 1}, {1, 0, 3}, {3, 0, 2}, {2, 1, 3}
    };
    const INT cubeFaces[][4] = {
        {2, 3, 1, 0}, {4, 5, 7, 6}, {0, 1, 5, 4}, {6, 7, 3, 2}, {4, 6, 2, 0}, {1, 3, 7, 5},
    };
    const INT octahedronFaces[][3] = {
        {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 1}, {5, 4, 3}, {5, 3, 2}, {5, 2, 1}, {5, 1, 4},
    };
    const INT dodecahedronFaces[][5] = {
        {16, 18, 12, 0, 8}, {18, 16, 10, 2, 14}, {17, 19, 15, 3, 11}, {19, 17, 9, 1, 13}, {9, 4, 8, 0, 1},
        {12, 6, 13, 1, 0}, {15, 7, 14, 2, 3}, {10, 5, 11, 3, 2}, {4, 5, 10, 16, 8}, {5, 4, 9, 17, 11},
        {6, 7, 15, 19, 13}, {7, 6, 12, 18, 14}
    };
    const INT icosahedronFaces[][3] = {
        {2, 0, 1}, {1, 3, 2}, {5, 3, 4}, {4, 3, 8}, {7, 0, 6}, {6, 0, 9}, {11, 4, 10}, {10, 6, 11},
        {9, 2, 5}, {5, 11, 9}, {8, 1, 7}, {7, 10, 8}, {2, 3, 5}, {8, 3, 1}, {9, 0, 2}, {1, 0, 7},
        {11, 6, 9}, {7, 6, 10}, {5, 4, 11}, {10, 4, 8}
    };

    INT i = 0, j = 0, verticesNum = 0, facesNum = 0;
    const FLOAT (*vertices)[3] = NULL;
    const INT (*faces3v)[3] = NULL;
    const INT (*faces4v)[4] = NULL;
    const INT (*faces5v)[5] = NULL;

    switch (type) {
        case TETRAHEDRON:
            verticesNum = tetrahedronVerticesNum;
            facesNum = tetrahedronFacesNum;
            vertices = tetrahedronVertices;
            faces3v = tetrahedronFaces;
            break;
        case CUBE:
            verticesNum = cubeVerticesNum;
            facesNum = cubeFacesNum;
            vertices = cubeVertices;
            faces4v = cubeFaces;
            break;
        case OCTAHEDRON:
            verticesNum = octahedronVerticesNum;
            facesNum = octahedronFacesNum;
            vertices = octahedronVertices;
            faces3v = octahedronFaces;
            break;
        case DODECAHEDRON:
            verticesNum = dodecahedronVerticesNum;
            facesNum = dodecahedronFacesNum;
            vertices = dodecahedronVertices;
            faces5v = dodecahedronFaces;
            break;
        case ICOSAHEDRON:
            verticesNum = icosahedronVerticesNum;
            facesNum = icosahedronFacesNum;
            vertices = icosahedronVertices;
            faces3v = icosahedronFaces;
            break;
        default:
            break;
    }
    OBJ_3D *obj = obj_3d(verticesNum, facesNum);

    for (i=0; i < obj->vcnt; i++) {
        copy_v4(&obj->vertices[i].root, &(VEC_4){vertices[i][0]*a, vertices[i][1]*a, vertices[i][2]*a, 1});
    }

    if (type == DODECAHEDRON)
        for (i = 0; i < obj->fcnt; i++) {
            obj->faces[i].vcnt = 5;
            for (j = 0; j < 5; j++)
                obj->faces[i].vi[j] = faces5v[i][j];
            obj->faces[i].t[0].u = 0.9045;
            obj->faces[i].t[0].v = 0.5;
            obj->faces[i].t[1].u = 0.5590;
            obj->faces[i].t[1].v = 0.9755;
            obj->faces[i].t[2].u = 0.0;
            obj->faces[i].t[2].v = 0.7939;
            obj->faces[i].t[3].u = 0.0;
            obj->faces[i].t[3].v = 0.2061;
            obj->faces[i].t[4].u = 0.5590;
            obj->faces[i].t[4].v = 0.02447;
        }
    else if (type == CUBE)
        for (INT i=0; i < obj->fcnt; i++) {
            obj->faces[i].vcnt = 4;
            for (j = 0; j < 4; j++)
                obj->faces[i].vi[j] = faces4v[i][j];
            obj->faces[i].t[0].u = 0.0;
            obj->faces[i].t[0].v = 0.0;
            obj->faces[i].t[1].u = 1.0;
            obj->faces[i].t[1].v = 0.0;
            obj->faces[i].t[2].u = 1.0;
            obj->faces[i].t[2].v = 1.0;
            obj->faces[i].t[3].u = 0.0;
            obj->faces[i].t[3].v = 1.0;
        }
    else
        for (INT i=0; i < obj->fcnt; i++) {
            obj->faces[i].vcnt = 3;
            for (j = 0; j < 3; j++)
                obj->faces[i].vi[j] = faces3v[i][j];
            obj->faces[i].t[0].u = 0.0;
            obj->faces[i].t[0].v = 0.0;
            obj->faces[i].t[1].u = 1.0;
            obj->faces[i].t[1].v = 0.0;
            obj->faces[i].t[2].u = 0.5;
            obj->faces[i].t[2].v = 0.866;
        }
    obj_3d_init_geometry(obj);

    return obj;
}

/**
    uv - vertex count along axis u
    vv - vertex count along axis v
 */
OBJ_3D * obj_3d_uv_mesh(MESH_TYPE type, SURFACE_WRAP wrap_surface, INT uv, INT vv, INT ut, INT vt) {
    INT u = 0, v = 0, up = 0, vp = 0; //U poly index, V poly index, U poly count, V poly count
    INT A = 0, B = 0, C = 0, D = 0, F = 0; //vertex and face indices
    INT VI = 0; //general purpose vertex index
    bool wrap_map_u = false, wrap_map_v = false; //wrap texture along U/V axis flags
    OBJ_3D *obj = NULL;

    //Vertex counts along U/V axis should be even when surface wrapping is used,
    //otherwise they should be odd, when surface wrapping is not used.
    if ((uv&1) == 1) {
        if (wrap_surface == WRAP_U || wrap_surface == WRAP_UV) uv++;
    }
    else {
        if (wrap_surface == NO_WRAP) uv++;
    }

    if ((vv&1) == 1) {
        if (wrap_surface == WRAP_UV) vv++;
    }
    else {
        if (wrap_surface == NO_WRAP || wrap_surface == WRAP_U) vv++;
    }


    //Counts of polygons in the mesh along axes
    up = wrap_surface != NO_WRAP ? uv : uv - 1; //count of polygons along u axis
    vp = wrap_surface == WRAP_UV ? vv : vv - 1; //count of polygons along v axis

    if (type == QUAD) {
        //allocate object
        obj = obj_3d(uv*vv, up*vp);
    }
    else //if (type == RIGHT_TRIANGLE || type == EQUILATERAL_TRIANGLE || type == WIDE_TRIANGLE)
    {
        //allocate object
        obj = obj_3d(uv*vv, 2*up*vp);
    }

    FLOAT v_v_adj = 0.; //vertex v adjustment
    if (type == EQUILATERAL_TRIANGLE || type == WIDE_TRIANGLE) {
        v_v_adj = 0.5;
    }

    //A, B, C, D: Indices for polygon vertices 
    //store U/V coord offsets later used for suface wrapping
    //wrap texture along U axis if needed
    //wrap texture along V axis if needed
    #define GET_VERTEX_INDICES {\
        A = u*vv+v;\
        B = ((u+1)%uv)*vv+v;\
        C = ((u+1)%uv)*vv+((v+1)%vv);\
        D = u*vv+((v+1)%vv);\
        wrap_map_u = (u+1)/(uv/ut) != u/(uv/ut) ? true : false;\
        wrap_map_v = (v+1)/(vv/vt) != v/(vv/vt) ? true : false; }

    if (type == QUAD) {
        // Assign vertex indexes to faces. Keep left-hand orientation.
        //dont wrap around any polygons
        for (u = 0; u < up; u++) {
            for (v = 0; v < vp; v++) {
                F = u*vp+v;
                obj->faces[F].vcnt = 4;
                GET_VERTEX_INDICES;
                obj->faces[F].vi[0] = A;
                obj->faces[F].vi[1] = B;
                obj->faces[F].vi[2] = C;
                obj->faces[F].vi[3] = D;
                obj->faces[F].bc[0].u = 0;
                obj->faces[F].bc[0].v = 0;
                obj->faces[F].bc[1].u = wrap_map_u ? 1 : 0;
                obj->faces[F].bc[1].v = 0;
                obj->faces[F].bc[2].u = wrap_map_u ? 1 : 0;
                obj->faces[F].bc[2].v = wrap_map_v ? 1 : 0;
                obj->faces[F].bc[3].u = 0;
                obj->faces[F].bc[3].v = wrap_map_v ? 1 : 0;
            }
        }
    }
    else { //type == RIGHT_TRIANGLE || type == EQUILATERAL_TRIANGLE || type == WIDE_TRIANGLE
        // Assign vertex indexes to faces. Keep left-hand orientation.
        //dont wrap around any polygons
        bool switch_orientation = false;
        for (u = 0; u < up; u++) {
            if (type == EQUILATERAL_TRIANGLE || type == WIDE_TRIANGLE) {
                switch_orientation = false;
                if (u&1) {
                    if (type == EQUILATERAL_TRIANGLE)
                        switch_orientation = true;
                }
                else {
                    if (type == WIDE_TRIANGLE)
                        switch_orientation = true;
                }
            }
            for (v = 0; v < vp; v++) {
                if (type == RIGHT_TRIANGLE)
                    switch_orientation = (bool)((u^v)&1);
                F = 2*(u*vp+v);
                obj->faces[F].vcnt = 3;
                obj->faces[F+1].vcnt = 3;
                GET_VERTEX_INDICES;
                if (switch_orientation) {
                    obj->faces[F].vi[0] = A;
                    obj->faces[F].vi[1] = B;
                    obj->faces[F].vi[2] = C;
                    obj->faces[F].bc[0].u = 0;
                    obj->faces[F].bc[0].v = 0;
                    obj->faces[F].bc[1].u = wrap_map_u ? 1 : 0;
                    obj->faces[F].bc[1].v = 0;
                    obj->faces[F].bc[2].u = wrap_map_u ? 1 : 0;
                    obj->faces[F].bc[2].v = wrap_map_v ? 1 : 0;
                    obj->faces[F+1].vi[0] = A;
                    obj->faces[F+1].vi[1] = C;
                    obj->faces[F+1].vi[2] = D;
                    obj->faces[F+1].bc[0].u = 0;
                    obj->faces[F+1].bc[0].v = 0;
                    obj->faces[F+1].bc[1].u = wrap_map_u ? 1 : 0;
                    obj->faces[F+1].bc[1].v = wrap_map_v ? 1 : 0;
                    obj->faces[F+1].bc[2].u = 0;
                    obj->faces[F+1].bc[2].v = wrap_map_v ? 1 : 0;
                }
                else {
                    obj->faces[F].vi[0] = A;
                    obj->faces[F].vi[1] = B;
                    obj->faces[F].vi[2] = D;
                    obj->faces[F].bc[0].u = 0;
                    obj->faces[F].bc[0].v = 0;
                    obj->faces[F].bc[1].u = wrap_map_u ? 1 : 0;
                    obj->faces[F].bc[1].v = 0;
                    obj->faces[F].bc[2].u = 0;
                    obj->faces[F].bc[2].v = wrap_map_v ? 1 : 0;
                    obj->faces[F+1].vi[0] = B;
                    obj->faces[F+1].vi[1] = C;
                    obj->faces[F+1].vi[2] = D;
                    obj->faces[F+1].bc[0].u = wrap_map_u ? 1 : 0;
                    obj->faces[F+1].bc[0].v = 0;
                    obj->faces[F+1].bc[1].u = wrap_map_u ? 1 : 0;
                    obj->faces[F+1].bc[1].v = wrap_map_v ? 1 : 0;
                    obj->faces[F+1].bc[2].u = 0;
                    obj->faces[F+1].bc[2].v = wrap_map_v ? 1 : 0;
                }
            }
        }
    }

    //for each vertex create temporary uv coordinates
    for (u = 0, VI = 0; u < uv; u++) {
        for (v = 0; v < vv; v++, VI++) {
            obj->vertex_s[VI].u = 
                (FLOAT)u/(FLOAT)(up);
            obj->vertex_s[VI].v = 
                ((FLOAT)v+(FLOAT)(u&1)*v_v_adj)/(FLOAT)(vp);
        }
    }

    //build texture map uv coordinates from vertex uv coordinates
    for (INT i = 0; i < obj->fcnt; i++) {
        for (INT j = 0; j < obj->faces[i].vcnt; j++) {
            VI = obj->faces[i].vi[j];
            //Account for faces that are crossing map seam.
            //This is a measure for objects with map wrapping.
            obj->faces[i].t[j].u = FMOD(obj->vertex_s[VI].u*ut, (FLOAT)1.0) + obj->faces[i].bc[j].u;
            obj->faces[i].t[j].v = FMOD(obj->vertex_s[VI].v*vt, (FLOAT)1.0) + obj->faces[i].bc[j].v;
        }
    }

    //Build initial stub/flat surface
    obj_3d_uv_surface_parametric(obj, stub_surface, 1.0, 1.0, 1.0);
    return obj;
}

/**
    su - scaling factor for output object along U axis
    sv - scaling factor for output object along V axis
    sw - scaling factor for output object along W axis
 */
void obj_3d_uv_surface_parametric(OBJ_3D *obj, PARAMETRIC_SURFACE surf_func, FLOAT su, FLOAT sv, FLOAT sw) {
    INT VI = 0; //vertex index
    //Based on the UV map coordinates from vertices, build the actual geometry of the surface,
    //and scale the surface geometry to the requested ranges.
    //Store the calculated values in the object vertices.
    for (VI = 0; VI < obj->vcnt; VI++) {
        copy_v4(&obj->vertices[VI].root, surf_func(obj->vertex_s[VI].u, obj->vertex_s[VI].v));
        obj->vertices[VI].root[0] *= su;
        obj->vertices[VI].root[1] *= sv;
        obj->vertices[VI].root[2] *= sw;
    }
    obj_3d_init_geometry(obj);
}

void obj_3d_uv_surface_cycloid(OBJ_3D *obj, CYCLOID *cld, FLOAT su, FLOAT sv, FLOAT sw) {
    for (INT VI = 0; VI < obj->vcnt; VI++) {
        copy_v4(&obj->vertices[VI].root, cycloid_surface(cld, obj->vertex_s[VI].u, obj->vertex_s[VI].v));
        obj->vertices[VI].root[0] *= su;
        obj->vertices[VI].root[1] *= sv;
        obj->vertices[VI].root[2] *= sw;
    }
    obj_3d_init_geometry(obj);
}

OBJ_3D * obj_3d_toroid(MESH_TYPE type, FLOAT R1, FLOAT R2, INT uv, INT vv, INT ut, INT vt) {
    surf_params[0] = R1/(R1+R2);
    surf_params[1] = R2/(R1+R2);

    OBJ_3D *obj = obj_3d_uv_mesh(type, WRAP_UV, uv, vv, ut, vt);
    obj_3d_uv_surface_parametric(obj, toroid_uv, R1+R2, R1+R2, R1+R2);
    return obj;
}


//Internal functions
VEC_4* stub_surface(FLOAT u, FLOAT v) {
    return store_v4(&(VEC_4){u-0.5, v-0.5, 0.0, 1.0});
}

FLOAT cycle_function(CYCLE_COMPONENT *c, FLOAT u) {
    FLOAT v = 0.0;
    for (INT i = 0; i < c->cnt; i++)
        v += c->amp[i]*sin(c->frq[i]*u+c->phs[i]);
    return v;
}
FLOAT cycle_tangent(CYCLE_COMPONENT *c, FLOAT u) {
    FLOAT v = 0.0;
    for (INT i = 0; i < c->cnt; i++)
        v += c->amp[i]*c->frq[i]*sin(c->frq[i]*u+c->phs[i]+PI_2);
    return v;
}

VEC_4* cycloid_surface(CYCLOID *cld, FLOAT u, FLOAT v) {
    u *= 2*PI;
    v *= 2*PI;

    VEC_4 R = {0.0, 1.0, 0.0, 1.0};
    VEC_4 W, T;
    W[0] = cycle_function(&cld->x, u);    T[0] = cycle_tangent(&cld->x, u);
    W[1] = cycle_function(&cld->y, u);    T[1] = cycle_tangent(&cld->y, u);
    W[2] = cycle_function(&cld->z, u);    T[2] = cycle_tangent(&cld->z, u);
    W[3] = 1.0;    T[3] = 1.0;
    VEC_4 *TN = norm_v((VEC_4*)T);
    VEC_4 *S = cross_vv((VEC_4*)R, TN); //othogonal vector to RT plane
    //slice cross-section is located in the plane of the vectors R and S

    return add_vv(add_vv(mul_vd((VEC_4*)R, cld->phi*cos(v)), mul_vd(S, cld->phi*-sin(v))), (VEC_4*)W);
}

VEC_4* toroid_uv(FLOAT u, FLOAT v) {
    u *= 2*PI;
    v *= 2*PI;
    FLOAT x = (surf_params[1]*sin(v) + surf_params[0])*sin(u);
    FLOAT z = (surf_params[1]*sin(v) + surf_params[0])*cos(u);
    FLOAT y =  surf_params[1]*cos(v);
    return store_v4(&(VEC_4){x, y, z, 1.0});
}
