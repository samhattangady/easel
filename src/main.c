#include "SDL.h"
#include "SDL_vulkan.h"

#include <vulkan/vulkan.h>
#include "es_painter.h"
#include "es_world.h"
#include "es_warehouse.h"
#include "es_ui.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_bool result;

    EsPainter painter;
    EsWorld world;
    EsUI ui;
    result = ui_init(&ui);
    if (!result)
        return -1;
    painter.ui = &ui;
    result = world_init(&world);
    if (!result)
        return -1;
    painter.world = &world;
    result = painter_initialise(&painter);
    if (!result)
        return -1;

    SDL_Log("Running Event Loop\n");
    SDL_Event event;
    while (world.running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                world.running = SDL_FALSE;
            } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                SDL_Log("Window is resized\n");
                painter.buffer_resized = SDL_TRUE;
            } else {
                world_process_input_event(&world, event);
            }
        }
        ui_render_text(&ui, "Easel", 0.0f, 0.0f);
        result = world_update(&world);
        result = painter_paint_frame(&painter);
        if (!result)
            return -2;
        result = world_flush_inputs(&world);
        result = ui_flush(&ui);
    }

    SDL_Log("Program quit after %i ticks", event.quit.timestamp);
    painter_cleanup(&painter);
    SDL_Log("Quitting Easel\n");
    return 0;
}
