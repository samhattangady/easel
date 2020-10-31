#include "es_warehouse.h"

float warehouse_log_2(float num) {
    return SDL_log(num) / SDL_log(2);
}
void warehouse_error_popup(const char* error_header, const char* error_text) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, error_text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_header, error_text, NULL);
    return;
}

mat4 mat4_mat4_multiply(mat4 a, mat4 b) {
    mat4 result;
    result.a.x = a.a.x*b.a.x + a.a.y*b.b.x + a.a.z*b.c.x + a.a.w*b.d.x;    
    result.a.y = a.a.x*b.a.y + a.a.y*b.b.y + a.a.z*b.c.y + a.a.w*b.d.y;    
    result.a.z = a.a.x*b.a.z + a.a.y*b.b.z + a.a.z*b.c.z + a.a.w*b.d.z;    
    result.a.w = a.a.x*b.a.w + a.a.y*b.b.w + a.a.z*b.c.w + a.a.w*b.d.w;    
    result.b.x = a.b.x*b.a.x + a.b.y*b.b.x + a.b.z*b.c.x + a.b.w*b.d.x;    
    result.b.y = a.b.x*b.a.y + a.b.y*b.b.y + a.b.z*b.c.y + a.b.w*b.d.y;    
    result.b.z = a.b.x*b.a.z + a.b.y*b.b.z + a.b.z*b.c.z + a.b.w*b.d.z;    
    result.b.w = a.b.x*b.a.w + a.b.y*b.b.w + a.b.z*b.c.w + a.b.w*b.d.w;    
    result.c.x = a.c.x*b.a.x + a.c.y*b.b.x + a.c.z*b.c.x + a.c.w*b.d.x;    
    result.c.y = a.c.x*b.a.y + a.c.y*b.b.y + a.c.z*b.c.y + a.c.w*b.d.y;    
    result.c.z = a.c.x*b.a.z + a.c.y*b.b.z + a.c.z*b.c.z + a.c.w*b.d.z;    
    result.c.w = a.c.x*b.a.w + a.c.y*b.b.w + a.c.z*b.c.w + a.c.w*b.d.w;    
    result.d.x = a.d.x*b.a.x + a.d.y*b.b.x + a.d.z*b.c.x + a.d.w*b.d.x;    
    result.d.y = a.d.x*b.a.y + a.d.y*b.b.y + a.d.z*b.c.y + a.d.w*b.d.y;    
    result.d.z = a.d.x*b.a.z + a.d.y*b.b.z + a.d.z*b.c.z + a.d.w*b.d.z;    
    result.d.w = a.d.x*b.a.w + a.d.y*b.b.w + a.d.z*b.c.w + a.d.w*b.d.w;    
    return result;
}

vec4 mat4_vec4_multiply(mat4 a, vec4 b) {
    vec4 result;
    result.x = a.a.x*b.x + a.a.y*b.y + a.a.z*b.z + a.a.w*b.w;    
    result.y = a.b.x*b.x + a.b.y*b.y + a.b.z*b.z + a.b.w*b.w;    
    result.z = a.c.x*b.x + a.c.y*b.y + a.c.z*b.z + a.c.w*b.w;    
    result.w = a.d.x*b.x + a.d.y*b.y + a.d.z*b.z + a.d.w*b.w;    
    return result;
}

void print_mat4(mat4 a) {
    SDL_Log("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
            a.a.x, a.a.y, a.a.z, a.a.w,
            a.b.x, a.b.y, a.b.z, a.b.w,
            a.c.x, a.c.y, a.c.z, a.c.w,
            a.d.x, a.d.y, a.d.z, a.d.w);
    return;
}

void print_vec4(vec4 a) {
    SDL_Log("%f\n%f\n%f\n%f\n",
            a.x, a.y, a.z, a.w);
    return;
}

mat4 identity_mat4() {
    mat4 result;
    result.a.x = 1.0f;    
    result.a.y = 0.0f;    
    result.a.z = 0.0f;    
    result.a.w = 0.0f;    
    result.b.x = 0.0f;    
    result.b.y = 1.0f;    
    result.b.z = 0.0f;    
    result.b.w = 0.0f;    
    result.c.x = 0.0f;    
    result.c.y = 0.0f;    
    result.c.z = 1.0f;    
    result.c.w = 0.0f;    
    result.d.x = 0.0f;    
    result.d.y = 0.0f;    
    result.d.z = 0.0f;    
    result.d.w = 1.0f;    
    return result;
}

mat4 build_mat4(float a1, float a2, float a3, float a4,
                       float b1, float b2, float b3, float b4,
                       float c1, float c2, float c3, float c4,
                       float d1, float d2, float d3, float d4) {
    mat4 result;
    result.a.x = a1;    
    result.a.y = a2;    
    result.a.z = a3;    
    result.a.w = a4;    
    result.b.x = b1;    
    result.b.y = b2;    
    result.b.z = b3;    
    result.b.w = b4;    
    result.c.x = c1;    
    result.c.y = c2;    
    result.c.z = c3;    
    result.c.w = c4;    
    result.d.x = d1;    
    result.d.y = d2;    
    result.d.z = d3;    
    result.d.w = d4;    
    return result;
}

vec3 vec3_sub(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

vec3 vec3_add(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

mat4 look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 zaxis = vec3_normalize(vec3_sub(eye, target));
    vec3 xaxis = vec3_normalize(vec3_cross(up, zaxis));
    vec3 yaxis = vec3_normalize(vec3_cross(zaxis, xaxis));
    mat4 result = build_mat4(
            xaxis.x, yaxis.x, zaxis.x, 0.0f,
            xaxis.y, yaxis.y, zaxis.y, 0.0f,
            xaxis.z, yaxis.z, zaxis.z, 0.0f,
            -vec3_dot(xaxis, eye), 
            -vec3_dot(yaxis, eye), 
            -vec3_dot(zaxis, eye), 
            1.0f);
    return result;
}

mat4 look_at_z(vec3 eye, vec3 target) {
    return look_at(eye, target, build_vec3(0.0f, 0.0f, 1.0f));
}

vec3 vec3_cross(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - a.x*b.z;
    result.z = a.x*b.y - a.y*b.x;
    return result;
}

float vec3_dot(vec3 a, vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

vec3 vec3_normalize(vec3 a) {
    vec3 result;
    float magnitude = vec3_magnitude(a);
    result.x = a.x/magnitude;
    result.y = a.y/magnitude;
    result.z = a.z/magnitude;
    return result;
}

float vec3_magnitude(vec3 a) {
    return SDL_sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

vec3 build_vec3(float x, float y, float z) {
    vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}



void _test_multiplication() {
    mat4 a;
    a.a.x = 5.0f;
    a.a.y = 2.0f;
    a.a.z = 6.0f;
    a.a.w = 1.0f;
    a.b.x = 0.0f;
    a.b.y = 6.0f;
    a.b.z = 2.0f;
    a.b.w = 0.0f;
    a.c.x = 3.0f;
    a.c.y = 8.0f;
    a.c.z = 1.0f;
    a.c.w = 4.0f;
    a.d.x = 1.0f;
    a.d.y = 8.0f;
    a.d.z = 5.0f;
    a.d.w = 6.0f;
    mat4 b;
    b.a.x = 7.0f;
    b.a.y = 5.0f;
    b.a.z = 8.0f;
    b.a.w = 0.0f;
    b.b.x = 1.0f;
    b.b.y = 8.0f;
    b.b.z = 2.0f;
    b.b.w = 6.0f;
    b.c.x = 9.0f;
    b.c.y = 4.0f;
    b.c.z = 3.0f;
    b.c.w = 8.0f;
    b.d.x = 5.0f;
    b.d.y = 3.0f;
    b.d.z = 7.0f;
    b.d.w = 9.0f;
    // print_mat4(mat4_vec4_multiply(a, b));
    // print_vec4(mat4_vec4_multiply(a, b.a));
}
