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

#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t INT;
typedef uint32_t UINT;

typedef float FLOAT;
#define FMOD fmodf
#define FABS fabsf
/*
typedef double FLOAT;
#define FMOD fmod
#define FABS fabs
*/

typedef uint32_t ARGB_PIXEL;
typedef uint32_t Z_PIXEL;
typedef uint32_t BUMP_PIXEL;

#define FRACT_SHIFT 16
#define FRACT_MASK ((1<<16)-1)

#define A_SHIFT 24
#define R_SHIFT 16
#define G_SHIFT 8
#define B_SHIFT 0

#define A_MASK 0xFF000000
#define R_MASK 0x00FF0000
#define G_MASK 0x0000FF00
#define B_MASK 0x000000FF
#define LSB8_MASK 0x000000FF

//Overflow bits from each component
#define R_OVFL 0x01000000
#define G_OVFL 0x00010000
#define B_OVFL 0x00000100

#define ARGB_PIXEL_ALPHA(P) ((P)>>24)
#define ARGB_PIXEL_RED(P) (((P)>>16)&0x000000FF)
#define ARGB_PIXEL_GREEN(P) (((P)>>8)&0x000000FF)
#define ARGB_PIXEL_BLUE(P) ((P)&0x000000FF)

#define Z_BUFFER_ON 1
#define Z_BUFFER_OFF 0
//#define Z_BUFFER_MAX (65535) //2^16-1
#define Z_BUFFER_MAX (2147483647) //2^31-1
//#define Z_BUFFER_MAX (4294967295) //2^32-1

#define MAX_FACE_VERTICES (6)

#define TWOPI (6.283185307)
#define PI (3.141592654)
#define PI_2 (1.570796327)
#define PI_4 (0.785398163)

typedef struct {
    int frames;
    double time;
    double music_position;
} RUN_STATS;

/** Types for object generators*/
typedef enum {
    TETRAHEDRON,
    CUBE,
    OCTAHEDRON,
    DODECAHEDRON,
    ICOSAHEDRON
} POLYHEDRON_TYPE;
typedef enum {
    //othogonal mesh of vertices
    QUAD,
    RIGHT_TRIANGLE,

    //hexagonal mesh of vertices
    EQUILATERAL_TRIANGLE,
    WIDE_TRIANGLE,
} MESH_TYPE;
typedef enum {
    NO_WRAP,
    WRAP_U,
    WRAP_UV
} SURFACE_WRAP;
typedef enum {
    HIDDEN,
    SOLID_UNSHADED,
    SOLID_DIFF,
    SOLID_SPEC,
    SOLID_DIFF_SPEC,
    SOLID_DIFF_TEXTURED,
    SOLID_SPEC_TEXTURED,
    SOLID_DIFF_SPEC_TEXTURED,
    INTERP_UNSHADED,
    INTERP_DIFF,
    INTERP_SPEC,
    INTERP_DIFF_SPEC,
    INTERP_DIFF_TEXTURED,
    INTERP_SPEC_TEXTURED,
    INTERP_DIFF_SPEC_TEXTURED,
    TX_MAP_BASE,
    TX_MAP_BASE_MUL,
    TX_MAP_BASE_ADD,
    TX_MAP_BASE_MUL_ADD,
    TX_MAP_BUMP_REFLECTION,
    REFLECTION,
    POINT_LIGHTS,
    PARTICLES
} OBJ_3D_TYPE;

typedef struct {
    FLOAT amp[3]; //amplitude
    FLOAT frq[3]; //frequency
    FLOAT phs[3]; //phase [radians]
    INT cnt; //components count(0 to 3)
} CYCLE_COMPONENT;

typedef struct {
    CYCLE_COMPONENT x;
    CYCLE_COMPONENT y;
    CYCLE_COMPONENT z;
    FLOAT phi; //diameter
} CYCLOID;

typedef enum {
    //default key code
    K_UNSUPPORTED = 0x00,
    //min code supported by this engine
    K_MIN_CODE = 0x20,
    //Range of printable key codes
    K_MIN_PRINTABLE = K_MIN_CODE,
    K_MAX_PRINTABLE = 0x7F,
    //Non-printable key codes
    K_RETURN,
    K_ESCAPE,
    K_BACKSPACE,
    K_TAB,
    K_CAPSLOCK,
    K_F1,
    K_F2,
    K_F3,
    K_F4,
    K_F5,
    K_F6,
    K_F7,
    K_F8,
    K_F9,
    K_F10,
    K_F11,
    K_F12,
    K_PRINTSCREEN,
    K_SCROLLLOCK,
    K_PAUSE,
    K_INSERT,
    K_HOME,
    K_PAGEUP,
    K_DELETE,
    K_END,
    K_PAGEDOWN,
    K_RIGHT,
    K_LEFT,
    K_DOWN,
    K_UP,
    K_NUMLOCKCLEAR,
    K_KP_DIVIDE,
    K_KP_MULTIPLY,
    K_KP_MINUS,
    K_KP_PLUS,
    K_KP_ENTER,
    K_KP_1,
    K_KP_2,
    K_KP_3,
    K_KP_4,
    K_KP_5,
    K_KP_6,
    K_KP_7,
    K_KP_8,
    K_KP_9,
    K_KP_0,
    K_KP_PERIOD,
    K_KP_COMMA,
    K_MUTE,
    K_VOLUMEUP,
    K_VOLUMEDOWN,
    K_LCTRL,
    K_LSHIFT,
    K_LALT,
    K_RCTRL,
    K_RSHIFT,
    K_RALT,
    //max code supported by this engine
    K_MAX_CODE = K_RALT
} KEY_CODE;

typedef enum {NO_EVENT, KEY_PRESSED, KEY_HOLD, KEY_RELEASED, QUIT_REQUEST} EVENT_TYPE;
typedef struct {
    EVENT_TYPE type;
    INT timestamp; //time when event was generated

    //key event-related members
    KEY_CODE code; //range 0x20-0x7F for printable characters
                   //or one of above K_x definitions for non-printable
    FLOAT hold_time; //key hold time (0 for PRESSED)
} EVENT;

typedef struct { //at any given time only r, g and b have to filled/valid
    FLOAT a; //alpha channel
    //Rules for alpha channel:
    //1. When two COLORs are blended: blend A just like other components (r, g, b)
    //2. When two COLORs are added: A = (A1 + A2)/2
    //3. When COLOR is scaled: do not change A
    FLOAT r, g, b; //red, green blue
    FLOAT h, s, l; //hue, saturation, lightness
} COLOR;

typedef struct {
    FLOAT t[20]; //control points coordinates
    COLOR color[20]; //control points colors
    COLOR background; //color used if t coordinates don't reach 0.0 or 1.0
    INT count; //number of stops used
} GRADIENT;

typedef struct {
    ARGB_PIXEL *pixval; //gradient precalculated values
    INT length; //number of precalculated values
} DISCRETE_GRADIENT;

typedef FLOAT VEC_4[4];
typedef FLOAT MAT_4_4[4][4]; //Transform matrix type for transforming 3D coordinates
/*
 Coordinates in rendering space
 Each entry has: [0] - screen X, [1] - screen Y, [2] - zbuffer Z.
 Negative/over max limit values are allowed.
*/
typedef INT PROJECTION_COORD[3];

typedef struct {
    //Alpha channel by default is set to 0 for RENDER_BUFFERs and
    //ARGB_MAPs used a rasterization targets.
    //Alpha channel is set to 255 for ARGB_MAPs holding textures without alpha channel
    //Alpha channel is set between 0-255 for ARGB_MAPs holding textures with alpha channel
    ARGB_PIXEL *data;
    INT width, height, height_with_margin;
} ARGB_MAP;

typedef struct {
    Z_PIXEL *data;
    INT width, height;
} Z_MAP;

typedef struct {
    BUMP_PIXEL *data;
    INT width, height;
    FLOAT margin; //margin of reflection map to use as extra space for bump reflections
} BUMP_MAP;

typedef struct {
    ARGB_MAP *map; //pixels buffer
    Z_MAP *z; //z buffer
    INT width, height;
} RENDER_BUFFER;

typedef struct {
    FLOAT u;
    FLOAT v;
} SURFACE_COORD; //map(texture, reflection. etc.) coordinates

typedef struct {
    INT u;
    INT v;
} MAP_COORD; //map(texture, reflection. etc.) coordinates

typedef struct {
    VEC_4 root;//Root space coordinates
    VEC_4 camera; //Camera space coordinates
    VEC_4 normal_root; //Vertex normal in root space
    VEC_4 normal_camera; //Vertex normal in camera space
    VEC_4 light; //normalized light vector
    COLOR color_surf; //surface color, assigned by user
    //REMARK: for texture mapped objects color_surf is not used.
    COLOR color_diff; //diffuse lighting component
    COLOR color_spec; //specular lighting component
    PROJECTION_COORD projection; //perspective projected coordinates
    INT avcnt; //count of adjacent vertices
    //TODO: avi should be preferably allocated dynamically...
    INT avi[8]; //adjacent vertices indexes (inside OBJ_3D.vertices) - for wireframe
    bool front; //if true: Vertex is facing the camera
} VERTEX;

typedef struct {
    INT vcnt; //Face vertices count (3 to 6)
    INT vi[MAX_FACE_VERTICES]; //Vertex indexes
    SURFACE_COORD t[MAX_FACE_VERTICES]; //Texture map coordinates, [0., 1.] relative range.
    MAP_COORD bc[MAX_FACE_VERTICES]; //Texture map base coordinates, [0, map_width/height] map range.
    MAP_COORD rc[MAX_FACE_VERTICES]; //Reflection/mul/add map coordinates set 1, [0, map_width/height] map range.
    VEC_4 normal_root; //Face normal in root space
    VEC_4 normal_camera; //Face normal in camera space
    COLOR color_surf; //surface color, assigned by user
    //REMARK: for texture mapped objects color_surf is not used.
    COLOR color_diff; //diffuse lighting component
    COLOR color_spec; //specular lighting component
} FACE;

typedef struct {
    FLOAT specular_power;
    bool wireframe_on;

    // Zero point coordinates transformed to camera space. Used for determination of transformed normals origin
    VEC_4 zero_camera;

    INT fcnt; // Object total face count
    FACE *faces;
    INT front_fcnt; // Object visible face count
    FACE **front_faces; // Array of pointers to visible faces in "faces"
    INT vcnt; // Object total vertices count
    VERTEX *vertices;
    SURFACE_COORD *vertex_s;

    OBJ_3D_TYPE type;

    //The color of the whole object. This can be overloaded by FACE.color or VERTEX.color
    COLOR surface_color;
    COLOR wireframe_color;
    ARGB_MAP *base_map;
    BUMP_MAP *bump_map;
    ARGB_MAP *mul_map;
    ARGB_MAP *add_map;
    ARGB_MAP *reflection_map;
} OBJ_3D;

typedef struct {
    VEC_4 look_at; //Root-space coordinates. Location of look-at point
    VEC_4 pos; //Root-space coordinates. Location of camera
    FLOAT roll; // Camera roll along viewing axis in degrees
    FLOAT fov; // Camera Field Of View in degrees
    FLOAT near_z; // Near Z clipping plane
    FLOAT far_z; // Far Z clipping plane
} CAMERA_SETTINGS;

typedef struct {
    bool enabled;
    COLOR ambient; //ambient light color
    COLOR directional; // Directional light color (only 1 per scene)
    VEC_4 direction; // Directional light vector (only 1 per scene)
    VEC_4 direction_in_camera_space; // Directional light vector (only 1 per scene)
    FLOAT attenuation; //point lights attenuation
} GLOBAL_LIGHT_SETTINGS;

typedef struct SCENE_3D SCENE_3D;
typedef struct OBJ_3D_CONTAINER OBJ_3D_CONTAINER;

struct OBJ_3D_CONTAINER {
    FLOAT ax, ay, az; // Object angle relative to parent object
    FLOAT px, py, pz; // Object position relative to parent object
    FLOAT sx, sy, sz; // Object position relative to parent object
    MAT_4_4 object_matrix; // Transformation matrix from root space to camera space for object
    MAT_4_4 normals_matrix; // Transformation matrix from root space to camera space for normals
    OBJ_3D* obj; // Object geometrical/visual description
    bool obj_owned; //if true, then this container owns obj, and should free it when is itself freed
    INT child_cnt; // Children count
    OBJ_3D_CONTAINER* parent;
    OBJ_3D_CONTAINER** child;
    SCENE_3D *scene;
};

struct SCENE_3D{
    RENDER_BUFFER* render_buf;
    INT root_cnt; // Root containers count
    OBJ_3D_CONTAINER** root; // Scene root containers(all object types)
    CAMERA_SETTINGS camera;
    //Renderable objects are anything that can be itself rendered.
    //That is everything except light sources, invisible enchor objects, etc.
    INT renderable_cnt; // Renderble objects count
    OBJ_3D_CONTAINER** renderable; // Renderable objects containers in root-space coordinates

    GLOBAL_LIGHT_SETTINGS light_settings;
    INT light_cnt; // Lights count
    OBJ_3D_CONTAINER** light; //Lights containers in root-space coordinates

    //if true rotate v-normals for every object in the scene
    //if false - do it on per-object basis
    bool rotate_all_objects_vertex_normals;
};

// Signature for parametric surface calculation function
typedef VEC_4* (*PARAMETRIC_SURFACE)(FLOAT, FLOAT);

#include "utils.h"
#endif