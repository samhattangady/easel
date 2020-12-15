#include "es_world.h"
#include "stb_perlin.h"

#define MOVE_SPEED 0.006f

SDL_bool world_init(EsWorld* w) {
    w->running = SDL_TRUE;
    w->position = build_vec3(0.0f, 8.0f, 35.0f);
    w->target = build_vec3(0.0f, 4.0f, 0.0f);
    w->up_axis = build_vec3(0.0f, 1.0f, 0.0f);
    // TODO (15 Dec 2020 sam): For some reason this stopped working. Works till commit 1f5810444a27b03c9e9903fba99c17f6d806509b
    int result = SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_Log("Relavive mouse = %i", result);
    w->controls.up_down = SDL_FALSE;
    w->controls.down_down = SDL_FALSE;
    w->controls.left_down = SDL_FALSE;
    w->controls.right_down = SDL_FALSE;
    w->tree_geom = geom_init_geometry_size(300000, 700000, 2, 300000, 0);
    w->refresh_tree = SDL_FALSE;
    w->mouse.l_down = SDL_FALSE;
    w->mouse.r_down = SDL_FALSE;
    w->mouse.l_pressed = SDL_FALSE;
    w->mouse.r_pressed = SDL_FALSE;
    w->mouse.l_released = SDL_FALSE;
    w->mouse.r_released = SDL_FALSE;
    w->mouse.l_down_x = 0.0f;
    w->mouse.l_down_y = 0.0f;
    w->mouse.r_down_x = 0.0f;
    w->mouse.r_down_y = 0.0f;
    w->mouse.current_x = 0.0f;
    w->mouse.current_y = 0.0f;
    w->mouse.moved_x = 0.0f;
    w->mouse.moved_y = 0.0f;
    return SDL_TRUE;
}

SDL_bool world_update(EsWorld* w) {
    vec3 target_y = build_vec3(w->target.x, 0.0f, w->target.z);
    vec3 position_y = build_vec3(w->position.x, 0.0f, w->position.z);
    vec3 forward = vec3_normalize(vec3_sub(target_y, position_y));
    vec3 right = rotate_about_origin_yaxis(forward, -(float)M_PI/2.0f);
    vec3 movement = build_vec3(0.0f, 0.0f, 0.0f);
    if (w->controls.up_down)
        movement = vec3_add(movement, vec3_scale(forward, MOVE_SPEED));
    if (w->controls.down_down)
        movement = vec3_add(movement, vec3_scale(forward, -MOVE_SPEED));
    if (w->controls.right_down)
        movement = vec3_add(movement, vec3_scale(right, MOVE_SPEED));
    if (w->controls.left_down)
        movement = vec3_add(movement, vec3_scale(right, -MOVE_SPEED));
    // TODO (03 Dec 2020 sam): Normalize when moving forward and side.
    w->position = vec3_add(w->position, movement);
    w->target = vec3_add(w->target, movement);
    float x_angle = -w->mouse.moved_x / 768.0f;
    w->target = rotate_about_anchor_axis(w->target, w->position, x_angle, w->up_axis);
    float y_angle = w->mouse.moved_y / 1024.0f;
    w->target = rotate_about_anchor_axis(w->target, w->position, y_angle, vec3_cross(w->up_axis, vec3_sub(w->target, w->position)));
    if (w->mouse.l_pressed) {
        // create tree at x, z where the target is
        float x = w->target.x;
        float z = w->target.z;
        float y = 1.0f * stb_perlin_noise3(x/10.0f, 0, z/10.0f, 0, 0, 0);
        vec3 pos = build_vec3(x, y, z);
        EsTree tree = trees_gen_test();
        trees_add_to_geom_at_pos(&tree, &w->tree_geom, pos);
        geom_simplify_geometry(&w->tree_geom);
        w->refresh_tree = SDL_TRUE;
    }
    return SDL_TRUE;
}

SDL_bool world_process_input_event(EsWorld* w, SDL_Event e) {
    if (e.type == SDL_MOUSEMOTION) {
        float current_x = (float) e.motion.x;
        float current_y = (float) e.motion.y;
        w->mouse.moved_x = (float) e.motion.xrel;
        w->mouse.moved_y = (float) e.motion.yrel;
        w->mouse.current_x = current_x;
        w->mouse.current_y = current_y;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        Uint8 button = e.button.button;
        if (button == SDL_BUTTON_LEFT) {
            w->mouse.l_pressed = SDL_TRUE;
            w->mouse.l_down = SDL_TRUE;
            w->mouse.l_down_x = (float) e.button.x;
            w->mouse.l_down_y = (float) e.button.y;
        } 
        if (button == SDL_BUTTON_RIGHT) {
            w->mouse.r_pressed = SDL_TRUE;
            w->mouse.r_down = SDL_TRUE;
            w->mouse.r_down_x = (float) e.button.x;
            w->mouse.r_down_y = (float) e.button.y;
        }
    }
    if (e.type == SDL_MOUSEBUTTONUP) {
        Uint8 button = e.button.button;
        if (button == SDL_BUTTON_LEFT)
            w->mouse.l_released = SDL_TRUE;
        if (button == SDL_BUTTON_RIGHT)
            w->mouse.r_released = SDL_TRUE;
    }
    if (e.type == SDL_KEYDOWN) {
        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_ESCAPE)
            w->running = SDL_FALSE;
        if (key == SDLK_w)
            w->controls.up_down = SDL_TRUE;
        if (key == SDLK_s)
            w->controls.down_down = SDL_TRUE;
        if (key == SDLK_a)
            w->controls.left_down = SDL_TRUE;
        if (key == SDLK_d)
            w->controls.right_down = SDL_TRUE;
    }
    if (e.type == SDL_KEYUP) {
        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_w)
            w->controls.up_down = SDL_FALSE;
        if (key == SDLK_s)
            w->controls.down_down = SDL_FALSE;
        if (key == SDLK_a)
            w->controls.left_down = SDL_FALSE;
        if (key == SDLK_d)
            w->controls.right_down = SDL_FALSE;
    }
    return SDL_TRUE;
}

SDL_bool world_flush_inputs(EsWorld* w) {
    w->mouse.l_pressed = SDL_FALSE;
    w->mouse.r_pressed = SDL_FALSE;
    w->mouse.l_released = SDL_FALSE;
    w->mouse.r_released = SDL_FALSE;
    w->mouse.moved_x = 0.0f;
    w->mouse.moved_y = 0.0f;
    w->refresh_tree = SDL_FALSE;
    return SDL_TRUE;
}
