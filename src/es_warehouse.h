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
    float z;
} vec3;

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    vec2 pos;
    vec3 color;
} TutorialVertex;

extern void warehouse_error_popup(const char* error_header, const char* error_text);

#endif
