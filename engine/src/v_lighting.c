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

void lighting_face_calculation(SCENE_3D *scene, OBJ_3D *obj) {
    VEC_4 *lv; //light vector
    VEC_4 ev; //eye vector
    FACE *face;
    VEC_4 face_center, *fc;
    FLOAT d, s;
    INT i, j;
    OBJ_3D_CONTAINER **lights = scene->light;
    INT light_cnt = scene->light_cnt;
    FLOAT lv_length; //light vector length
    FLOAT ldf; //light damping factor
    bool diffuse = true, specular = true;

    // Front faces lighting calculation first
    for (i=0; i<obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        face->color_diff = scene->light_settings.ambient;
        face->color_spec = (COLOR){.r = 0.0, .g = 0.0, .b = 0.0};

        // Determine face centre and eye vector(ev) from eye to face centre
        fc = &obj->vertices[face->vi[0]].camera;
        for (j=1; j < face->vcnt; j++)
            fc = add_vv(fc, &obj->vertices[face->vi[j]].camera);
        copy_v4(&face_center, div_vd(fc, face->vcnt));
        if (specular) {
            copy_v4(&ev, norm_v(&face_center));
        }

        // If directional light defined - calculate illumination from it
        if (scene->light_settings.directional.b > 0.0 ||
            scene->light_settings.directional.g > 0.0 ||
            scene->light_settings.directional.r > 0.0) {
            lv = (VEC_4*)scene->light_settings.direction_in_camera_space;

            d = dot_vv(&face->normal_camera, lv); //diffuse light factor
            if (specular) {
                s = dot_vv(sub_vv(mul_vd(&face->normal_camera, 2*d), lv), &ev); //specular light factor
            }
            if (diffuse && d > 0.0) {
                // Diffuse illumination color components
                face->color_diff = *COLOR_add(&face->color_diff,
                                              COLOR_scale(&scene->light_settings.directional, d));

            }
            if (specular && s > 0.0) {
                // Specular illumination color components
                s = pow(s, obj->specular_power);
                face->color_spec = *COLOR_add(&face->color_spec,
                                              COLOR_scale(&scene->light_settings.directional, s));
            }
        }

        // Calculate illumination from all point lights
        for (j = 0; j < light_cnt; j++) {
            // Calculate light vector
            lv = sub_vv(&face_center, &lights[j]->obj->vertices[0].camera);
            lv_length = length_v(lv);
            lv = div_vd(lv, lv_length);

            // Calculate light damping factor
            // TODO Consider other form of this factor, i.e. inverse square length
            // Maybe it should be lv_length+ev_length
            ldf = 1.0/(1.0 + scene->light_settings.attenuation*lv_length);

            d = dot_vv(&face->normal_camera, lv); //diffuse light factor
            if (specular) {
                s = dot_vv(sub_vv(mul_vd(&face->normal_camera, 2*d), lv), &ev); //specular light factor
            }
            if (diffuse && d > 0.0) {
                // Diffuse illumination color components
                d *= ldf;
                face->color_diff = *COLOR_add(&face->color_diff,
                                              COLOR_scale(&lights[j]->obj->vertices[0].color_surf, d));
            }
            if (specular && s > 0.0) {
                // Specular illumination color components
                s = ldf * pow(s, obj->specular_power);
                face->color_spec = *COLOR_add(&face->color_spec,
                                              COLOR_scale(&lights[j]->obj->vertices[0].color_surf, s));
            }
        }
    }
}

void lighting_vertex_calculation(SCENE_3D *scene, OBJ_3D *obj) {
    VERTEX *vertex;
    VEC_4 *lv, *ev; //light vector, eye vector, face normal
    FLOAT d, s;
    INT i, j;
    OBJ_3D_CONTAINER **lights = scene->light;
    INT light_cnt = scene->light_cnt;
    FLOAT lv_length; //light vector length
    FLOAT ldf; //light damping factor
    bool diffuse = true, specular = true;

    // All vertex lighting calculation first
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) { //calculate lighting only for front-facing vertices
            vertex->color_diff = scene->light_settings.ambient;
            vertex->color_spec = (COLOR){.r = 0.0, .g = 0.0, .b = 0.0};

            if (specular) {
                ev = norm_v(&vertex->camera); //eye vector
            }
            // If directional light defined - calculate illumination from it
            if (scene->light_settings.directional.r != 0.0 ||
                scene->light_settings.directional.g != 0.0 ||
                scene->light_settings.directional.b != 0.0) {
                lv = (VEC_4*)scene->light_settings.direction_in_camera_space;

                d = dot_vv(&vertex->normal_camera, lv); //diffuse light factor
                if (specular) {
                    s = dot_vv(sub_vv(mul_vd(&vertex->normal_camera, 2*d), lv), ev); //specular light factor
                }
                if (diffuse && d > 0.0) {
                    // Diffuse illumination color components
                    vertex->color_diff = *COLOR_add(&vertex->color_diff,
                                                    COLOR_scale(&scene->light_settings.directional, d));
                }
                if (specular && s > 0.0) {
                    // Specular illumination color components
                    s = pow(s, obj->specular_power);
                    vertex->color_spec = *COLOR_add(&vertex->color_spec,
                                                    COLOR_scale(&scene->light_settings.directional, s));
                }
            }

            for (j = 0; j < light_cnt; j++) {
                // Calculate light vector
                lv = sub_vv(&vertex->camera, &lights[j]->obj->vertices[0].camera);
                lv_length = length_v(lv);
                lv = div_vd(lv, lv_length);

                // Calculate light damping factor
                // TODO Consider other form of this factor, i.e. inverse square length
                // Maybe it should be lv_length+ev_length
                ldf = 1.0/(1.0 + scene->light_settings.attenuation*lv_length);


                d = dot_vv(&vertex->normal_camera, lv); //diffuse light factor
                if (specular) {
                    s = dot_vv(sub_vv(mul_vd(&vertex->normal_camera, 2*d), lv), ev); //specular light factor
                }
                if (diffuse && d > 0.0) {
                    // Diffuse illumination color components
                    d *= ldf;
                    vertex->color_diff = *COLOR_add(&vertex->color_diff, 
                                                    COLOR_scale(&lights[j]->obj->vertices[0].color_surf, d));

                }
                if (specular && s > 0.0) {
                    // Specular illumination color components
                    s = ldf * pow(s, obj->specular_power);
                    vertex->color_spec = *COLOR_add(&vertex->color_spec,
                                                    COLOR_scale(&lights[j]->obj->vertices[0].color_surf, s));
                }
            }
        }
    }
}

void lighting_face_coloring_diffuse(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        face->color_diff = *COLOR_mul(&face->color_diff, &face->color_surf);
    }
}

void lighting_vertex_coloring_diffuse(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            vertex->color_diff = *COLOR_mul(&vertex->color_diff, &vertex->color_surf);
        }
    }
}

void lighting_face_coloring_specular(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        //TODO: instead of color_surf it should be ambient*color_surf
        // Clip result color components to upper bound
        face->color_diff = *COLOR_add_sat(&face->color_surf, &face->color_spec);
    }
}

void lighting_vertex_coloring_specular(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            //TODO: instead of color_surf it should be ambient*color_surf
            // Clip result color components to upper bound
            vertex->color_diff = *COLOR_add_sat(&vertex->color_surf, &vertex->color_spec);
        }
    }
}

void lighting_face_coloring_merge(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        // Clip result color components to upper bound
        face->color_diff = *COLOR_add_sat(&face->color_spec,
                                          COLOR_mul(&face->color_diff, &face->color_surf));

    }
}

void lighting_vertex_coloring_merge(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            // Clip result color components to upper bound
            vertex->color_diff = *COLOR_add_sat(&vertex->color_spec,
                                                COLOR_mul(&vertex->color_diff, &vertex->color_surf));
        }
    }
}
