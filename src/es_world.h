/*
 * es_world is the file that holds all the data for the game.
 * The player controls generally directly affect the params in
 * structs defined in this file
 */

#ifndef ES_WORLD_DEFINED
#define ES_WORLD_DEFINED

#include "SDL.h"
#include "es_warehouse.h"
#include "es_geometrygen.h"
#include "es_trees.h"


typedef struct {
    SDL_bool l_down;
    SDL_bool r_down;
    SDL_bool l_pressed; // set for just the one tick where it was released
    SDL_bool r_pressed; // set for just the one tick where it was released
    SDL_bool l_released; // set for just the one tick where it was released
    SDL_bool r_released; // set for just the one tick where it was released
    float l_down_x;
    float l_down_y;
    float r_down_x;
    float r_down_y;
    float current_x;
    float current_y;
    float moved_x;
    float moved_y;
} MouseData;

typedef struct {
    SDL_bool up_down;
    SDL_bool down_down;
    SDL_bool left_down;
    SDL_bool right_down;
} ControlsData;

typedef struct {
    SDL_bool running;
    vec3 position;
    vec3 target;
    vec3 up_axis;
    MouseData mouse;
    ControlsData controls;
    EsGeometry tree_geom;
    SDL_bool refresh_tree;
} EsWorld;

extern SDL_bool world_init(EsWorld* w);
extern SDL_bool world_update(EsWorld* w);
extern SDL_bool world_process_input_event(EsWorld* w, SDL_Event e);
extern SDL_bool world_flush_inputs(EsWorld* w);

#endif
