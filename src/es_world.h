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
    SDL_bool l_pressed;
    SDL_bool r_pressed;
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
    SDL_bool up_pressed;
    SDL_bool down_pressed;
    SDL_bool left_pressed;
    SDL_bool right_pressed;
} ControlsData;

typedef struct {
    SDL_bool running;
    vec3 position;
    vec3 target;
    vec3 up_axis;
    MouseData mouse;
    ControlsData controls;
} EsWorld;

extern SDL_bool world_init(EsWorld* w);
extern SDL_bool world_update(EsWorld* w);
extern SDL_bool world_process_input_event(EsWorld* w, SDL_Event e);
extern SDL_bool world_flush_inputs(EsWorld* w);

#endif
