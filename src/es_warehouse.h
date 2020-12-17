/*
 * es_warehouse is where we store all miscellaneous types and structs
 */

#include "SDL.h"

#ifndef ES_WAREHOUSE_DEFINED
#define ES_WAREHOUSE_DEFINED

#define DEBUG_BUILD SDL_TRUE
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    Uint32 x;
    Uint32 y;
} vec2ui;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    Uint32 x;
    Uint32 y;
    Uint32 z;
} vec3ui;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct {
    Uint32 x;
    Uint32 y;
    Uint32 z;
    Uint32 w;
} vec4ui;

typedef struct {
    vec4 a;
    vec4 b;
    vec4 c;
    vec4 d;
} mat4;

typedef struct {
    vec3 pos;
    vec3 color;
    vec2 tex;
    vec3 normal;
    vec4 assorted;
} EsVertex;

extern void warehouse_error_popup(const char* error_header, const char* error_text);
extern float warehouse_log_2(float num);
extern vec2 build_vec2(float x, float y);
extern vec2ui build_vec2ui(Uint32 x, Uint32 y);
extern mat4 mat4_mat4_multiply(mat4 a, mat4 b);
extern vec4 mat4_vec4_multiply(mat4 a, vec4 b);
extern void print_mat4(mat4 a);
extern void print_vec4(vec4 a);
extern void print_vec3ui(vec3ui a);
extern mat4 identity_mat4();
extern mat4 build_mat4(float a1, float a2, float a3, float a4,
                       float b1, float b2, float b3, float b4,
                       float c1, float c2, float c3, float c4,
                       float d1, float d2, float d3, float d4);
extern mat4 look_at(vec3 eye, vec3 target, vec3 up);
extern mat4 look_at_z(vec3 eye, vec3 target);
extern mat4 look_at_y(vec3 eye, vec3 target);
extern vec3 vec3_sub(vec3 a, vec3 b);
extern vec3 vec3_add(vec3 a, vec3 b);
extern vec3 vec3_cross(vec3 a, vec3 b);
extern vec3 vec3_scale(vec3 a, float b);
extern SDL_bool vec3_is_zero(vec3 a);
extern float vec3_dot(vec3 a, vec3 b);
extern vec3 vec3_normalize(vec3 a);
extern float vec3_length(vec3 a);
extern float vec3_length_squared(vec3 a);
extern float vec3_magnitude(vec3 a);
extern float vec3_distance(vec3 a, vec3 b);
extern float vec3_distance_squared(vec3 a, vec3 b);
extern vec3 build_vec3(float x, float y, float z);
extern vec4 build_vec4(float x, float y, float z, float w);
extern vec4 build_vec4_vec3f(vec3 v, float w);
extern vec3ui build_vec3ui(Uint32 x, Uint32 y, Uint32 z);
extern vec4ui build_vec4ui(Uint32 x, Uint32 y, Uint32 z, Uint32 w);
extern mat4 rotation_matrix_axis(float angle, vec3 axis);
extern mat4 rotation_matrix_xaxis(float angle);
extern mat4 rotation_matrix_yaxis(float angle);
extern mat4 rotation_matrix_zaxis(float angle);
extern vec3 rotate_about_origin_axis(vec3 point, float angle, vec3 axis);
extern vec3 rotate_about_anchor_axis(vec3 point, vec3 anchor, float angle, vec3 axis);
extern vec3 rotate_about_origin_xaxis(vec3 point, float angle);
extern vec3 rotate_about_origin_yaxis(vec3 point, float angle);
extern vec3 rotate_about_origin_zaxis(vec3 point, float angle);
extern mat4 perspective_projection(float angle, float aspect_ratio, float near, float far);
extern float deg_to_rad(float deg);
extern float lerp(float start, float end, float val);
extern float rand_pos();
extern float rand_negpos();
extern vec3 rand_vec3();
extern vec3 vec3_from_vec4(vec4 a);

#endif
