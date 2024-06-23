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

#define VEC_4_CNT (64) //power of 2
VEC_4 *vec_4_buf;
static INT vec_4_idx = 0;

#define MAT_4_4_CNT (64) //power of 2
MAT_4_4 *mat_4_4_buf;
static INT mat_4_4_idx = 0;

#define A (*a)
#define B (*b)
#define C (*c)
#define V (vec_4_buf[vec_4_idx])
#define M (mat_4_4_buf[mat_4_4_idx])

void vr_geometry_init() {
    vec_4_buf = calloc(VEC_4_CNT, sizeof(VEC_4));
    mat_4_4_buf = calloc(MAT_4_4_CNT, sizeof(MAT_4_4));
}

void vr_geometry_cleanup() {
    free(vec_4_buf);
    free(mat_4_4_buf);
}

void inc_vec_4_index() {
    vec_4_idx = (vec_4_idx + 1) & (VEC_4_CNT-1);
}
void inc_mat_4_4_index() {
    mat_4_4_idx = (mat_4_4_idx + 1) & (MAT_4_4_CNT-1);
}

VEC_4 *store_v4(VEC_4 *src) {
    inc_vec_4_index();
    for(INT i=0; i<4; i++)
        V[i] = (*src)[i];
    return &V;
}

MAT_4_4 *store_m(MAT_4_4 *src) {
    inc_mat_4_4_index();
    for(INT i=0; i<4; i++)
        for(INT j=0; j<4; j++)
            M[i][j] = (*src)[i][j];
    return &M;
}

VEC_4 *copy_v4(VEC_4 *dst, VEC_4 *src) {
    for(INT i=0; i<4; i++)
        (*dst)[i] = (*src)[i];
    return dst;
}

MAT_4_4 *copy_m(MAT_4_4 *dst, MAT_4_4 *src) {
    for(INT i=0; i<4; i++)
        for(INT j=0; j<4; j++)
            (*dst)[i][j] = (*src)[i][j];
    return dst;
}

VEC_4 *add_vv(VEC_4 *a, VEC_4 *b) {
    inc_vec_4_index();
    V[0] = A[0]+B[0];
    V[1] = A[1]+B[1];
    V[2] = A[2]+B[2];
    V[3] = 1.0;
    return &V;
}

VEC_4 *sub_vv(VEC_4 *a, VEC_4 *b) {
    inc_vec_4_index();
    V[0] = A[0]-B[0];
    V[1] = A[1]-B[1];
    V[2] = A[2]-B[2];
    V[3] = 1.0;
    return &V;
}

VEC_4 *mul_vd(VEC_4 *a, FLOAT b) {
    inc_vec_4_index();
    V[0] = A[0]*b;
    V[1] = A[1]*b;
    V[2] = A[2]*b;
    V[3] = 1.0;
    return &V;
}

VEC_4 *div_vd(VEC_4 *a, FLOAT b) {
    inc_vec_4_index();
    V[0] = A[0]/b;
    V[1] = A[1]/b;
    V[2] = A[2]/b;
    V[3] = 1.0;
    return &V;
}

VEC_4 *cross_vv(VEC_4 *a, VEC_4 *b) {
    inc_vec_4_index();
    V[0] =   A[1]*B[2]-A[2]*B[1];
    V[1] = -(A[0]*B[2]-A[2]*B[0]);
    V[2] =   A[0]*B[1]-A[1]*B[0];
    V[3] = 1.0;
    return &V;
}

FLOAT dot_vv(VEC_4 *a, VEC_4 *b) {
    FLOAT dp = 0.0;
    for (INT i=0; i<3; i++)
        dp += A[i]*B[i];
    return dp;
}

FLOAT length_v(VEC_4 *a) {
    return sqrt(A[0]*A[0] + A[1]*A[1] + A[2]*A[2]);
}

VEC_4 *norm_v(VEC_4 *a) {
    inc_vec_4_index();
    FLOAT v_len = length_v(a);
    V[0] = A[0]/v_len;
    V[1] = A[1]/v_len;
    V[2] = A[2]/v_len;
    V[3] = 1.0;

    return &V;
}

VEC_4 *mul_mv(MAT_4_4 *a, VEC_4 *b) {
    inc_vec_4_index();
    for (INT i=0; i<4; i++) {
        V[i] = 0.;
        for (INT j=0; j<4; j++) {
            V[i] += A[i][j] * B[j];
        }
    }
    return &V;
}

MAT_4_4 *mul_mm(MAT_4_4 *a, MAT_4_4 *b) {
    inc_mat_4_4_index();
    for (INT i=0; i<4; i++) {
        for (INT j=0; j<4; j++) {
            M[i][j] = 0.0;
            for (INT k=0; k<4; k++) {
                M[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return &M;
}

MAT_4_4 *scale_m(FLOAT scale_x, FLOAT scale_y, FLOAT scale_z) {
    return
        store_m(&(MAT_4_4){
            { scale_x,     0.0,     0.0, 0.0 },
            {     0.0, scale_y,     0.0, 0.0 },
            {     0.0,     0.0, scale_z, 0.0 },
            {     0.0,     0.0,     0.0, 1.0 }}
        );
}

MAT_4_4 *transform_m(FLOAT ang_x, FLOAT ang_y, FLOAT ang_z, FLOAT dx, FLOAT dy, FLOAT dz, FLOAT s_x, FLOAT s_y, FLOAT s_z) {
    FLOAT cx, cy, cz; // cosinus of ang_x, ang_y, ang_z
    FLOAT sx, sy, sz; // sinus of ang_x, ang_y, ang_z 
    ang_x = PI*ang_x/180.0;
    ang_y = PI*ang_y/180.0;
    ang_z = PI*ang_z/180.0;
    cx = cos(ang_x); cy = cos(ang_y); cz = cos(ang_z);
    sx = sin(ang_x); sy = sin(ang_y); sz = sin(ang_z);
    return
        store_m(&(MAT_4_4){
            {           cy*cz*s_x,          -cy*sz*s_x,     sy*s_x,  dx},
            {  sx*sy*cz+cx*sz*s_y, -sx*sy*sz+cx*cz*s_y, -sx*cy*s_y,  dy},
            { -cx*sy*cz+sx*sz*s_z,  cx*sy*sz+sx*cz*s_z,  cx*cy*s_z,  dz},
            {                 0.0,                 0.0,        0.0, 1.0}}
        );
}

MAT_4_4 *camera_m(VEC_4 *AT, VEC_4 *EYE, FLOAT roll) {
    VEC_4 *X, *Y, *Z;
    VEC_4 UP = {0.0, 1.0, 0.0, 0.0};

    // Calculate Z camera axis
    Z = norm_v(sub_vv(AT, EYE));

    // Calculate X camera axis
    X = norm_v(cross_vv(Z, &UP));

    // Calculate Y camera axis
    Y = cross_vv(X, Z);

    return
        mul_mm(
            transform_m(0.0, 0.0, roll, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0),
            store_m(&(MAT_4_4){
                { (*X)[0], (*X)[1], (*X)[2], -dot_vv(X, EYE)},
                { (*Y)[0], (*Y)[1], (*Y)[2], -dot_vv(Y, EYE)},
                { (*Z)[0], (*Z)[1], (*Z)[2], -dot_vv(Z, EYE)},
                {       0,       0,       0,             1.0}})
        );
}

MAT_4_4 *projection_m(FLOAT fov, FLOAT w, FLOAT h, FLOAT n, FLOAT f) {
    FLOAT tan_h = tan(PI*fov/360.0); //tangent of fov/2
    FLOAT ar = (FLOAT)w/(FLOAT)h;
    FLOAT w_h = w/2.0;
    FLOAT h_h = h/2.0;
    return
        store_m(&(MAT_4_4){
            { w_h/tan_h,          0.0,          0.0,           0.0 },
            {       0.0, h_h*ar/tan_h,          0.0,           0.0 },
            {       0.0,          0.0,    1.0/(f-n),      -n/(f-n) }, //frustum Z rescaling to [0.0, 1.0]
            {       0.0,          0.0,          1.0,           0.0 }} //perspective Z division
        );
}
