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

#include <math.h>
#include <stdlib.h>

#include "engine_types.h"
#include "v_geometry.h"
#include "v_obj_3d.h"
#include "v_lighting.h"

OBJ_3D_CONTAINER * obj_3d_container(OBJ_3D *obj, INT max_children) {
    OBJ_3D_CONTAINER *cont = calloc(1, sizeof(OBJ_3D_CONTAINER));
    cont->child = calloc(max_children, sizeof(OBJ_3D_CONTAINER*));
    cont->child_cnt = 0;
    cont->parent = NULL;
    cont->obj = obj;
    cont->obj_owned = (obj != NULL) ? true : false;
    cont->ax = 0.0;
    cont->ay = 0.0;
    cont->az = 0.0;
    cont->px = 0.0;
    cont->py = 0.0;
    cont->pz = 0.0;
    copy_m(&cont->matrix, scale_m(1.0, 1.0, 1.0));
    return cont;
}

void obj_3d_container_free(OBJ_3D_CONTAINER *cont) {
    if (cont == NULL) return;
    for (INT i = 0; i < cont->child_cnt; i++) {
        obj_3d_container_free(cont->child[i]);
    }
    if (cont->obj_owned) {
        obj_3d_free(cont->obj);
    }
    free(cont->child);
    free(cont);
}

void obj_3d_container_set_transform(OBJ_3D_CONTAINER *cont, FLOAT ax, FLOAT ay, FLOAT az, FLOAT px, FLOAT py, FLOAT pz) {
    cont->ax = ax;
    cont->ay = ay;
    cont->az = az;
    cont->px = px;
    cont->py = py;
    cont->pz = pz;
}

void obj_3d_container_calc_matrices(OBJ_3D_CONTAINER *cont) {
    MAT_4_4* parent_matrix;
    if (cont->parent == NULL)
        parent_matrix = scale_m(1.0, -1.0, 1.0); //Invert Y axis to direct it upwards
    else
        parent_matrix = &cont->parent->matrix;

    copy_m(
        &cont->matrix,
        mul_mm(
            parent_matrix,
            transform_m(cont->ax, cont->ay, cont->az, cont->px, cont->py, cont->pz)
        )
    );

    for (INT i = 0; i < cont->child_cnt; i++)
        obj_3d_container_calc_matrices(cont->child[i]);
}

void obj_3d_container_transform_geometry(OBJ_3D_CONTAINER *cont, MAT_4_4 *camera_space, MAT_4_4 *projection_space, INT scr_w, INT scr_h) {

    OBJ_3D* obj = cont->obj;
    MAT_4_4 camera_transform;
    MAT_4_4 projection_transform;
    FACE *face;
    INT rotate_vertex_normals = 
        obj->type == INTERP_DIFF || 
        obj->type == INTERP_SPEC || 
        obj->type == INTERP_DIFF_SPEC || 
        obj->type == INTERP_DIFF_TEXTURED || 
        obj->type == INTERP_SPEC_TEXTURED || 
        obj->type == INTERP_DIFF_SPEC_TEXTURED || 
        obj->type == TX_MAP_BASE_MUL || 
        obj->type == TX_MAP_BASE_ADD || 
        obj->type == TX_MAP_BASE_MUL_ADD || 
        obj->type == TX_MAP_BUMP_REFLECTION ||
        obj->type == REFLECTION ||
        cont->scene->rotate_all_objects_vertex_normals;
    copy_m(&camera_transform, mul_mm(camera_space, &cont->matrix));
    copy_m(&projection_transform, mul_mm(projection_space, &camera_transform));

    //Vertex transform to camera space
    for(INT i=0; i<obj->vcnt; i++) {
        copy_v4(&obj->vertices[i].camera, mul_mv(&camera_transform, &obj->vertices[i].root));
    }

    if (obj->type == POINT_LIGHTS || obj->type == PARTICLES) {
        for(INT i=0; i<obj->vcnt; i++) {
            obj->vertices[i].front = true;
        }
    }
    else {
        for(INT i=0; i<obj->vcnt; i++) {
            obj->vertices[i].front = false;
        }
        //Transform zero point to camera space
        copy_v4(&obj->zero_camera, mul_mv(&camera_transform, &(VEC_4){0.0, 0.0, 0.0, 1.0}));
        //Normals rotation and back face occlusion
        obj->front_fcnt = 0;
        for (INT i = 0; i < obj->fcnt; i++) {
            face = obj->faces + i;
            copy_v4(&face->normal_camera,
                sub_vv(mul_mv(&camera_transform, &face->normal_root), &obj->zero_camera));
            if (dot_vv(&obj->vertices[face->vi[1]].camera, &face->normal_camera) > 0.0) {
                obj->front_faces[obj->front_fcnt++] = obj->faces + i;
                for (INT j = 0; j < face->vcnt; j++)
                    obj->vertices[face->vi[j]].front = true; //record that each vertex of the face is visibile
            }
        }
    }

    //Vertex transform to camera space and perspective projection
    VEC_4 *c;
    for(INT i = 0; i < obj->vcnt; i++) {
        if (obj->vertices[i].front) {
            if (rotate_vertex_normals) {
                copy_v4(&obj->vertices[i].normal_camera,
                    sub_vv(mul_mv(&camera_transform, &obj->vertices[i].normal_root), &obj->zero_camera));
            }
            c = mul_mv(&projection_transform, &obj->vertices[i].root);
            //X perspective division
            obj->vertices[i].projection[0] = (*c)[0]/(*c)[3] + scr_w/2.0;
            //Y perspective division, inverse back Y axis
            obj->vertices[i].projection[1] = (*c)[1]/(*c)[3] + scr_h/2.0;
            //TODO frustum Z occlusion should go here?
            //rescale frustum Z value to Z-buffer space [0, zbuf_max]
            obj->vertices[i].projection[2] = (FLOAT)Z_BUFFER_MAX * (*c)[2];
        }
    }
}
void obj_3d_container_apply_light(OBJ_3D_CONTAINER *cont) {
    switch (cont->obj->type) {
        case SOLID_DIFF_TEXTURED:
        case SOLID_SPEC_TEXTURED:
        case SOLID_DIFF_SPEC_TEXTURED:
            lighting_face_calculation(cont->scene, cont->obj);
            break;
        case SOLID_DIFF:
            lighting_face_calculation(cont->scene, cont->obj);
            lighting_face_coloring_diffuse(cont->obj);
            break;
        case SOLID_SPEC:
            lighting_face_calculation(cont->scene, cont->obj);
            lighting_face_coloring_specular(cont->obj);
            break;
        case SOLID_DIFF_SPEC:
            lighting_face_calculation(cont->scene, cont->obj);
            lighting_face_coloring_merge(cont->obj);
            break;
        case INTERP_DIFF:
            lighting_vertex_calculation(cont->scene, cont->obj);
            lighting_vertex_coloring_diffuse(cont->obj);
            break;
        case INTERP_SPEC:
            lighting_vertex_calculation(cont->scene, cont->obj);
            lighting_vertex_coloring_specular(cont->obj);
            break;
        case INTERP_DIFF_SPEC:
            lighting_vertex_calculation(cont->scene, cont->obj);
            lighting_vertex_coloring_merge(cont->obj);
            break;
        case INTERP_DIFF_TEXTURED:
        case INTERP_SPEC_TEXTURED:
        case INTERP_DIFF_SPEC_TEXTURED:
            lighting_vertex_calculation(cont->scene, cont->obj);
            break;
        default:
            break;
    }
}

void obj_3d_container_render(OBJ_3D_CONTAINER *cont) {
    switch (cont->obj->type) {
        case SOLID_UNSHADED:
            obj_3d_draw_solid_unshaded(cont->obj);
            break;
        case SOLID_DIFF:
        case SOLID_SPEC:
        case SOLID_DIFF_SPEC:
            obj_3d_draw_solid_shaded(cont->obj);
            break;
        case INTERP_UNSHADED:
            obj_3d_draw_interp_unshaded(cont->obj);
            break;
        case INTERP_DIFF:
        case INTERP_SPEC:
        case INTERP_DIFF_SPEC:
            obj_3d_draw_interp_shaded(cont->obj);
            break;
        case SOLID_DIFF_TEXTURED:
            obj_3d_draw_solid_diff_textured(cont->obj);
            break;
        case SOLID_SPEC_TEXTURED:
            obj_3d_draw_solid_spec_textured(cont->obj);
            break;
        case SOLID_DIFF_SPEC_TEXTURED:
            obj_3d_draw_solid_diff_spec_textured(cont->obj);
            break;
        case INTERP_DIFF_TEXTURED:
            obj_3d_draw_interp_diff_textured(cont->obj);
            break;
        case INTERP_SPEC_TEXTURED:
            obj_3d_draw_interp_spec_textured(cont->obj);
            break;
        case INTERP_DIFF_SPEC_TEXTURED:
            obj_3d_draw_interp_diff_spec_textured(cont->obj);
            break;
        case TX_MAP_BASE:
            obj_3d_draw_textured_base(cont->obj);
            break;
        case TX_MAP_BASE_MUL:
            obj_3d_draw_textured_base_mul(cont->obj);
            break;
        case TX_MAP_BASE_ADD:
            obj_3d_draw_textured_base_add(cont->obj);
            break;
        case TX_MAP_BASE_MUL_ADD:
            obj_3d_draw_textured_base_mul_add(cont->obj);
            break;
        case REFLECTION:
            obj_3d_draw_fake_reflection(cont->obj);
            break;
        case TX_MAP_BUMP_REFLECTION:
            obj_3d_draw_bump_fake_reflection(cont->obj);
            break;
        default:
            break;
    }

    if (cont->obj->wireframe_on)
        obj_3d_draw_wireframe(cont->obj);
}
