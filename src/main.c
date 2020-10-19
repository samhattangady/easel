#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include "es_painter.h"
#include "es_warehouse.h"

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_bool result;

    EsPainter painter;

    result = painter_initialise(&painter);
    if (!result)
        return -1;

    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    while (SDL_TRUE) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } 
        result = painter_paint_frame(&painter);
        if (!result)
            return -2;
    }
    
    painter_cleanup(&painter);
    return 0;
}
