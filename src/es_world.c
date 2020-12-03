#include "es_world.h"

#define MOVE_SPEED 0.002

SDL_bool world_init(EsWorld* w) {
    w->running = SDL_TRUE;
    w->position = build_vec3(0.0f, 4.0f, 35.0f);
    w->target = build_vec3(0.0f, 0.0f, 0.0f);
    w->up_axis = build_vec3(0.0f, 1.0f, 0.0f);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    w->controls.up_pressed = SDL_FALSE;
    w->controls.down_pressed = SDL_FALSE;
    w->controls.left_pressed = SDL_FALSE;
    w->controls.right_pressed = SDL_FALSE;
    // TODO (03 Dec 2020 sam): Init mouse
    return SDL_TRUE;
}

SDL_bool world_update(EsWorld* w) {
    vec3 target_y = build_vec3(w->target.x, 0.0f, w->target.z);
    vec3 position_y = build_vec3(w->position.x, 0.0f, w->position.z);
    vec3 forward = vec3_normalize(vec3_sub(target_y, position_y));
    vec3 right = rotate_about_origin_yaxis(forward, -M_PI/2.0f);
    vec3 movement = build_vec3(0.0f, 0.0f, 0.0f);
    if (w->controls.up_pressed)
        movement = vec3_add(movement, vec3_scale(forward, MOVE_SPEED));
    if (w->controls.down_pressed)
        movement = vec3_add(movement, vec3_scale(forward, -MOVE_SPEED));
    if (w->controls.right_pressed)
        movement = vec3_add(movement, vec3_scale(right, MOVE_SPEED));
    if (w->controls.left_pressed)
        movement = vec3_add(movement, vec3_scale(right, -MOVE_SPEED));
    // TODO (03 Dec 2020 sam): Normalize when moving forward and side.
    w->position = vec3_add(w->position, movement);
    w->target = vec3_add(w->target, movement);
    float angle = -w->mouse.moved_x / 768.0f;
    w->target = rotate_about_anchor_axis(w->target, w->position, angle, w->up_axis);
    return SDL_TRUE;
}

SDL_bool world_process_input_event(EsWorld* w, SDL_Event e) {
    if (e.type == SDL_MOUSEMOTION) {
        float current_x = 1.0f*e.motion.x;
        float current_y = 1.0f*e.motion.y;
        w->mouse.moved_x = e.motion.xrel;
        w->mouse.moved_y = e.motion.yrel;
        w->mouse.current_x = current_x;
        w->mouse.current_y = current_y;
    }
    if (e.type == SDL_KEYDOWN) {
        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_ESCAPE)
            w->running = SDL_FALSE;
        if (key == SDLK_w)
            w->controls.up_pressed = SDL_TRUE;
        if (key == SDLK_s)
            w->controls.down_pressed = SDL_TRUE;
        if (key == SDLK_a)
            w->controls.left_pressed = SDL_TRUE;
        if (key == SDLK_d)
            w->controls.right_pressed = SDL_TRUE;
    }
    if (e.type == SDL_KEYUP) {
        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_w)
            w->controls.up_pressed = SDL_FALSE;
        if (key == SDLK_s)
            w->controls.down_pressed = SDL_FALSE;
        if (key == SDLK_a)
            w->controls.left_pressed = SDL_FALSE;
        if (key == SDLK_d)
            w->controls.right_pressed = SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_bool world_flush_inputs(EsWorld* w) {
    w->mouse.l_released = SDL_FALSE;
    w->mouse.r_released = SDL_FALSE;
    w->mouse.moved_x = 0.0f;
    w->mouse.moved_y = 0.0f;
    return SDL_TRUE;
}
