#include "es_world.h"
#include "stb_perlin.h"

#define MOVE_SPEED 6.0f
#define THROW_MAGNITUDE 20.0f
#define DRAG_COEFFICIENT 0.05f
#define ROLL_SPEED M_PI * 2.0f
#define PITCH_SPEED M_PI * 2.0f

SDL_bool _world_aim_update(EsWorld* w, Uint32 timestep);
SDL_bool _world_fly_update(EsWorld* w, Uint32 timestep);

SDL_bool world_init(EsWorld* w) {
    w->running = SDL_TRUE;
    w->mode = MODE_AIM;
    w->player_transform.position = build_vec3(0.0f, 3.0f, 35.0f);
    w->player_transform.facing = build_vec3(0.0f, 0.0f, -1.0f);
    w->player_transform.up = build_vec3(0.0f, 1.0f, 0.0f);
    w->player_transform.world_down = build_vec3(0.0f, 1.0f, 0.0f);
    w->player_forces.velocity = build_vec3(0.0f, 0.0f, 0.0f);
    w->player_forces.lift = build_vec3(0.0f, 0.0f, 0.0f);
    w->player_forces.weight = vec3_scale(w->player_transform.world_down, -1.0f);
    w->player_forces.thrust = build_vec3(0.0f, 0.0f, 0.0f);
    w->player_forces.drag = build_vec3(0.0f, 0.0f, 0.0f);
    w->position = build_vec3(0.0f, 0.0f, 0.0f);
    w->target = build_vec3(0.0f, 0.0f, 0.0f);
    w->up_axis = build_vec3(0.0f, 1.0f, 0.0f);
    w->controls.up_down = SDL_FALSE;
    w->controls.down_down = SDL_FALSE;
    w->controls.left_down = SDL_FALSE;
    w->controls.right_down = SDL_FALSE;
    w->controls.q_down = SDL_FALSE;
    w->controls.e_down = SDL_FALSE;
    w->tree_geom = geom_init_geometry_size(300000, 700000, 2, 300000, 0);
    w->refresh_tree = SDL_FALSE;
    w->refresh_shaders = SDL_FALSE;
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

SDL_bool _world_aim_update(EsWorld* w, Uint32 timestep) {
    vec3 forward = vec3_normalize(w->player_transform.facing);
    vec3 right = vec3_normalize(vec3_cross(forward, w->player_transform.up));
    vec3 movement = build_vec3(0.0f, 0.0f, 0.0f);
    if (w->controls.up_down)
        movement = vec3_add(movement, vec3_scale(forward, MOVE_SPEED));
    if (w->controls.down_down)
        movement = vec3_add(movement, vec3_scale(forward, -MOVE_SPEED));
    if (w->controls.right_down)
        movement = vec3_add(movement, vec3_scale(right, MOVE_SPEED));
    if (w->controls.left_down)
        movement = vec3_add(movement, vec3_scale(right, -MOVE_SPEED));
    if (w->controls.q_down)
        w->player_transform.up = rotate_about_origin_axis(w->up_axis, -0.005, w->player_transform.facing);
    if (w->controls.e_down)
        w->player_transform.up = rotate_about_origin_axis(w->up_axis, 0.005, w->player_transform.facing);
    if (w->mouse.l_pressed) {
        w->mode = MODE_FLY;
        w->player_forces.velocity = vec3_scale(w->player_transform.facing, THROW_MAGNITUDE);
    }
    // TODO (03 Dec 2020 sam): Normalize when moving forward and side.
    movement = vec3_scale(movement, timestep/1000.0f);
    // w->player_transform.position = vec3_add(w->player_transform.position, movement);
    float x_angle = -w->mouse.moved_x / 768.0f;
    float y_angle = w->mouse.moved_y / 1024.0f;
    w->player_transform.facing = rotate_about_origin_axis(w->player_transform.facing, x_angle, w->player_transform.up);
    w->player_transform.facing = rotate_about_origin_axis(w->player_transform.facing, y_angle, vec3_cross(w->player_transform.up, w->player_transform.facing));
    return SDL_TRUE;
}

SDL_bool _world_fly_update(EsWorld* w, Uint32 timestep) {
    // Apply velocity at given position
    w->player_transform.position = vec3_add(w->player_transform.position, vec3_scale(w->player_forces.velocity, timestep/1000.0f));
    // Calculate forces at given orientation
    // Update velocity for next frame
    w->player_forces.velocity = vec3_add(w->player_forces.velocity, vec3_scale(w->player_forces.weight, timestep/1000.0f));
    w->player_forces.velocity = vec3_add(w->player_forces.velocity, vec3_scale(w->player_forces.lift, timestep/1000.0f));
    if (w->controls.up_down)
        w->player_transform.facing = rotate_about_origin_axis(w->player_transform.facing, PITCH_SPEED/timestep/1000.0f, vec3_cross(w->player_transform.up, w->player_transform.facing));
    if (w->controls.down_down)
        w->player_transform.facing = rotate_about_origin_axis(w->player_transform.facing, -PITCH_SPEED/timestep/1000.0f, vec3_cross(w->player_transform.up, w->player_transform.facing));
    if (w->controls.right_down)
        w->player_transform.up = rotate_about_origin_axis(w->player_transform.up, 0.005f, w->player_transform.facing);
    if (w->controls.left_down)
        w->player_transform.up = rotate_about_origin_axis(w->player_transform.up, -0.005f, w->player_transform.facing);
    // float x_angle = (w->mouse.moved_x / 768.0f) * M_PI;
    // float y_angle = (-w->mouse.moved_y / 1024.0f) * M_PI;
    // w->player_transform.up = rotate_about_origin_axis(w->up_axis, x_angle, w->player_transform.facing);
    // w->player_transform.facing = rotate_about_origin_axis(w->player_transform.facing, y_angle, vec3_cross(w->player_transform.up, w->player_transform.facing));
    // TODO (20 Jan 2021 sam): Make this check against the noise at xz rather than constant.
    if (w->player_transform.position.y < 0.3f) {
        w->mode = MODE_AIM;
        w->player_transform.position.y = 5.0f;
        w->player_transform.up = build_vec3(0.0f, 1.0f, 0.0f);
    }
    return SDL_TRUE;
}


SDL_bool world_update(EsWorld* w, Uint32 timestep) {
    if (w->mode == MODE_AIM)
        _world_aim_update(w, timestep);
    else if (w->mode == MODE_FLY)
        _world_fly_update(w, timestep);
    w->position = vec3_add(w->player_transform.position, vec3_scale(w->player_transform.facing, -5.0));
    w->target = vec3_add(w->player_transform.position, vec3_scale(w->player_transform.facing, 10.0f));
    w->up_axis = w->player_transform.up;
    if (w->mouse.r_pressed) {
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
        if (key == SDLK_q)
            w->controls.q_down = SDL_TRUE;
        if (key == SDLK_e)
            w->controls.e_down = SDL_TRUE;
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
        if (key == SDLK_q)
            w->controls.q_down = SDL_FALSE;
        if (key == SDLK_e)
            w->controls.e_down = SDL_FALSE;
        if (key == SDLK_r)
            w->refresh_shaders = SDL_TRUE;
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
