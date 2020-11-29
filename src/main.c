#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include "es_painter.h"
#include "es_warehouse.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_bool result;
    SDL_bool run_app = SDL_TRUE;

    EsPainter painter;
    result = painter_initialise(&painter);
    if (!result)
        return -1;

    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    while (run_app) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
            SDL_Log("Window is resized\n");
            painter.buffer_resized = SDL_TRUE;
        }
        result = painter_paint_frame(&painter);
        if (!result)
            return -2;
    }
    
    painter_cleanup(&painter);
    SDL_Log("Quitting Easel\n");
    return 0;
}
