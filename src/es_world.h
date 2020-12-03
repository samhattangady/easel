/*
 * es_world is the file that holds all the data for the game.
 * The player controls generally directly affect the params in
 * structs defined in this file
 */

#ifndef ES_WORLD_DEFINED
#define ES_WORLD_DEFINED

#include "SDL.h"
#include "es_warehouse.h"

typedef struct {
    vec3 position;
    vec3 target;
    vec3 up_axis;
} EsWorld;

extern SDL_bool world_init(EsWorld* w);
extern SDL_bool world_update(EsWorld* w);

#endif
