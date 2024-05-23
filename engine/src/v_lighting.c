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

#include "engine_types.h"
#include "v_geometry.h"

void lighting_face_calculation(SCENE_3D *scene, OBJ_3D *obj) {
    VEC_4 *lv; //light vector
    VEC_4 ev; //eye vector
    FACE *face;
    VEC_4 face_center, *fc;
    FLOAT d, s;
    INT i, j, k;
    OBJ_3D_CONTAINER **lights = scene->light;
    INT light_cnt = scene->light_cnt;
    FLOAT lv_length; //light vector length
    FLOAT ldf; //light damping factor
    bool diffuse = true, specular = true;

    // Front faces lighting calculation first
    for (i=0; i<obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (k=0; k<3; k++) {
            face->color_diff[k] = scene->light_settings.ambient[k];
            face->color_spec[k] = 0.0;
        }

        // Determine face centre and eye vector(ev) from eye to face centre
        fc = &obj->vertices[face->vi[0]].camera;
        for (j=1; j < face->vcnt; j++)
            fc = add_vv(fc, &obj->vertices[face->vi[j]].camera);
        copy_v4(&face_center, div_vd(fc, face->vcnt));
        if (specular) {
            copy_v4(&ev, norm_v(&face_center));
        }

        // If directional light defined - calculate illumination from it
        if (scene->light_settings.directional[0] > 0.0 || scene->light_settings.directional[1] > 0.0 || scene->light_settings.directional[2] > 0.0) {
            lv = (VEC_4*)scene->light_settings.direction_in_camera_space;

            d = dot_vv(&face->normal_camera, lv); //diffuse light factor
            if (specular) {
                s = dot_vv(sub_vv(mul_vd(&face->normal_camera, 2*d), lv), &ev); //specular light factor
            }
            if (diffuse && d > 0.0) {
                // Diffuse illumination color components
                for (k=0; k<3; k++) {
                    face->color_diff[k] += d*scene->light_settings.directional[k];
                }
            }
            if (specular && s > 0.0) {
                // Specular illumination color components
                s = pow(s, obj->specular_power);
                for (k=0; k<3; k++) {
                    face->color_spec[k] += s*scene->light_settings.directional[k];
                }
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
                for (k=0; k<3; k++) {
                    face->color_diff[k] += d*lights[j]->obj->vertices[0].color_surf[k];
                }
            }
            if (specular && s > 0.0) {
                // Specular illumination color components
                s = ldf * pow(s, obj->specular_power);
                for (k=0; k<3; k++) {
                    face->color_spec[k] += s*lights[j]->obj->vertices[0].color_surf[k];
                }
            }
        }
    }
}

void lighting_vertex_calculation(SCENE_3D *scene, OBJ_3D *obj) {
    VERTEX *vertex;
    VEC_4 *lv, *ev; //light vector, eye vector, face normal
    FLOAT d, s;
    INT i, j, k;
    OBJ_3D_CONTAINER **lights = scene->light;
    INT light_cnt = scene->light_cnt;
    FLOAT lv_length; //light vector length
    FLOAT ldf; //light damping factor
    bool diffuse = true, specular = true;

    // All vertex lighting calculation first
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) { //calculate lighting only for front-facing vertices
            for (k=0; k<3; k++) {
                vertex->color_diff[k] = scene->light_settings.ambient[k];
                vertex->color_spec[k] = 0.0;
            }

            if (specular) {
                ev = norm_v(&vertex->camera); //eye vector
            }
            // If directional light defined - calculate illumination from it
            if (scene->light_settings.directional[0] != 0.0 || scene->light_settings.directional[1] != 0.0 || scene->light_settings.directional[2] != 0.0) {
                lv = (VEC_4*)scene->light_settings.direction_in_camera_space;

                d = dot_vv(&vertex->normal_camera, lv); //diffuse light factor
                if (specular) {
                    s = dot_vv(sub_vv(mul_vd(&vertex->normal_camera, 2*d), lv), ev); //specular light factor
                }
                if (diffuse && d > 0.0) {
                    // Diffuse illumination color components
                    for (k=0; k<3; k++) {
                        vertex->color_diff[k] += d*scene->light_settings.directional[k];
                    }
                }
                if (specular && s > 0.0) {
                    // Specular illumination color components
                    s = pow(s, obj->specular_power);
                    for (k=0; k<3; k++) {
                        vertex->color_spec[k] += s*scene->light_settings.directional[k];
                    }
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
                    for (k=0; k<3; k++) {
                        vertex->color_diff[k] += d*lights[j]->obj->vertices[0].color_surf[k];
                    }
                }
                if (specular && s > 0.0) {
                    // Specular illumination color components
                    s = ldf * pow(s, obj->specular_power);
                    for (k=0; k<3; k++) {
                        vertex->color_spec[k] += s*lights[j]->obj->vertices[0].color_surf[k];
                    }
                }
            }
        }
    }
}

void lighting_face_coloring_diffuse(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0, j = 0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j=0; j<3; j++) {
            face->color_diff[j] *= face->color_surf[j];
        }
    }
}

void lighting_vertex_coloring_diffuse(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0, j = 0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            for (j=0; j<3; j++) {
                vertex->color_diff[j] *= vertex->color_surf[j];
            }
        }
    }
}

void lighting_face_coloring_specular(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0, j = 0;
    FLOAT f = 0.0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j=0; j<3; j++) {
            //TODO: instead of color_surf it should be ambient*color_surf
            f = face->color_surf[j]+face->color_spec[j];
            // Clip result color to upper bound
            face->color_diff[j] = f > 1.0 ? 1.0 : f;
        }
    }
}

void lighting_vertex_coloring_specular(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0, j = 0;
    FLOAT f = 0.0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            for (j=0; j<3; j++) {
                //TODO: instead of color_surf it should be ambient*color_surf
                f = vertex->color_surf[j]+vertex->color_spec[j];
                // Clip result color to upper bound
                vertex->color_diff[j] = f > 1.0 ? 1.0 : f;
            }
        }
    }
}

void lighting_face_coloring_merge(OBJ_3D *obj) {
    FACE *face = NULL;
    INT i = 0, j = 0;
    FLOAT f = 0.0;
    for (i = 0; i < obj->front_fcnt; i++) {
        face = obj->front_faces[i];
        for (j=0; j<3; j++) {
            f = face->color_diff[j]*face->color_surf[j] + face->color_spec[j];
            // Clip result color to upper bound
            face->color_diff[j] = f > 1.0 ? 1.0 : f;
        }
    }
}

void lighting_vertex_coloring_merge(OBJ_3D *obj) {
    VERTEX *vertex = NULL;
    INT i = 0, j = 0;
    FLOAT f = 0.0;
    for (i=0; i<obj->vcnt; i++) {
        vertex = obj->vertices + i;
        if (vertex->front) {
            for (j=0; j<3; j++) {
                f = vertex->color_diff[j]*vertex->color_surf[j] + vertex->color_spec[j];
                // Clip result color to upper bound
                vertex->color_diff[j] = f > 1.0 ? 1.0 : f;
            }
        }
    }
}
