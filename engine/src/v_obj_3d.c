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
#include <string.h>

#include "engine_types.h"
#include "v_renderer.h"
#include "v_geometry.h"

//Internal functions forward declarations
void add_adjacent_vertex_index(VERTEX *v, INT avi);

OBJ_3D *obj_3d(INT vcnt, INT fcnt) {
    OBJ_3D *obj = calloc(1, sizeof(OBJ_3D));

    *obj = (OBJ_3D){
        .wireframe_on = false,
        .fcnt = fcnt,
        .faces = calloc(fcnt, sizeof(FACE)),
        .front_fcnt = 0,
        .front_faces = calloc(fcnt, sizeof(FACE*)),
        .vcnt = vcnt,
        .vertices = calloc(vcnt, sizeof(VERTEX)),
        .vertex_s = calloc(vcnt, sizeof(SURFACE_COORD)),
        .type = HIDDEN,
        .base_map = NULL,
        .bump_map = NULL,
        .mul_map = NULL,
        .add_map = NULL,
        .reflection_map = NULL
    };

    return obj;
}

OBJ_3D *obj_3d_copy(OBJ_3D *src) {
    OBJ_3D *obj = calloc(1, sizeof(OBJ_3D));

    memcpy(obj, src, sizeof(OBJ_3D));

    obj->faces = calloc(obj->fcnt, sizeof(FACE));
    obj->front_faces = calloc(obj->fcnt, sizeof(FACE*));
    obj->vertices = calloc(obj->vcnt, sizeof(VERTEX));
    obj->vertex_s = calloc(obj->vcnt, sizeof(SURFACE_COORD));

    memcpy(obj->faces, src->faces, sizeof(FACE)*obj->fcnt);
    memcpy(obj->front_faces, src->front_faces, sizeof(FACE*)*obj->front_fcnt);
    memcpy(obj->vertices, src->vertices, sizeof(VERTEX)*obj->vcnt);
    memcpy(obj->vertex_s, src->vertex_s, sizeof(SURFACE_COORD)*obj->vcnt);

    return obj;
}

void obj_3d_free(OBJ_3D *obj) {
    free(obj->faces);
    free(obj->front_faces);
    free(obj->vertices);
    free(obj->vertex_s);
    free(obj);
}

/*
members needed in props:
color, wireframe_color, type, wireframe_on, specular_power, base_map, reflection_map
*/
void obj_3d_set_properties(OBJ_3D *obj, OBJ_3D *props) {
    INT i = 0, j = 0;
    copy_v3(&obj->wireframe_color, &props->wireframe_color);
    copy_v3(&obj->surface_color, &props->surface_color);
    for (i = 0; i < obj->fcnt; i++) {
        copy_v3(&obj->faces[i].color_surf, &props->surface_color);
    }
    for (i = 0; i < obj->vcnt; i++) {
        copy_v3(&obj->vertices[i].color_surf, &props->surface_color);
    }

    obj->type = props->type;
    if (props->wireframe_on == 0)
        obj->wireframe_on = false;
    else
        obj->wireframe_on = props->wireframe_on;
    obj->specular_power = props->specular_power;

    //If user didn't specified base_map or reflection_map in props,
    //(therefore these values implicitly became 0):
    //Assure that obj->base_map and obj->reflection_map will be always valid or NULL,
    obj->base_map = props->base_map == 0 ? NULL : props->base_map;
    obj->bump_map = props->bump_map == 0 ? NULL : props->bump_map;
    obj->mul_map = props->mul_map == 0 ? NULL : props->mul_map;
    obj->add_map = props->add_map == 0 ? NULL : props->add_map;
    obj->reflection_map = props->reflection_map == 0 ? NULL : props->reflection_map;

    /* Convert texture map coordinates from relative scale [0., 1.] to map scale [0, map_width/height] */
    if (obj->base_map != NULL) {
        for (i = 0; i < obj->fcnt; i++) {
            FACE *face = obj->faces + i;
            for (j = 0; j < face->vcnt; j++) {
                face->bc[j].u = (face->t[j].u * (obj->base_map->width-1));
                if (face->bc[j].u > obj->base_map->width-1)
                    face->bc[j].u = obj->base_map->width-1;
                face->bc[j].v = (face->t[j].v * (obj->base_map->height-1));
                if (face->bc[j].v > obj->base_map->height_with_margin-1)
                    face->bc[j].v = obj->base_map->height_with_margin-1;
            }
        }
    }
}

void obj_3d_determine_edges(OBJ_3D *obj) {
    //based on all edges in every face, add to every vertex
    //its adjacent(neighbour) vertices
    INT i = 0, j = 0, v1 = 0, v2 = 0;

    for (i = 0; i < obj->vcnt; i++) {
        obj->vertices[i].avcnt = 0;
    }

    //for every face...
    for (i = 0; i < obj->fcnt; i++) {
        for (j = 0; j < obj->faces[i].vcnt; j++) {
            //iterate over every edge of that face...
            v1 = obj->faces[i].vi[j];
            v2 = obj->faces[i].vi[(j+1) % obj->faces[i].vcnt];
            //and to each ending vertex in that edge
            //add the other vertex as adjacent
            add_adjacent_vertex_index(&obj->vertices[v1], v2);
            add_adjacent_vertex_index(&obj->vertices[v2], v1);
        }
    }
}

void obj_3d_calc_face_normals(OBJ_3D *obj) {
    VERTEX *vertices = obj->vertices;
    for (INT i=0; i<obj->fcnt; i++) {
        FACE *face = obj->faces+i;
        copy_v4(&obj->faces[i].normal_root, norm_v(cross_vv(
            sub_vv(&vertices[face->vi[2]].root, &vertices[face->vi[1]].root),
            sub_vv(&vertices[face->vi[0]].root, &vertices[face->vi[1]].root))));
    }
}

void obj_3d_calc_vertex_normals(OBJ_3D *obj) {
    INT i = 0, j = 0;
    FACE *face = NULL;
    for (i = 0; i < obj->vcnt; i++) {
        copy_v4(
            &obj->vertices[i].normal_root,
            &(VEC_4){0.0, 0.0, 0.0, 0.0});
    }

    for (i = 0; i < obj->fcnt; i++) {
        face = obj->faces + i;
        for (j = 0; j < face->vcnt; j++) {
            copy_v4(
                &obj->vertices[face->vi[j]].normal_root,
                add_vv(&obj->vertices[face->vi[j]].normal_root, &face->normal_root));
        }
    }

    for (i = 0; i < obj->vcnt; i++) {
        copy_v4(
            &obj->vertices[i].normal_root,
            norm_v(&obj->vertices[i].normal_root));
    }
}

/**
 * Helper function
 * Performs all geometry-ralated object initializations
 */
void obj_3d_init_geometry(OBJ_3D *obj) {
    obj_3d_determine_edges(obj);
    obj_3d_calc_face_normals(obj);
    obj_3d_calc_vertex_normals(obj);
}

void obj_3d_draw_wireframe(OBJ_3D *obj) {
    INT i = 0, j = 0;
    VERTEX *v1, *v2;
    PROJECTION_COORD *v[2]; //endings of currently drawn line

    /* Move all vertices projections a bit forward along Z-axis, in order
       to prevent obstruction of their visibility by background in Z buffer.

       TODO: below loop permanently modifies vertices projections, which in the future
       might hamper some other operations on these vertices. Maybe it would be better to
       have something less invasive?
    */

    for (i=0; i<obj->vcnt; i++) {
        v1 = obj->vertices + i;
        if (v1->front) {
            (v1->projection)[2] -= Z_BUFFER_MAX/350;
        }
    }
    // For every front vertex draw a line to each of its adjacent front vertices
    for (i=0; i<obj->vcnt; i++) {
        v1 = obj->vertices + i;
        if (v1->front) {
            for (j=0; j<v1->avcnt; j++) {
                if (i < v1->avi[j]) {
                    v2 = obj->vertices + v1->avi[j];
                    if (v2->front) {
                        v[0] = (PROJECTION_COORD*)v1->projection;
                        v[1] = (PROJECTION_COORD*)v2->projection;
                        line_flat_z(v, (VEC_3*)obj->wireframe_color);
                    }
                }
            }
        }
    }
}

void obj_3d_draw_solid_unshaded(OBJ_3D *obj) {
    PROJECTION_COORD *v[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j=0; j<face->vcnt; j++)
            v[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        polygon_solid_z(face->vcnt, v, (VEC_3*)face->color_surf);
    }
}

void obj_3d_draw_solid_shaded(OBJ_3D *obj) {
    PROJECTION_COORD *v[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++)
            v[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        polygon_solid_z(face->vcnt, v, (VEC_3*)face->color_diff);
    }
}

void obj_3d_draw_interp_unshaded(OBJ_3D *obj) {
    FACE *face;
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    VEC_3 *vc[MAX_FACE_VERTICES];
    INT i, j;

    // Draw all the faces
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            vc[j] = (VEC_3*)obj->vertices[face->vi[j]].color_surf;
        }
        polygon_interp_z(face->vcnt, vp, vc);
    }
}

void obj_3d_draw_interp_shaded(OBJ_3D *obj) {
    FACE *face;
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    VEC_3 *vc[MAX_FACE_VERTICES];
    INT i, j;

    // Draw all the faces
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            vc[j] = (VEC_3*)obj->vertices[face->vi[j]].color_diff;
        }
        polygon_interp_z(face->vcnt, vp, vc);
    }
}

void obj_3d_draw_textured_base(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        }
        polygon_texture_base_z(face->vcnt, vp, face->bc, obj->base_map);
    }
}

void obj_3d_draw_textured_base_mul(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            face->rc[j].u = (obj->vertices[face->vi[j]].normal_camera[0] - 1.0) * -(obj->mul_map->width-1)/2.0;
            face->rc[j].v = (obj->vertices[face->vi[j]].normal_camera[1] - 1.0) * -(obj->mul_map->height-1)/2.0;
        }
        polygon_texture_base_mul_z(face->vcnt, vp, face->bc, obj->base_map, face->rc, obj->mul_map);
    }
}

void obj_3d_draw_textured_base_add(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            face->rc[j].u = (obj->vertices[face->vi[j]].normal_camera[0] - 1.0) * -(obj->add_map->width-1)/2.0;
            face->rc[j].v = (obj->vertices[face->vi[j]].normal_camera[1] - 1.0) * -(obj->add_map->height-1)/2.0;
        }
        polygon_texture_base_add_z(face->vcnt, vp, face->bc, obj->base_map, face->rc, obj->add_map);
    }
}

void obj_3d_draw_textured_base_mul_add(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;
    //Mul map and add map shall always have same dimensions. Dont draw otherwise.
    if (obj->mul_map->width != obj->add_map->width || obj->mul_map->height != obj->add_map->height)
        return;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            face->rc[j].u = (obj->vertices[face->vi[j]].normal_camera[0] - 1.0) * -(obj->mul_map->width-1)/2.0;
            face->rc[j].v = (obj->vertices[face->vi[j]].normal_camera[1] - 1.0) * -(obj->mul_map->height-1)/2.0;
        }
        polygon_texture_base_mul_add_z(face->vcnt, vp, face->bc, obj->base_map, face->rc, obj->mul_map, obj->add_map);
    }
}

void obj_3d_draw_fake_reflection(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            //Fetch vertex projection coordinates
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            //Find reflection surface map coordinates
            face->rc[j].u = (obj->vertices[face->vi[j]].normal_camera[0] - 1.0) * -(obj->reflection_map->width-1)/2.0;
            face->rc[j].v = (obj->vertices[face->vi[j]].normal_camera[1] - 1.0) * -(obj->reflection_map->height-1)/2.0;
        }
        polygon_texture_base_z(face->vcnt, vp, face->rc, obj->reflection_map);
    }
}

void obj_3d_draw_bump_fake_reflection(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            //Fetch vertex projection coordinates
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            //Find reflection surface map coordinates
            face->rc[j].u = (obj->vertices[face->vi[j]].normal_camera[0] * (0.5-obj->bump_map->margin) + 0.5) * obj->reflection_map->width;
            face->rc[j].v = (obj->vertices[face->vi[j]].normal_camera[1] * (0.5-obj->bump_map->margin) + 0.5) * obj->reflection_map->height;
        }
        polygon_texture_bump_z(face->vcnt, vp, face->bc, obj->bump_map, face->rc, obj->reflection_map);
    }
}

void obj_3d_draw_solid_diff_textured(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        }
        polygon_solid_diff_texture_z(face->vcnt, vp, (VEC_3*)face->color_diff, face->bc, obj->base_map);
    }
}

void obj_3d_draw_solid_spec_textured(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        }
        polygon_solid_spec_texture_z(face->vcnt, vp, (VEC_3*)face->color_spec, face->bc, obj->base_map);
    }
}

void obj_3d_draw_solid_diff_spec_textured(OBJ_3D *obj) {
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    FACE *face;
    INT i, j;

    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
        }
        polygon_solid_diff_spec_texture_z(face->vcnt, vp, (VEC_3*)face->color_diff, (VEC_3*)face->color_spec, face->bc, obj->base_map);
    }
}

void obj_3d_draw_interp_diff_textured(OBJ_3D *obj) {
    FACE *face;
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    VEC_3 *vdiff[MAX_FACE_VERTICES];
    INT i, j;

    // Draw all the faces
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            vdiff[j] = (VEC_3*)obj->vertices[face->vi[j]].color_diff;
        }
        polygon_interp_diff_texture_z(face->vcnt, vp, vdiff, face->bc, obj->base_map);
    }
}

void obj_3d_draw_interp_spec_textured(OBJ_3D *obj) {
    FACE *face;
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    VEC_3 *vspec[MAX_FACE_VERTICES];
    INT i, j;

    // Draw all the faces
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            vspec[j] = (VEC_3*)obj->vertices[face->vi[j]].color_spec;
        }
        polygon_interp_spec_texture_z(face->vcnt, vp, vspec, face->bc, obj->base_map);
    }
}

void obj_3d_draw_interp_diff_spec_textured(OBJ_3D *obj) {
    FACE *face;
    PROJECTION_COORD *vp[MAX_FACE_VERTICES]; //vertices of currently drawn face
    VEC_3 *vdiff[MAX_FACE_VERTICES];
    VEC_3 *vspec[MAX_FACE_VERTICES];
    INT i, j;

    // Draw all the faces
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j = 0; j < face->vcnt; j++) {
            vp[j] = (PROJECTION_COORD*)obj->vertices[face->vi[j]].projection;
            vdiff[j] = (VEC_3*)obj->vertices[face->vi[j]].color_diff;
            vspec[j] = (VEC_3*)obj->vertices[face->vi[j]].color_spec;
        }
        polygon_interp_diff_spec_texture_z(face->vcnt, vp, vdiff, vspec, face->bc, obj->base_map);
    }
}

//Internal functions
void add_adjacent_vertex_index(VERTEX *v, INT avi) {
    //check if avi is already present
    //in the adjacent vertices list
    for (INT i = 0; i < v->avcnt; i++) {
        if (v->avi[i] == avi) {
            return; //if present, then don't add avi
        }
    }
    v->avi[v->avcnt] = avi;
    v->avcnt++;
}
