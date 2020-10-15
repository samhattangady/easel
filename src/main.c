#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include <stdio.h>

int main(int argc, char** argv) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    int init_success = SDL_Init(SDL_INIT_VIDEO);
    if (init_success != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initalise SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window;
    window = SDL_CreateWindow("Easel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_VULKAN);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window %s\n", SDL_GetError());
        SDL_Quit();
        return -2;
    }

    int extensions = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensions, NULL);
    SDL_Log("Vulkan seems to support %i extensions.\n", extensions);
    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
