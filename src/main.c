#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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

    VkInstance instance;
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Easel";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "chapliboy engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.pApplicationInfo = &app_info;
    uint32_t extension_count = 0;
    const char** extension_names;
    SDL_bool get_extensions;
    get_extensions = SDL_Vulkan_GetInstanceExtensions(window, &extension_count, NULL);
    if (!get_extensions) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not get vulkan extensions %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -3;
    }
    // TODO (16 Oct 2020 sam): This malloc doesn't seem to be needed.
    // extension_names = (char**) malloc(extension_count * sizeof(char*));
    // TODO (16 Oct 2020 sam): Somehow when we run this the second time with &extension_names
    // it runs correctly. If we run it first time, it fails.
    get_extensions = SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names);
    if (!get_extensions) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not get vulkan extensions %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -3;
    }
    SDL_Log("SDL got %i extensions\n", extension_count);
    for (int i=0; i<extension_count; i++) {
        SDL_Log("Extension %i = %s\n", i, extension_names[i]);
    }
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extension_names;
    create_info.enabledLayerCount = 0;
    SDL_Log("Creating Vulkan Instance\n");
    VkResult create_instance = vkCreateInstance(&create_info, NULL, &instance);
    if (create_instance != VK_SUCCESS) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create vulkan instance\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -4;
    }

    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    while (true) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } 
    }

    SDL_Log("Quitting Application\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
