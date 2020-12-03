#include "es_world.h"

SDL_bool world_init(EsWorld* w) {
    w->position = build_vec3(0.0f, 14.0f, 35.0f);
    w->target = build_vec3(0.0f, 4.0f, 0.0f);
    w->up_axis = build_vec3(0.0f, 1.0f, 0.0f);
    return SDL_TRUE;
}

SDL_bool world_update(EsWorld* w) {
    // w->up_axis = rotate_about_origin_zaxis(w->up_axis, -0.002);
    return SDL_TRUE;
}
